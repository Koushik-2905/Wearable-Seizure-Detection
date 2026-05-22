# Hardware + ML integration

Your **bench-tested sketch** logic lives in `neuroguard_main/hardware_logic.cpp` (unchanged thresholds).

## One firmware — full project

Flash: `firmware/neuroguard_main/neuroguard_main.ino`

| Layer | What runs |
|-------|-----------|
| **Your rules** | Every 20 ms — IR, BPM, accel → `NORMAL` / `ABNORMAL - …` |
| **ML model** | Every 2 s — 200×6 window → TFLite seizure score |
| **BLE + GPS** | Phone sends lat/lng → stored for SMS |
| **Alerts** | Hardware abnormal **or** ML score ≥ 0.65 → SMS + BLE |

## Serial output (same as your test + ML)

```
IR,BPM,AX,AY,AZ,STATUS,ML
123456,72,1200,-800,16000,NORMAL
[ML] Seizure score: 0.12
```

## Libraries (Arduino)

- SparkFun MAX3010x
- MPU6050
- EloquentTinyML
- ArduinoJson
- Adafruit SSD1306 (optional OLED)

## Config (`config.h`)

```cpp
#define USE_HARDWARE_LOGIC 1   // your sketch rules
#define ENABLE_ML 1            // TFLite model
#define ML_ALERT_THR 0.65f     // ML-only trigger threshold
```

Set `USE_HARDWARE_LOGIC 0` to use only the old 5-feature + ML blend (no bench rules).

## Important

- **Your logic** uses raw MPU `getAcceleration` thresholds (`25000`, `30000`).
- **ML** uses scaled `getMotion6` at 100 Hz (`/16384`, `/131`) — matches training.
- Both run in parallel; alerts fire if **either** flags a problem.
