# NeuroGuard ML

**Status:** Integrated into `firmware/neuroguard_main/` from `mlmodel/`.

## On-device pipeline

1. MPU6050 → 200×6 ring buffer @ 100 Hz  
2. Every 2 s: `extractFeatures()` → 60 floats (z-scored)  
3. TFLite → Motion / Rest / **Seizure** probabilities  
4. Blended with 5-feature threshold (`ML_CONFIDENCE_BLEND` in `config.h`)

## Arduino library

Install **EloquentTinyML** (Library Manager).

## Toggle ML

`firmware/neuroguard_main/config.h`:

```cpp
#define ENABLE_ML 1   // 0 = threshold only
```

## Training notes

See `TRAINING_GUIDE.md`. Your trained `FEAT_MEAN` / `FEAT_STD` in `model_params.h` must match the IMU scaling in `readIMU()` (accel `/16384`, gyro `/131`).
