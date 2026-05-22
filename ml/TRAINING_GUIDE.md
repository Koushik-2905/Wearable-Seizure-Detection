# How to do the ML part for NeuroGuard (step-by-step)

Your firmware **already runs without ML** using the 5-feature threshold detector in `detection.cpp`. ML is an **optional upgrade** that blends in when `ENABLE_ML 1`.

---

## Big picture

```
MPU6050 (100 Hz) → ring buffer 200 × 6 axes
        ↓
┌───────────────────┐     ┌─────────────────────┐
│ Threshold voting  │     │ TinyML (TFLite)     │
│ (already coded)   │     │ seizure probability │
└─────────┬─────────┘     └──────────┬──────────┘
          └──────── blend ───────────┘
                    ↓
            confidence > 0.80 → alert → SMS + BLE
```

Blend weight: `ML_CONFIDENCE_BLEND` in `firmware/neuroguard_main/config.h` (default 35% ML, 65% threshold).

---

## Phase 1 — Collect training data

### What each sample must be

| Field | Spec |
|-------|------|
| Window length | **200 timesteps** (= 2 seconds at 100 Hz) |
| Channels | **6**: ax, ay, az, gx, gy, gz (same scaling as firmware) |
| Label | `0` = rest, `1` = normal motion, `2` = seizure-like |

### How to record (recommended)

1. Flash firmware with **Serial CSV logging** (add temporarily in `readIMU()`):
   - Print: `label,ax,ay,az,gx,gy,gz` at 100 Hz while someone labels the activity.
2. Record scenarios:
   - **Rest**: sitting still (5+ min)
   - **Motion**: walk, run, daily activities (false-positive test)
   - **Seizure-like**: shake wrist ~3–8 Hz, >2.5g for 10–20 s (proxy; real clinical data is better)
3. Store under `ml/data/raw/`.

### Hackathon shortcut (no clinical dataset)

- Use **public IMU activity datasets** (UCI HAR, WISDM) and **map** vigorous classes → pseudo-seizure.
- Or train only **binary**: seizure vs non-seizure from your own 30–60 min lab recordings.

Minimum viable: **~50 seizure windows + ~200 non-seizure windows** before the model is useful.

---

## Phase 2 — Train in Python (Google Colab or local)

### Folder layout

```
ml/
  data/raw/          ← CSV logs from ESP32 or phone
  data/processed/    ← numpy windows
  train/train_model.py
  models/seizure_model.tflite
  models/model_metadata.json
```

### Preprocessing (must match firmware)

```python
# Same scaling as neuroguard_main.ino readIMU():
# accel: raw / 16384.0  (±2g)
# gyro:  raw / 131.0    (250 dps)

WINDOW = 200
CHANNELS = 6
```

Build windows with 50% overlap, normalize per-channel (mean/std from training set only).

### Model (matches project spec)

```text
Input: (200, 6)
  Conv1D(32, kernel=5, activation='relu')
  MaxPooling1D(2)
  LSTM(64)
  Dense(32, relu)
  Dense(3, softmax)   # rest, motion, seizure
```

Train with `class_weight` if seizure class is rare. Target: **>90% sensitivity** on seizure, **<5% false positives** on motion (tune on validation set).

### Export TFLite for ESP32

```python
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
# Provide representative_dataset for full int8
tflite_model = converter.convert()
open("ml/models/seizure_model.tflite", "wb").write(tflite_model)
```

Check size **< 100 KB** (aim ~48 KB). If too large: fewer LSTM units or pure CNN.

---

## Phase 3 — Put model on ESP32 (TFLite Micro)

### Tools

- Arduino library: **TensorFlowLite_ESP32** or Espressif **esp-tflite-micro**
- Convert model to C array:
  ```bash
  xxd -i seizure_model.tflite > firmware/neuroguard_main/seizure_model.cc
  ```

### Implement `ml_inference.cpp`

1. `mlModelReady()` → return `true` after `MicroMutableOpResolver` + `AllocateTensors()` succeed.
2. In `mlSeizureProbability(IMUWindow w)`:
   - Flatten `aX,aY,aZ,gX,gY,gZ` into input tensor (200×6), quantize to int8 using scale/zero_point from model.
   - `Invoke()`.
   - Read output class index **2** (seizure) probability → float 0..1.

Hook is already in `detection.cpp`:

```cpp
thresholdScore = thresholdScore * (1.0f - ML_CONFIDENCE_BLEND)
               + mlProb * ML_CONFIDENCE_BLEND;
```

### Enable in firmware

`firmware/neuroguard_main/config.h`:

```cpp
#define ENABLE_ML 1
```

Re-flash `neuroguard_main.ino`.

---

## Phase 4 — Test ML on device

| Test | How |
|------|-----|
| Inference runs | Serial: print `mlProb` every 2 s without crash |
| Latency | Must be **< 50 ms** per window on ESP32 |
| True seizure | Shake test → confidence rises, alert after grace period |
| False positive | Walk/run 5 min → no alert |
| With phone | BLE + GPS still works (ML does not block GPS path) |

---

## Phase 5 — Optional: train with heart rate later

Firmware already has HR feature in threshold (10% weight). v2 ML can add **7th channel** (HR repeated or HR delta) — requires retraining and new TFLite input shape.

---

## What you do NOT need ML for (already done)

- BLE GPS phone → ESP32 → SMS  
- 5-feature threshold detection  
- Flutter app + caretaker dashboard (demo data)

---

## Recommended order for your team

1. **Week 1:** Log IMU CSV from real ESP32 + label in spreadsheet  
2. **Week 2:** Train/export TFLite in Colab  
3. **Week 3:** Integrate TFLite Micro + tune `CONFIDENCE_THR` and `ML_CONFIDENCE_BLEND`  
4. **Demo:** Show Serial confidence + one intentional shake alert with GPS in SMS  

---

## When your `.tflite` is ready

Place file in `ml/models/` and ask to wire `ml_inference.cpp` — the repo hook in `detection.cpp` is already waiting.
