// Bench-tested sensor logic from your standalone sketch (logic preserved).
#include "hardware_logic.h"
#include "config.h"

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <MPU6050.h>
#include <math.h>

MAX30105 particleSensor;
MPU6050 mpu;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute = 0;
int beatAvg = 0;

const int BUFFER_SIZE = 20;
float accelBuffer[BUFFER_SIZE];
int accelIndex = 0;
float accelMag = 0;
float accelAvg = 0;

void hardwareLogicInit() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);

  Serial.println("Initializing sensors...");

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 FAILED");
    while (1)
      ;
  }

  particleSensor.setup(50, 4, 2, 100, 411, 4096);
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);
  delay(500);

  mpu.initialize();
  delay(500);

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  delay(500);

  Serial.println("IR,BPM,AX,AY,AZ,STATUS,ML");
}

HardwareLogicState hardwareLogicTick() {
  HardwareLogicState out;
  int16_t ax, ay, az;

  long irValue = particleSensor.getIR();
  out.irValue = irValue;

  if (irValue < 10000) {
    beatAvg = 0;
    out.fingerPresent = false;
    out.beatAvg = 0;
    Serial.println("No finger");
    return out;
  }
  out.fingerPresent = true;

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60.0 / (delta / 1000.0);
    if (beatsPerMinute > 50 && beatsPerMinute < 120) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      int total = 0;
      for (byte i = 0; i < RATE_SIZE; i++) total += rates[i];
      beatAvg = total / RATE_SIZE;
    }
  }

  mpu.getAcceleration(&ax, &ay, &az);
  out.ax = ax;
  out.ay = ay;
  out.az = az;

  accelMag = sqrt((float)ax * ax + (float)ay * ay + (float)az * az);
  accelBuffer[accelIndex++] = accelMag;
  accelIndex %= BUFFER_SIZE;

  accelAvg = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) accelAvg += accelBuffer[i];
  accelAvg /= BUFFER_SIZE;

  out.accelMag = accelMag;
  out.accelAvg = accelAvg;
  out.beatAvg = beatAvg;

  bool bpmAbnormal = (beatAvg > 120 || beatAvg < 50);
  bool highMotion = (accelAvg > 25000);
  bool extremeShake = (accelMag > 30000);

  String status = "NORMAL";
  if (bpmAbnormal && highMotion) {
    status = "ABNORMAL - POSSIBLE FIT";
  } else if (highMotion && extremeShake) {
    status = "ABNORMAL - SHAKING";
  }

  out.isAbnormal = status.startsWith("ABNORMAL");
  status.toCharArray(out.status, sizeof(out.status));

  Serial.print(irValue);
  Serial.print(",");
  Serial.print(beatAvg);
  Serial.print(",");
  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.print(az);
  Serial.print(",");
  Serial.println(status);

  return out;
}
