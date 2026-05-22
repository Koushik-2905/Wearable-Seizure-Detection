#pragma once

#include "config.h"

enum DeviceState { MONITORING, DETECTED, ALERT_SENT, CANCELLED };

struct DetectionContext {
  float aX[WINDOW_SIZE], aY[WINDOW_SIZE], aZ[WINDOW_SIZE];
  float gX[WINDOW_SIZE], gY[WINDOW_SIZE], gZ[WINDOW_SIZE];
  int bufIdx = 0;
  bool bufFull = false;
  int hrBPM = 72;
  float confidence = 0.0f;
  float mlSeizureScore = 0.0f;
};

float analyzePattern(DetectionContext &ctx);
