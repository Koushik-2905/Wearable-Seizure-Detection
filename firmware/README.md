# NeuroGuard Firmware (ESP32 v2.0)

GPS from phone over BLE — no NEO-6M module.

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

## Pin map

See docs site section **04 Circuit Design** or `../hardware/wiring.md`.
