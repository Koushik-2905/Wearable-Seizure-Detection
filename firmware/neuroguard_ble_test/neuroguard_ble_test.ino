// ================================================================
// NeuroGuard BLE CONNECTION TEST (minimal)
// Upload this to ESP32 → open phone app → should see "NeuroGuard"
// No sensors, GSM, or seizure logic — BLE only.
// ================================================================

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME "NeuroGuard"
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHAR_UUID_GPS_WRITE "12345678-1234-1234-1234-123456789c9c"
#define CHAR_UUID_ALERT     "12345678-1234-1234-1234-123456789abd"
#define CHAR_UUID_DATA      "12345678-1234-1234-1234-123456789abe"

BLECharacteristic* alertChar;
BLECharacteristic* dataChar;
bool phoneConnected = false;

float gpsLat = 0.0f;
float gpsLng = 0.0f;
bool gpsOk = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* s) {
    phoneConnected = true;
    Serial.println("[BLE] Phone CONNECTED");
  }
  void onDisconnect(BLEServer* s) {
    phoneConnected = false;
    BLEDevice::startAdvertising();
    Serial.println("[BLE] Phone disconnected — advertising again");
  }
};

class GPSWriteCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) {
    String s = String(pChar->getValue().c_str());
    int comma = s.indexOf(',');
    if (comma < 0) return;
    gpsLat = s.substring(0, comma).toFloat();
    gpsLng = s.substring(comma + 1).toFloat();
    gpsOk = true;
    Serial.printf("[GPS] From phone: %.6f, %.6f\n", gpsLat, gpsLng);
  }
};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== NeuroGuard BLE TEST ===");

  BLEDevice::init(DEVICE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);

  BLECharacteristic* gpsChar = service->createCharacteristic(
      CHAR_UUID_GPS_WRITE,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  gpsChar->setCallbacks(new GPSWriteCallback());

  alertChar = service->createCharacteristic(
      CHAR_UUID_ALERT, BLECharacteristic::PROPERTY_NOTIFY);
  alertChar->addDescriptor(new BLE2902());

  dataChar = service->createCharacteristic(
      CHAR_UUID_DATA, BLECharacteristic::PROPERTY_NOTIFY);
  dataChar->addDescriptor(new BLE2902());

  service->start();
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising as \"NeuroGuard\"");
  Serial.println("Open the app → scan → tap NeuroGuard");
}

void loop() {
  static unsigned long lastNotify = 0;
  if (!phoneConnected || !dataChar) return;

  if (millis() - lastNotify < 1000) return;
  lastNotify = millis();

  // JSON the Flutter app expects
  char json[180];
  snprintf(json, sizeof(json),
           "{\"state\":0,\"hr\":72,\"conf\":0.05,\"gpsOk\":%s,\"lat\":%.6f,\"lng\":%.6f}",
           gpsOk ? "true" : "false", gpsLat, gpsLng);

  dataChar->setValue(json);
  dataChar->notify();
  Serial.print("[DATA] Sent: ");
  Serial.println(json);
}
