#include "detection.h"
#include "seizure_tflite.h"
#include <math.h>

float analyzePattern(DetectionContext &ctx) {
  float score = 0;
  int votes = 0;
  float sumMag = 0, maxMag = 0;

  for (int i = 0; i < WINDOW_SIZE; i++) {
    float m = sqrtf(ctx.aX[i] * ctx.aX[i] + ctx.aY[i] * ctx.aY[i] + ctx.aZ[i] * ctx.aZ[i]);
    sumMag += m;
    if (m > maxMag) maxMag = m;
  }
  if (maxMag > SEIZURE_MAG_THR) {
    score += 0.30f;
    votes++;
  }

  int zc = 0;
  for (int i = 1; i < WINDOW_SIZE; i++)
    if ((ctx.aX[i] > 0) != (ctx.aX[i - 1] > 0)) zc++;
  float zcr = (float)zc / (WINDOW_SIZE / SAMPLE_RATE);
  if (zcr >= ZCR_LOW && zcr <= ZCR_HIGH) {
    score += 0.25f;
    votes++;
  }

  float avg = sumMag / WINDOW_SIZE, vari = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    float m = sqrtf(ctx.aX[i] * ctx.aX[i] + ctx.aY[i] * ctx.aY[i] + ctx.aZ[i] * ctx.aZ[i]);
    vari += (m - avg) * (m - avg);
  }
  if (vari / WINDOW_SIZE > 1.5f) {
    score += 0.20f;
    votes++;
  }

  float gs = 0;
  for (int i = 0; i < WINDOW_SIZE; i++)
    gs += sqrtf(ctx.gX[i] * ctx.gX[i] + ctx.gY[i] * ctx.gY[i] + ctx.gZ[i] * ctx.gZ[i]);
  if (gs / WINDOW_SIZE > 100.0f) {
    score += 0.15f;
    votes++;
  }

  if (ctx.hrBPM > 100) {
    score += 0.10f;
    votes++;
  }

  float thresholdScore = (votes >= 3) ? score : 0.0f;

#if ENABLE_ML
  if (gMlReady) {
    thresholdScore =
        thresholdScore * (1.0f - ML_CONFIDENCE_BLEND) + ctx.mlSeizureScore * ML_CONFIDENCE_BLEND;
  }
#endif

  return thresholdScore;
}
