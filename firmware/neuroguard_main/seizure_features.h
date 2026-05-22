#pragma once

#include <math.h>
#include <string.h>
#include "config.h"
#include "model_params.h"
#include "detection.h"

// 60 features from 200×6 IMU window (must match training pipeline in mlmodel/)

inline void extractFeatures(const DetectionContext &ctx, float *out) {
  const float *axes[6] = {ctx.aX, ctx.aY, ctx.aZ, ctx.gX, ctx.gY, ctx.gZ};
  int fi = 0;

  for (int ax = 0; ax < 6; ax++) {
    const float *sig = axes[ax];

    float sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) sum += sig[i];
    float mean = sum / WINDOW_SIZE;
    out[fi++] = mean;

    float sq_sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) sq_sum += (sig[i] - mean) * (sig[i] - mean);
    out[fi++] = sqrtf(sq_sum / WINDOW_SIZE);

    float mx = sig[0], mn = sig[0];
    for (int i = 1; i < WINDOW_SIZE; i++) {
      if (sig[i] > mx) mx = sig[i];
      if (sig[i] < mn) mn = sig[i];
    }
    out[fi++] = mx;
    out[fi++] = mn;
    out[fi++] = mx - mn;

    float abs_sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) abs_sum += fabsf(sig[i]);
    out[fi++] = abs_sum / WINDOW_SIZE;

    float rms_sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) rms_sum += sig[i] * sig[i];
    out[fi++] = sqrtf(rms_sum / WINDOW_SIZE);

    int zc = 0;
    for (int i = 1; i < WINDOW_SIZE; i++) {
      if ((sig[i] > 0) != (sig[i - 1] > 0)) zc++;
    }
    out[fi++] = (float)zc;

    float buf[WINDOW_SIZE];
    memcpy(buf, sig, WINDOW_SIZE * sizeof(float));
    for (int i = 1; i < WINDOW_SIZE; i++) {
      float key = buf[i];
      int j = i - 1;
      while (j >= 0 && buf[j] > key) {
        buf[j + 1] = buf[j];
        j--;
      }
      buf[j + 1] = key;
    }
    out[fi++] = buf[149] - buf[49];

    float wl = 0;
    for (int i = 1; i < WINDOW_SIZE; i++) wl += fabsf(sig[i] - sig[i - 1]);
    out[fi++] = wl;
  }

  for (int i = 0; i < N_FEATURES; i++) {
    out[i] = (out[i] - FEAT_MEAN[i]) / FEAT_STD[i];
  }
}
