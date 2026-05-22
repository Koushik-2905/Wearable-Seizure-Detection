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

## Simulation (no sensors)

Set `SIMULATION_MODE` to `1` in `config.h` — IMU data is synthetic; GSM/OLED skipped on serial.

## ML model (integrated from `mlmodel/`)

1. Install **EloquentTinyML** library  
2. `ENABLE_ML` is `1` in `config.h` by default  
3. Files: `seizure_model.h`, `seizure_features.h`, `model_params.h`, `seizure_tflite.h`  
4. Set `ENABLE_ML 0` to use threshold-only detection

## Pin map

See docs site section **04 Circuit Design** or `../hardware/wiring.md`.
