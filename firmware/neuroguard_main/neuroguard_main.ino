// NeuroGuard ESP32 Firmware v2.0 — GPS via BLE from phone
#include "config.h"
#include "detection.h"
#include "seizure_tflite.h"

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
#if ENABLE_HR_SENSOR
#include <MAX30105.h>
#endif
#endif

#if ENABLE_GSM
HardwareSerial gsmSerial(2);
#endif

#if !SIMULATION_MODE
MPU6050 mpu;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
#if ENABLE_HR_SENSOR
MAX30105 particleSensor;
#endif
#endif

BLECharacteristic *alertChar = nullptr;
BLECharacteristic *dataChar = nullptr;
BLECharacteristic *gpsWriteChar = nullptr;
bool deviceConnected = false;

DetectionContext detCtx;
DeviceState state = MONITORING;
unsigned long tDetect = 0;
bool cancelFlag = false;

float gpslat = 0.0f, gpslng = 0.0f;
bool gpsValid = false;
unsigned long lastGPSReceived = 0;

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
    std::string val = pChar->getValue();
    if (val.length() < 5) return;
    String s = String(val.c_str());
    int commaIdx = s.indexOf(',');
    if (commaIdx < 0) return;
    float lat = s.substring(0, commaIdx).toFloat();
    float lng = s.substring(commaIdx + 1).toFloat();
    if (lat >= -90.0f && lat <= 90.0f && lng >= -180.0f && lng <= 180.0f) {
      gpslat = lat;
      gpslng = lng;
      gpsValid = true;
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
  initMPU();
  initGSM();
  initBLE();

#if ENABLE_ML
  initMlModel();
#endif

  showOLED("NeuroGuard v2.0", "Monitoring...", "Waiting for phone");
  Serial.println("[BOOT] NeuroGuard ready — GPS via BLE");
}

void loop() {
  static unsigned long tSample = 0;
  static unsigned long tHR = 0;
  static unsigned long tMl = 0;

  if (millis() - tSample >= 10) {
    tSample = millis();
    readIMU();

#if ENABLE_ML
    if (gMlReady && detCtx.bufFull && millis() - tMl >= ML_INFERENCE_INTERVAL_MS) {
      tMl = millis();
      detCtx.mlSeizureScore = runSeizureInference(detCtx);
    }
#endif

    if (detCtx.bufFull) {
      detCtx.confidence = analyzePattern(detCtx);
      if (state == MONITORING && detCtx.confidence > CONFIDENCE_THR) {
        state = DETECTED;
        tDetect = millis();
        activateLocal();
        Serial.printf("[DETECT] Confidence: %.2f\n", detCtx.confidence);
      }
    }
  }

  if (state == DETECTED) {
    unsigned long elapsed = millis() - tDetect;
    if (cancelFlag) {
      doCancel();
    } else if (elapsed >= CANCEL_WINDOW_MS && elapsed >= MIN_DURATION_MS) {
      dispatchAlerts();
      state = ALERT_SENT;
    }
    int rem = max(0, (int)(CANCEL_WINDOW_MS - elapsed) / 1000);
    char buf[32];
    snprintf(buf, 32, "Cancel: %ds", rem);
    showOLED("!! SEIZURE !!", "Sending alert...", buf);
  }

  if (millis() - tHR >= 5000) {
    tHR = millis();
#if !SIMULATION_MODE && ENABLE_HR_SENSOR
    (void)particleSensor.getIR();
    // TODO: wire SparkFun heartRate.ino algorithm or your signal processing
#else
    detCtx.hrBPM = 72;
#endif
  }

  if (deviceConnected) broadcastData();

  if (gpsValid && millis() - lastGPSReceived > GPS_STALE_MS) {
    gpsValid = false;
    Serial.println("[GPS] Stale — phone may be out of BLE range");
  }

  if (state == ALERT_SENT && millis() - tDetect > 60000) {
    state = MONITORING;
    cancelFlag = false;
    showOLED("NeuroGuard", "Monitoring...", "Alert resolved");
  }
}

void dispatchAlerts() {
  char loc[140];
  if (gpsValid)
    snprintf(loc, 140, "maps.google.com/?q=%.6f,%.6f", gpslat, gpslng);
  else
    strcpy(loc, "GPS unavailable (open app)");

  char sms[256];
  snprintf(sms, 256, "NEUROGUARD ALERT: Seizure! HR:%dBPM Conf:%.0f%% %s", detCtx.hrBPM,
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
    doc["valid"] = gpsValid;
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
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(VIB_MOTOR_PIN, LOW);
  cancelFlag = false;
  showOLED("Cancelled", "False positive?", "Resuming...");
  delay(3000);
  state = MONITORING;
}

void IRAM_ATTR handleSOS() {
  if (state == DETECTED)
    cancelFlag = true;
  else if (state == MONITORING) {
    state = DETECTED;
    tDetect = millis() - CANCEL_WINDOW_MS;
  }
}

void sendSMS(const char *num, const char *msg) {
#if ENABLE_GSM
  gsmSerial.println("AT+CMGF=1");
  delay(500);
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(num);
  gsmSerial.println("\"");
  delay(500);
  gsmSerial.print(msg);
  delay(100);
  gsmSerial.write(26);
  delay(3000);
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
  doc["gpsOk"] = gpsValid;
  doc["lat"] = gpslat;
  doc["lng"] = gpslng;
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
#if !SIMULATION_MODE
  mpu.initialize();
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  mpu.setDLPFMode(MPU6050_DLPF_BW_42);
#if ENABLE_HR_SENSOR
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
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);
  delay(3000);
  gsmSerial.println("AT");
  delay(500);
  gsmSerial.println("AT+CMGF=1");
  delay(500);
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
