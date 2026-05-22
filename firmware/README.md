# NeuroGuard Firmware (ESP32 v2.0)

GPS: **phone over BLE** (best). If no phone fix: **SIM800L cell location** (rough, ~100 m–2 km). No NEO-6M module.

## Hardware + ML (integrated)

Your working MAX30102 + MPU6050 sketch is integrated in `hardware_logic.cpp`.  
See [INTEGRATION.md](INTEGRATION.md).

## Flash

1. Open `neuroguard_main/neuroguard_main.ino` in Arduino IDE
2. Board: **ESP32 Dev Module**
3. Install libraries from `libraries.txt`
4. Edit `config.h`: phone numbers, `SIMULATION_MODE`, `ENABLE_GSM`
5. Upload  

If you see **multiple definition** errors for `heartRate` / `checkForBeat`, delete any local `heartRate.cpp` in the sketch folder (use the SparkFun library copy only), then **Sketch → Clean Build Folder** and compile again.

## Simulation (no sensors)

Set `SIMULATION_MODE` to `1` in `config.h` — IMU data is synthetic; GSM/OLED skipped on serial.

## ML model (integrated from `mlmodel/`)

1. Install **EloquentTinyML** v3.x from Library Manager  
2. Install **tflm_esp32**: [github.com/eloquentarduino/tflm_esp32](https://github.com/eloquentarduino/tflm_esp32) → clone into `Documents/Arduino/libraries/tflm_esp32`  
   - If you see **multiple definition** errors for `CircularBuffer*` / `kiss_fft*`, or missing `signal/...` headers, run `firmware/patch_tflm_esp32.ps1` once (keeps `src/signal` headers, removes only duplicate `src/signal/src/*.cpp`, drops broken `precompiled=full`).  
3. `ENABLE_ML` is `1` in `config.h` by default  
3. Files: `seizure_model.h`, `seizure_features.h`, `model_params.h`, `seizure_tflite.h`  
4. Set `ENABLE_ML 0` to use threshold-only detection

## SIM rough location (no phone GPS)

When `ENABLE_GSM` and `ENABLE_GSM_LBS` are `1` in `config.h`, the band queries the SIM800L on the cellular network (`AT+CLBS` then `AT+CIPGSMLOC`) before sending an alert if the phone did not provide GPS.

- Requires **SIM inserted**, antenna, and **network registration** (not Wi‑Fi on a phone).
- Set `GSM_APN` in `config.h` if `CIPGSMLOC` fails (carrier APN).
- Serial log: `[GSM-LBS] OK via CLBS` or `CIPGSMLOC`.
- SMS map link is tagged `(approx cell)` when only SIM location is used.

## Pin map

See docs site section **04 Circuit Design** or `../hardware/wiring.md`.
