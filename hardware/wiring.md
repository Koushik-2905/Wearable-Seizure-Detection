# NeuroGuard Wiring (v2.0 — no NEO-6M GPS)

## I2C bus (GPIO 21 SDA, 22 SCL)

| Device | Address | Notes |
|--------|---------|-------|
| MPU6050 | 0x68 | INT → GPIO 35 |
| MAX30102 / MAX30105 | 0x57 | Shared bus |
| SSD1306 OLED | 0x3C | 128×64 |

## UART2 — SIM800L (GPIO 16 RX, 17 TX)

- **Critical:** SIM800L VCC on separate 2A-capable rail + 1000µF cap
- Do not power SIM800L from ESP32 3.3V pin

## GPIO outputs

| Function | GPIO | Driver |
|----------|------|--------|
| Buzzer | 25 | NPN 2N2222 |
| Vibration | 26 | NPN 2N2222 |
| LED | 27 | 220Ω series |
| SOS button | 34 | INPUT_PULLUP, 10kΩ |

## Power

```
LiPo 3.7V → TP4056 → MT3608 → 3.3V → ESP32, sensors, OLED
LiPo direct → SIM800L (with bulk cap)
```

GPIO 13/14 freed (no GPS module).
