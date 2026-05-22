
// seizure_tflite.h
#pragma once
#include "seizure_model.h"
#include "seizure_features.h"
#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>

#define ARENA_SIZE 20 * 1024  

// FIX: Explicitly passing float types to matching the updated Float32 IO pipeline
Eloquent::TinyML::TensorFlow::TensorFlow<N_FEATURES, 3, ARENA_SIZE, float, float> ml;

bool initTFLite() {
    if (!ml.begin(seizure_model_tflite)) {
        Serial.println("[ML] Error loading model framework!");
        return false;
    }
    Serial.println("[ML] Target TFLite Engine Online.");
    return true;
}

float runSeizureInference() {
    static float features[N_FEATURES];
    static float scores[3];

    extractFeatures(features);
    ml.predict(features, scores); // Direct float ingestion safe now

    Serial.printf("[ML Score] Motion: %.2f | Rest: %.2f | Seizure: %.2f
",
                  scores[0], scores[1], scores[2]);
    return scores[2];
}
