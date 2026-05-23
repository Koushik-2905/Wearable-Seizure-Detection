#include "gsm_location.h"
#include "config.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#if ENABLE_GSM && !SIMULATION_MODE

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

static void gsmLogStatus(HardwareSerial &gsm, char *buf, size_t bufLen) {
  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CSQ");
  gsmCollect(gsm, buf, bufLen, 2000);
  Serial.print("[GSM] ");
  Serial.println(buf);

  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CPIN?");
  gsmCollect(gsm, buf, bufLen, 2000);
  Serial.print("[GSM] ");
  Serial.println(buf);

  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CREG?");
  gsmCollect(gsm, buf, bufLen, 4000);
  Serial.print("[GSM] ");
  Serial.println(buf);
}

// +CREG: 0,1 home | 0,5 roaming = OK. 0,2 searching | 0,0 not registered.
static int gsmCregStat(const char *resp) {
  const char *p = strstr(resp, "+CREG:");
  if (!p) return -1;
  int n = 0, stat = -1;
  if (sscanf(p, "+CREG: %d,%d", &n, &stat) >= 2) return stat;
  return -1;
}

static bool gsmRegistered(HardwareSerial &gsm, char *buf, size_t bufLen) {
  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CREG?");
  gsmCollect(gsm, buf, bufLen, 4000);
  int stat = gsmCregStat(buf);
  return stat == 1 || stat == 5;
}

static bool gsmWaitForNetwork(HardwareSerial &gsm, char *buf, size_t bufLen, unsigned long timeoutMs) {
  unsigned long t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (gsmRegistered(gsm, buf, bufLen)) return true;
    Serial.println("[GSM] Waiting for network registration...");
    gsmLogStatus(gsm, buf, bufLen);
    delay(3000);
  }
  return false;
}

bool gsmInitModule(HardwareSerial &gsm) {
  static char buf[256];
  bool atOk = false;

  for (int i = 0; i < 5; i++) {
    gsmDrain(gsm);
    gsmCmd(gsm, "AT");
    gsmCollect(gsm, buf, sizeof(buf), 1500);
    if (strstr(buf, "OK") != nullptr) {
      atOk = true;
      break;
    }
    delay(400);
  }

  if (!atOk) {
    Serial.println("[GSM] No AT response — check wiring: ESP RX=GPIO16<-SIM TX, ESP TX=GPIO17->SIM RX");
    return false;
  }

  gsmCmd(gsm, "AT+CFUN=1");
  gsmCollect(gsm, buf, sizeof(buf), 5000);
  gsmCmd(gsm, "AT+CMGF=1");
  gsmCollect(gsm, buf, sizeof(buf), 2000);
  gsmCmd(gsm, "AT+CREG=1");
  gsmCollect(gsm, buf, sizeof(buf), 2000);

  gsmDrain(gsm);
  gsmCmd(gsm, "AT+CPIN?");
  gsmCollect(gsm, buf, sizeof(buf), 3000);
  if (strstr(buf, "READY") == nullptr) {
    Serial.print("[GSM] SIM not ready: ");
    Serial.println(buf);
    Serial.println("[GSM] Check SIM inserted, PIN disabled, or valid prepaid plan");
  }

  if (gsmWaitForNetwork(gsm, buf, sizeof(buf), GSM_REG_WAIT_MS)) {
    Serial.println("[GSM] Registered on cellular network — SMS + LBS ready");
    return true;
  }

  Serial.println("[GSM] Not registered yet — SMS will retry at alert time");
  gsmLogStatus(gsm, buf, sizeof(buf));
  return true;
}

static bool gsmWaitChar(HardwareSerial &gsm, char want, unsigned long timeoutMs) {
  unsigned long t0 = millis();
  while (millis() - t0 < timeoutMs) {
    while (gsm.available()) {
      if ((char)gsm.read() == want) return true;
    }
    delay(5);
  }
  return false;
}

static void gsmEndDataSession(HardwareSerial &gsm, char *buf, size_t bufLen) {
  gsmDrain(gsm);
  gsmCmd(gsm, "AT+SAPBR=0,1");
  gsmCollect(gsm, buf, bufLen, 3000);
  gsmCmd(gsm, "AT+CMGF=1");
  gsmCollect(gsm, buf, bufLen, 2000);
}

bool gsmSendTextSms(HardwareSerial &gsm, const char *num, const char *msg) {
  static char buf[384];
  static char text[161];

  if (num == nullptr || msg == nullptr || num[0] == '\0') return false;

  strncpy(text, msg, 160);
  text[160] = '\0';

  Serial.println("[GSM] Preparing SMS — checking network...");
  if (!gsmWaitForNetwork(gsm, buf, sizeof(buf), 45000)) {
    Serial.println("[GSM] SMS skipped — not registered (see CSQ/CREG above)");
    gsmLogStatus(gsm, buf, sizeof(buf));
    return false;
  }

  gsmEndDataSession(gsm, buf, sizeof(buf));

  gsm.print("AT+CMGS=\"");
  gsm.print(num);
  gsm.println("\"");
  if (!gsmWaitChar(gsm, '>', 15000)) {
    Serial.println("[GSM] SMS failed — no '>' prompt (module busy or wrong number)");
    gsmCollect(gsm, buf, sizeof(buf), 2000);
    Serial.println(buf);
    return false;
  }

  gsm.print(text);
  gsm.write(26);
  gsmCollect(gsm, buf, sizeof(buf), 15000);

  Serial.print("[GSM] SMS response: ");
  Serial.println(buf);

  if (strstr(buf, "ERROR") != nullptr || strstr(buf, "+CMS ERROR") != nullptr) {
    return false;
  }
  return strstr(buf, "OK") != nullptr || strstr(buf, "+CMGS") != nullptr;
}

#if ENABLE_GSM_LBS

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

#else  // ENABLE_GSM_LBS

bool gsmFetchRoughLocation(HardwareSerial &gsm, float *lat, float *lng) {
  (void)gsm;
  (void)lat;
  (void)lng;
  return false;
}

#endif  // ENABLE_GSM_LBS

#else  // ENABLE_GSM && !SIMULATION_MODE

bool gsmInitModule(HardwareSerial &gsm) {
  (void)gsm;
  return false;
}

bool gsmSendTextSms(HardwareSerial &gsm, const char *num, const char *msg) {
  (void)gsm;
  (void)num;
  (void)msg;
  return false;
}

bool gsmFetchRoughLocation(HardwareSerial &gsm, float *lat, float *lng) {
  (void)gsm;
  (void)lat;
  (void)lng;
  return false;
}

#endif
