#pragma once

#include <HardwareSerial.h>

// SIM800L: SMS + cell-tower location (see config.h GSM_RX / GSM_TX)
bool gsmInitModule(HardwareSerial &gsm);
bool gsmSendTextSms(HardwareSerial &gsm, const char *num, const char *msg);
bool gsmFetchRoughLocation(HardwareSerial &gsm, float *lat, float *lng);
