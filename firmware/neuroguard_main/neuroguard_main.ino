// NeuroGuard ESP32 Firmware v2.0 — BLE GPS + SIM800L SMS/cell location
#include "config.h"
#include "detection.h"
#include "seizure_tflite.h"
#include "hardware_logic.h"  // MPU6050 + MAX30102 (extern mpu)
#include "gsm_location.h"      // SIM800L UART2 GPIO16/17

#include <Wire.h>
#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

#if !SIMULATION_MODE
#include <MPU6050.h>
#include <Adafruit_SSD1306.h>
#if ENABLE_HR_SENSOR && !USE_HARDWARE_LOGIC
#include <MAX30105.h>
#endif
#endif

#if ENABLE_GSM
HardwareSerial gsmSerial(2);
#endif

#if !SIMULATION_MODE
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
#endif

HardwareLogicState hwState;

BLECharacteristic *alertChar = nullptr;
BLECharacteristic *dataChar = nullptr;
BLECharacteristic *gpsWriteChar = nullptr;
bool deviceConnected = false;

DetectionContext detCtx;
DeviceState state = MONITORING;
unsigned long tDetect = 0;
unsigned long tBuzzerOn = 0;
bool cancelFlag = false;
bool buzzerActive = false;

static bool cancelButtonPressed() {
  static unsigned long lastMs = 0;
  if (digitalRead(SOS_BTN_PIN) != LOW) return false;
  unsigned long now = millis();
  if (now - lastMs < 250) return false;
  lastMs = now;
  return true;
}

float gpslat = 0.0f, gpslng = 0.0f;
bool gpsValidPhone = false;
bool gpsValidSim = false;
unsigned long lastGPSReceived = 0;
unsigned long lastSimLocationAttempt = 0;

static bool hasGpsFix() { return gpsValidPhone || gpsValidSim; }

static bool trySimLocationFallback() {
#if ENABLE_GSM && ENABLE_GSM_LBS && !SIMULATION_MODE
  float lat = 0, lng = 0;
  if (gsmFetchRoughLocation(gsmSerial, &lat, &lng)) {
    gpslat = lat;
    gpslng = lng;
    gpsValidSim = true;
    Serial.printf("[GPS] SIM rough location: %.6f, %.6f\n", gpslat, gpslng);
    return true;
  }
#endif
  return false;
}

void showOLED(const char *l1, const char *l2, const char *l3);
void initOLED();
void initMPU();
void initGSM();
void initBLE();
void readIMU();
void dispatchAlerts();
void activateLocal();
void doCancel();
void sendSMS(const char *num, const char *msg);
void broadcastData();
void IRAM_ATTR handleSOS();

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *s) {
    deviceConnected = true;
    showOLED("NeuroGuard", "Phone connected", "GPS syncing...");
  }
  void onDisconnect(BLEServer *s) {
    deviceConnected = false;
    BLEDevice::startAdvertising();
    showOLED("NeuroGuard", "Monitoring...", "BLE: searching");
  }
};

class GPSWriteCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    String s = String(pChar->getValue().c_str());
    if (s.length() < 5) return;
    int commaIdx = s.indexOf(',');
    if (commaIdx < 0) return;
    float lat = s.substring(0, commaIdx).toFloat();
    float lng = s.substring(commaIdx + 1).toFloat();
    if (lat >= -90.0f && lat <= 90.0f && lng >= -180.0f && lng <= 180.0f) {
      gpslat = lat;
      gpslng = lng;
      gpsValidPhone = true;
      gpsValidSim = false;
      lastGPSReceived = millis();
      Serial.printf("[GPS] Received from phone: %.6f, %.6f\n", gpslat, gpslng);
    }
  }
};

class ConfigWriteCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    Serial.print("[CONFIG] ");
    Serial.println(pChar->getValue().c_str());
  }
};

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(SOS_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(SOS_BTN_PIN), handleSOS, FALLING);

  initOLED();
#if USE_HARDWARE_LOGIC && !SIMULATION_MODE
  hardwareLogicInit();
#else
  initMPU();
#endif
  initGSM();
  initBLE();

#if ENABLE_ML
  initMlModel();
#endif

  showOLED("NeuroGuard v2.0", "Monitoring...", "Waiting for phone");
  Serial.println("[BOOT] NeuroGuard ready — GPS: phone BLE + SIM cell fallback");
}

void loop() {
  static unsigned long tSample = 0;
  static unsigned long tHw = 0;
  static unsigned long tMl = 0;

#if USE_HARDWARE_LOGIC && !SIMULATION_MODE
  if (millis() - tHw >= HARDWARE_TICK_MS) {
    tHw = millis();
    hwState = hardwareLogicTick();
    detCtx.hrBPM = hwState.beatAvg;

    if (state == MONITORING && hwState.isAbnormal && hwState.fingerPresent) {
      state = DETECTED;
      tDetect = millis();
      buzzerActive = false;
      cancelFlag = false;
      Serial.printf("[DETECT] Hardware rule: %s\n", hwState.status);
    }
  }
#endif

  if (millis() - tSample >= 10) {
    tSample = millis();
    readIMU();

#if ENABLE_ML
    if (gMlReady && detCtx.bufFull && millis() - tMl >= ML_INFERENCE_INTERVAL_MS) {
      tMl = millis();
      detCtx.mlSeizureScore = runSeizureInference(detCtx);
      Serial.printf("[ML] Seizure score: %.2f\n", detCtx.mlSeizureScore);
    }
#endif

    if (detCtx.bufFull) {
#if !USE_HARDWARE_LOGIC
      detCtx.confidence = analyzePattern(detCtx);
#else
      detCtx.confidence = detCtx.mlSeizureScore;
      if (hwState.isAbnormal) {
        if (detCtx.confidence < 0.92f) detCtx.confidence = 0.92f;
      }
#endif

      bool mlTrigger = gMlReady && (detCtx.mlSeizureScore >= ML_ALERT_THR);
      if (state == MONITORING && (mlTrigger
#if !USE_HARDWARE_LOGIC
          || detCtx.confidence > CONFIDENCE_THR
#endif
          )) {
        state = DETECTED;
        tDetect = millis();
        buzzerActive = false;
        cancelFlag = false;
        Serial.printf("[DETECT] ML/threshold confidence: %.2f\n", detCtx.confidence);
      }
    }
  }

  if (state == DETECTED) {
    unsigned long elapsed = millis() - tDetect;

    if (elapsed < MIN_DURATION_MS) {
      int rem = max(1, (int)(MIN_DURATION_MS - elapsed) / 1000);
      char buf[32];
      snprintf(buf, 32, "Buzzer in: %ds", rem);
      showOLED("!! ABNORMAL !!", "Confirming...", buf);
    } else {
      if (!buzzerActive) {
        activateLocal();
        buzzerActive = true;
        tBuzzerOn = millis();
        cancelFlag = false;
        Serial.println("[ALERT] Buzzer ON — 5s to cancel with button");
      }

      unsigned long buzzElapsed = millis() - tBuzzerOn;
      if (cancelFlag || cancelButtonPressed()) {
        doCancel();
      } else if (buzzElapsed >= CANCEL_WINDOW_MS) {
        dispatchAlerts();
        state = ALERT_SENT;
      } else {
        int rem = max(1, (int)(CANCEL_WINDOW_MS - buzzElapsed) / 1000);
        char buf[32];
        snprintf(buf, 32, "Cancel: %ds", rem);
        showOLED("!! SEIZURE !!", "Press D5 cancel", buf);
      }
    }
  }

  if (deviceConnected) broadcastData();

  if (gpsValidPhone && millis() - lastGPSReceived > GPS_STALE_MS) {
    gpsValidPhone = false;
    Serial.println("[GPS] Phone fix stale — trying SIM fallback");
#if ENABLE_GSM && ENABLE_GSM_LBS
    if (!gpsValidSim && millis() - lastSimLocationAttempt > 60000) {
      lastSimLocationAttempt = millis();
      trySimLocationFallback();
    }
#endif
  }

#if ENABLE_GSM && ENABLE_GSM_LBS && !SIMULATION_MODE
  if (!gpsValidPhone && !gpsValidSim && millis() - lastSimLocationAttempt > 120000) {
    lastSimLocationAttempt = millis();
    trySimLocationFallback();
  }
#endif

  if (state == ALERT_SENT && millis() - tDetect > 60000) {
    state = MONITORING;
    cancelFlag = false;
    buzzerActive = false;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(VIB_MOTOR_PIN, LOW);
    showOLED("NeuroGuard", "Monitoring...", "Alert resolved");
  }
}

void dispatchAlerts() {
#if ENABLE_GSM && ENABLE_GSM_LBS
  if (!gpsValidPhone) {
    trySimLocationFallback();
  }
#endif

  char loc[160];
  if (hasGpsFix()) {
    if (gpsValidSim && !gpsValidPhone)
      snprintf(loc, 160, "maps.google.com/?q=%.6f,%.6f (approx cell)", gpslat, gpslng);
    else
      snprintf(loc, 160, "maps.google.com/?q=%.6f,%.6f", gpslat, gpslng);
  } else {
    strcpy(loc, "Location unavailable");
  }

  // Single SMS segment (160 chars max for reliable SIM800L delivery)
  char sms[161];
  snprintf(sms, sizeof(sms), "NEUROGUARD ALERT! HR:%d Conf:%.0f%% %s", detCtx.hrBPM,
           (double)(detCtx.confidence * 100), loc);

#if ENABLE_GSM
  for (int i = 0; i < CONTACT_COUNT; i++) {
    sendSMS(CONTACTS[i], sms);
    delay(2000);
  }
#else
  Serial.println(sms);
#endif

  if (deviceConnected && alertChar) {
    StaticJsonDocument<160> doc;
    doc["type"] = "SEIZURE_ALERT";
    doc["conf"] = (int)(detCtx.confidence * 100);
    doc["hr"] = detCtx.hrBPM;
    doc["lat"] = gpslat;
    doc["lng"] = gpslng;
    doc["valid"] = hasGpsFix();
    if (gpsValidPhone)
      doc["locSrc"] = "phone";
    else if (gpsValidSim)
      doc["locSrc"] = "sim";
    else
      doc["locSrc"] = "none";
    char buf[160];
    serializeJson(doc, buf);
    alertChar->setValue(buf);
    alertChar->notify();
  }

  showOLED("ALERT SENT!", "SMS + BLE", "Help coming...");
  Serial.println("[ALERT] All notifications dispatched");
}

void activateLocal() {
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(VIB_MOTOR_PIN, HIGH);
}

void doCancel() {
  state = CANCELLED;
  buzzerActive = false;
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(VIB_MOTOR_PIN, LOW);
  cancelFlag = false;
  showOLED("Cancelled", "No SMS sent", "Resuming...");
  Serial.println("[ALERT] Cancelled — buzzer off, no GPS/SMS");
  delay(3000);
  state = MONITORING;
}

void IRAM_ATTR handleSOS() {
  if (state == DETECTED && buzzerActive)
    cancelFlag = true;
}

void sendSMS(const char *num, const char *msg) {
#if ENABLE_GSM && !SIMULATION_MODE
  if (gsmSendTextSms(gsmSerial, num, msg)) {
    Serial.printf("[GSM] SMS OK -> %s\n", num);
  } else {
    Serial.printf("[GSM] SMS FAILED -> %s\n", num);
  }
#endif
}

void broadcastData() {
  static unsigned long last = 0;
  if (millis() - last < 1000 || !dataChar) return;
  last = millis();
  StaticJsonDocument<200> doc;
  doc["state"] = (int)state;
  doc["hr"] = detCtx.hrBPM;
  doc["conf"] = detCtx.confidence;
  doc["ml"] = detCtx.mlSeizureScore;
  doc["gpsOk"] = hasGpsFix();
  doc["lat"] = gpslat;
  doc["lng"] = gpslng;
  if (gpsValidPhone)
    doc["locSrc"] = "phone";
  else if (gpsValidSim)
    doc["locSrc"] = "sim";
  else
    doc["locSrc"] = "none";
  char buf[200];
  serializeJson(doc, buf);
  dataChar->setValue(buf);
  dataChar->notify();
}

void readIMU() {
#if SIMULATION_MODE
  float t = millis() / 1000.0f;
  detCtx.aX[detCtx.bufIdx] = sinf(t * 8.0f) * 0.1f;
  detCtx.aY[detCtx.bufIdx] = cosf(t * 8.0f) * 0.1f;
  detCtx.aZ[detCtx.bufIdx] = 1.0f;
  detCtx.gX[detCtx.bufIdx] = detCtx.gY[detCtx.bufIdx] = detCtx.gZ[detCtx.bufIdx] = 0;
#else
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  detCtx.aX[detCtx.bufIdx] = ax / 16384.0f;
  detCtx.aY[detCtx.bufIdx] = ay / 16384.0f;
  detCtx.aZ[detCtx.bufIdx] = az / 16384.0f;
  detCtx.gX[detCtx.bufIdx] = gx / 131.0f;
  detCtx.gY[detCtx.bufIdx] = gy / 131.0f;
  detCtx.gZ[detCtx.bufIdx] = gz / 131.0f;
#endif
  detCtx.bufIdx = (detCtx.bufIdx + 1) % WINDOW_SIZE;
  if (detCtx.bufIdx == 0) detCtx.bufFull = true;
}

void initMPU() {
#if !SIMULATION_MODE && !USE_HARDWARE_LOGIC
  mpu.initialize();
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  mpu.setDLPFMode(MPU6050_DLPF_BW_42);
#if ENABLE_HR_SENSOR
  MAX30105 particleSensor;
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[HR] MAX30105 not found");
  } else {
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeIR(0x0A);
  }
#endif
#endif
}

void initOLED() {
#if ENABLE_OLED && !SIMULATION_MODE
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OLED] init failed");
    return;
  }
  oled.clearDisplay();
  oled.display();
#endif
}

void initGSM() {
#if ENABLE_GSM && !SIMULATION_MODE
  Serial.printf("[GSM] UART2 RX=GPIO%d (SIM TX)  TX=GPIO%d (SIM RX) @ %d\n", GSM_RX, GSM_TX,
                GSM_BAUD);
  gsmSerial.begin(GSM_BAUD, SERIAL_8N1, GSM_RX, GSM_TX);
  delay(GSM_BOOT_DELAY_MS);
  gsmInitModule(gsmSerial);
#endif
}

void showOLED(const char *l1, const char *l2, const char *l3) {
#if ENABLE_OLED && !SIMULATION_MODE
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println(l1);
  oled.setCursor(0, 22);
  oled.println(l2);
  oled.setCursor(0, 44);
  oled.println(l3);
  oled.display();
#else
  Serial.printf("[OLED] %s | %s | %s\n", l1, l2, l3);
#endif
}

void initBLE() {
  BLEDevice::init("NeuroGuard");
  BLEServer *srv = BLEDevice::createServer();
  srv->setCallbacks(new ServerCallbacks());
  BLEService *svc = srv->createService(SERVICE_UUID);

  gpsWriteChar = svc->createCharacteristic(
      CHAR_UUID_GPS_WRITE,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  gpsWriteChar->setCallbacks(new GPSWriteCallback());

  BLECharacteristic *configChar = svc->createCharacteristic(
      CHAR_UUID_CONFIG, BLECharacteristic::PROPERTY_WRITE);
  configChar->setCallbacks(new ConfigWriteCallback());

  alertChar = svc->createCharacteristic(CHAR_UUID_ALERT, BLECharacteristic::PROPERTY_NOTIFY);
  alertChar->addDescriptor(new BLE2902());

  dataChar = svc->createCharacteristic(CHAR_UUID_DATA, BLECharacteristic::PROPERTY_NOTIFY);
  dataChar->addDescriptor(new BLE2902());

  svc->start();
  BLEDevice::startAdvertising();
  Serial.println("[BLE] Advertising as 'NeuroGuard'");
}
