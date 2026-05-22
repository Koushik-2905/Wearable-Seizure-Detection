
// seizure_features.h
#pragma once
#include <math.h>
#include <string.h>  // FIX: Added for memcpy support
#include "model_params.h"

#define WINDOW_SIZE 200
#define N_AXES      6

// Declared as extern; will be physically allocated in the main .ino sketch
extern float aX[WINDOW_SIZE], aY[WINDOW_SIZE], aZ[WINDOW_SIZE];
extern float gX[WINDOW_SIZE], gY[WINDOW_SIZE], gZ[WINDOW_SIZE];
extern float* AXES[6];

void initAxesPtr() {
    AXES[0] = aX; AXES[1] = aY; AXES[2] = aZ;
    AXES[3] = gX; AXES[4] = gY; AXES[5] = gZ;
}

void extractFeatures(float* out) {
    int fi = 0;
    for (int ax = 0; ax < N_AXES; ax++) {
        float* sig = AXES[ax];

        float sum = 0;
        for (int i = 0; i < WINDOW_SIZE; i++) sum += sig[i];
        float mean = sum / WINDOW_SIZE;
        out[fi++] = mean;

        float sq_sum = 0;
        for (int i = 0; i < WINDOW_SIZE; i++) sq_sum += (sig[i]-mean)*(sig[i]-mean);
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
        for (int i = 0; i < WINDOW_SIZE; i++) rms_sum += sig[i]*sig[i];
        out[fi++] = sqrtf(rms_sum / WINDOW_SIZE);

        int zc = 0;
        for (int i = 1; i < WINDOW_SIZE; i++) {
            if ((sig[i] > 0) != (sig[i-1] > 0)) zc++;
        }
        out[fi++] = (float)zc;

        // Approx IQR
        float buf[WINDOW_SIZE];
        memcpy(buf, sig, WINDOW_SIZE * sizeof(float));
        for (int i = 1; i < WINDOW_SIZE; i++) {
            float key = buf[i]; int j = i-1;
            while (j >= 0 && buf[j] > key) { buf[j+1] = buf[j]; j--; }
            buf[j+1] = key;
        }
        out[fi++] = buf[149] - buf[49];

        float wl = 0;
        for (int i = 1; i < WINDOW_SIZE; i++) wl += fabsf(sig[i] - sig[i-1]);
        out[fi++] = wl;
    }

    // Z-Score Normalization
    for (int i = 0; i < N_FEATURES; i++) {
        out[i] = (out[i] - FEAT_MEAN[i]) / FEAT_STD[i];
    }
}
