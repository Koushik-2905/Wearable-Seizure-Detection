#include "gsm_location.h"
#include "config.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#if ENABLE_GSM && ENABLE_GSM_LBS

static void gsmDrain(HardwareSerial &gsm) {
  unsigned long t = millis();
  while (millis() - t < 80) {
    while (gsm.available()) gsm.read();
    delay(2);
  }
}

static void gsmCmd(HardwareSerial &gsm, const char *cmd) {
  gsm.print(cmd);
  gsm.print("\r\n");
}

static size_t gsmCollect(HardwareSerial &gsm, char *buf, size_t bufLen, unsigned long timeoutMs) {
  size_t pos = 0;
  unsigned long lastByte = millis();
  unsigned long start = millis();

  buf[0] = '\0';
  while (millis() - start < timeoutMs) {
    while (gsm.available() && pos < bufLen - 1) {
      buf[pos++] = (char)gsm.read();
      buf[pos] = '\0';
      lastByte = millis();
    }
    if (pos > 0 && millis() - lastByte > 350) break;
    delay(5);
  }
  return pos;
}

static bool gsmRegistered(HardwareSerial &gsm, char *buf, size_t bufLen) {
  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CREG?");
  gsmCollect(gsm, buf, bufLen, 4000);
  return strstr(buf, ",1") != nullptr || strstr(buf, ",5") != nullptr;
}

static bool parseClbs(const char *resp, float *lat, float *lng) {
  const char *p = strstr(resp, "+CLBS:");
  if (!p) return false;
  int err = 1;
  double la = 0, ln = 0;
  if (sscanf(p, "+CLBS: %d,%lf,%lf", &err, &la, &ln) >= 3 && err == 0) {
    if (la >= -90.0 && la <= 90.0 && ln >= -180.0 && ln <= 180.0 && (la != 0.0 || ln != 0.0)) {
      *lat = (float)la;
      *lng = (float)ln;
      return true;
    }
  }
  return false;
}

static bool parseCipgsmloc(const char *resp, float *lat, float *lng) {
  const char *p = strstr(resp, "+CIPGSMLOC:");
  if (!p) return false;
  int err = 1;
  double la = 0, ln = 0;
  if (sscanf(p, "+CIPGSMLOC: %d,%lf,%lf", &err, &la, &ln) >= 3 && err == 0) {
    if (la >= -90.0 && la <= 90.0 && ln >= -180.0 && ln <= 180.0 && (la != 0.0 || ln != 0.0)) {
      *lat = (float)la;
      *lng = (float)ln;
      return true;
    }
  }
  return false;
}

static bool gsmTryClbs(HardwareSerial &gsm, float *lat, float *lng, char *buf, size_t bufLen) {
  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CLBS=1,1");
  gsmCollect(gsm, buf, bufLen, GSM_LBS_TIMEOUT_MS);
  Serial.print("[GSM-LBS] CLBS response: ");
  Serial.println(buf);
  return parseClbs(buf, lat, lng);
}

static bool gsmTryCipgsmloc(HardwareSerial &gsm, float *lat, float *lng, char *buf, size_t bufLen) {
  gsmDrain(gsm);
  gsmCmd(gsm, "AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  gsmCollect(gsm, buf, bufLen, 2000);

  gsmDrain(gsm);
  if (GSM_APN[0] != '\0') {
    char apnCmd[64];
    snprintf(apnCmd, sizeof(apnCmd), "AT+SAPBR=3,1,\"APN\",\"%s\"", GSM_APN);
    gsmCmd(gsm, apnCmd);
  } else {
    gsmCmd(gsm, "AT+SAPBR=3,1,\"APN\",\"\"");
  }
  gsmCollect(gsm, buf, bufLen, 2000);

  gsmDrain(gsm);
  gsmCmd(gsm, "AT+SAPBR=1,1");
  gsmCollect(gsm, buf, bufLen, 5000);

  gsmDrain(gsm);
  gsmCmd(gsm, "AT+SAPBR=2,1");
  gsmCollect(gsm, buf, bufLen, 3000);

  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CIPGSMLOC=1,1");
  gsmCollect(gsm, buf, bufLen, GSM_LBS_TIMEOUT_MS);
  Serial.print("[GSM-LBS] CIPGSMLOC response: ");
  Serial.println(buf);

  gsmCmd(gsm, "AT+SAPBR=0,1");
  gsmCollect(gsm, buf, bufLen, 2000);

  return parseCipgsmloc(buf, lat, lng);
}

bool gsmFetchRoughLocation(HardwareSerial &gsm, float *lat, float *lng) {
  static char buf[512];

  gsmCmd(gsm, "AT");
  gsmCollect(gsm, buf, sizeof(buf), 1500);

  if (!gsmRegistered(gsm, buf, sizeof(buf))) {
    Serial.println("[GSM-LBS] Not registered on network (CREG)");
    return false;
  }

  if (gsmTryClbs(gsm, lat, lng, buf, sizeof(buf))) {
    Serial.println("[GSM-LBS] OK via CLBS (cell tower)");
    return true;
  }

  if (gsmTryCipgsmloc(gsm, lat, lng, buf, sizeof(buf))) {
    Serial.println("[GSM-LBS] OK via CIPGSMLOC (network)");
    return true;
  }

  Serial.println("[GSM-LBS] Failed — no cell location");
  return false;
}

#else

bool gsmFetchRoughLocation(HardwareSerial &gsm, float *lat, float *lng) {
  (void)gsm;
  (void)lat;
  (void)lng;
  return false;
}

#endif
