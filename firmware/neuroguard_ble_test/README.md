# BLE connection test only

Minimal ESP32 sketch — no extra libraries (BLE is built into ESP32 board package).

## Steps

1. Arduino IDE → open `neuroguard_ble_test.ino`
2. Board: **ESP32 Dev Module**
3. Upload
4. Serial Monitor **115200** → should show `Advertising as "NeuroGuard"`
5. On phone: NeuroGuard app → refresh scan → tap **NeuroGuard**
6. Live screen should show HR 72, low confidence, GPS after app sends location

## Optional: test alert button

Hold BOOT button on some boards — not wired here. For a fake alert, use Serial Monitor or extend this sketch later.
