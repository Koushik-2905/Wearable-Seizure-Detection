#pragma once

#include <Arduino.h>
#include <MPU6050.h>
#include "MAX30105.h"

extern MPU6050 mpu;
extern MAX30105 particleSensor;

// Your proven bench-test logic (unchanged thresholds / conditions)
struct HardwareLogicState {
  long irValue = 0;
  int beatAvg = 0;
  int16_t ax = 0, ay = 0, az = 0;
  float accelMag = 0;
  float accelAvg = 0;
  char status[40] = "NORMAL";
  bool isAbnormal = false;
  bool fingerPresent = true;
};

void hardwareLogicInit();
HardwareLogicState hardwareLogicTick();
