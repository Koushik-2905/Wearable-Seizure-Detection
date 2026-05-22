#pragma once

// ── Build modes ─────────────────────────────────────────────────
// SIMULATION_MODE=1: run without MPU6050/MAX30102/OLED/GSM (BLE + Serial only)
#define SIMULATION_MODE 0
#define ENABLE_GSM 1
#define ENABLE_OLED 1
#define ENABLE_HR_SENSOR 1

// Trained model from mlmodel/ (EloquentTinyML + 60-feature pipeline)
#define ENABLE_ML 1
#define ML_INFERENCE_INTERVAL_MS 2000

// ── Pins (ESP32 DevKit) ─────────────────────────────────────────
#define SOS_BTN_PIN 34
#define BUZZER_PIN 25
#define VIB_MOTOR_PIN 26
#define LED_PIN 27
#define MPU_INT_PIN 35
#define I2C_SDA 21
#define I2C_SCL 22
#define GSM_RX 16
#define GSM_TX 17

// ── BLE UUIDs (must match mobile_app/lib/constants/ble_uuids.dart) ──
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHAR_UUID_GPS_WRITE "12345678-1234-1234-1234-123456789c9c"
#define CHAR_UUID_ALERT "12345678-1234-1234-1234-123456789abd"
#define CHAR_UUID_DATA "12345678-1234-1234-1234-123456789abe"
#define CHAR_UUID_CONFIG "12345678-1234-1234-1234-123456789abf"

// ── Emergency SMS contacts (E.164 format) ─────────────────────
static const char *CONTACTS[] = {"+919876543210", "+919876543211"};
static const int CONTACT_COUNT = 2;

// ── Detection ───────────────────────────────────────────────────
#define SAMPLE_RATE 100
#define WINDOW_SIZE 200
#define SEIZURE_MAG_THR 2.5f
#define ZCR_LOW 6.0f
#define ZCR_HIGH 16.0f
#define CONFIDENCE_THR 0.80f
#define CANCEL_WINDOW_MS 5000
#define MIN_DURATION_MS 10000
#define GPS_STALE_MS 30000

// SIM800L cell-tower location when phone GPS / BLE unavailable (rough, ~100 m–2 km)
#define ENABLE_GSM_LBS 1
#define GSM_LBS_TIMEOUT_MS 15000
// Carrier APN for CIPGSMLOC fallback ("" = default; set e.g. "airtelgprs.com", "internet", "jionet")
static const char GSM_APN[] = "";
#define ML_CONFIDENCE_BLEND 0.35f

// Your bench-tested HR + motion rules (from standalone sketch)
#define USE_HARDWARE_LOGIC 1
#define HARDWARE_TICK_MS 20
#define ML_ALERT_THR 0.65f
