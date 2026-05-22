#pragma once

#include "config.h"

#if ENABLE_ML

// Arduino.h before TFLM — flatbuffers also defines "String"
#include <Arduino.h>

#include "model_params.h"
#include "seizure_model.h"
#include "seizure_features.h"
#include "detection.h"

#define N_CLASSES 3
#define TF_NUM_OPS 2
#define TF_NUM_INPUTS N_FEATURES
#define TF_NUM_OUTPUTS N_CLASSES
#define TF_OP_FULLYCONNECTED
#define TF_OP_SOFTMAX

#include <tflm_esp32.h>
#include <eloquent_tinyml.h>

#define ML_ARENA_SIZE (20 * 1024)

extern Eloquent::TF::Sequential<TF_NUM_OPS, ML_ARENA_SIZE> gMl;

inline bool gMlReady = false;

inline bool initMlModel() {
  gMl.setNumInputs(N_FEATURES);
  gMl.setNumOutputs(N_CLASSES);

  if (!gMl.begin(seizure_model_tflite).isOk()) {
    Serial.print("[ML] Failed to load TFLite model: ");
    Serial.println(gMl.exception.toCString());
    gMlReady = false;
    return false;
  }
  gMlReady = true;
  Serial.println("[ML] TFLite online (60 features -> 3 classes)");
  return true;
}

// scores[0]=Motion, scores[1]=Rest, scores[2]=Seizure
inline float runSeizureInference(const DetectionContext &ctx) {
  static float features[N_FEATURES];

  extractFeatures(ctx, features);

  if (!gMl.predict(features).isOk()) {
    Serial.print("[ML] predict failed: ");
    Serial.println(gMl.exception.toCString());
    return 0.0f;
  }

  float *scores = gMl.outputs;
  Serial.printf("[ML] Motion:%.2f Rest:%.2f Seizure:%.2f\n", scores[0], scores[1], scores[2]);
  return scores[2];
}

#else

inline bool gMlReady = false;
inline bool initMlModel() { return false; }
inline float runSeizureInference(const DetectionContext &) { return 0.0f; }

#endif
