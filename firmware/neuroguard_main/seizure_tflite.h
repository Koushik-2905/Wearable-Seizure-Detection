#pragma once

#include "config.h"

#if ENABLE_ML

#include "seizure_model.h"
#include "seizure_features.h"
#include "detection.h"
#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>

#define ML_ARENA_SIZE (20 * 1024)
#define N_CLASSES 3

Eloquent::TinyML::TensorFlow::TensorFlow<N_FEATURES, N_CLASSES, ML_ARENA_SIZE, float, float> gMl;

inline bool gMlReady = false;

inline bool initMlModel() {
  if (!gMl.begin(seizure_model_tflite)) {
    Serial.println("[ML] Failed to load TFLite model");
    gMlReady = false;
    return false;
  }
  gMlReady = true;
  Serial.println("[ML] TFLite online (60 features → 3 classes)");
  return true;
}

// scores[0]=Motion, scores[1]=Rest, scores[2]=Seizure
inline float runSeizureInference(const DetectionContext &ctx) {
  static float features[N_FEATURES];
  static float scores[N_CLASSES];

  extractFeatures(ctx, features);
  gMl.predict(features, scores);

  Serial.printf("[ML] Motion:%.2f Rest:%.2f Seizure:%.2f\n", scores[0], scores[1], scores[2]);
  return scores[2];
}

#else

inline bool gMlReady = false;
inline bool initMlModel() { return false; }
inline float runSeizureInference(const DetectionContext &) { return 0.0f; }

#endif
