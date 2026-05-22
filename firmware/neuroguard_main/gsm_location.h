#pragma once

#include <HardwareSerial.h>

// Rough lat/lng from SIM800L cell towers (LBS). Accuracy ~100 m–2 km.
bool gsmFetchRoughLocation(HardwareSerial &gsm, float *lat, float *lng);
