/*
Preferences.cpp

Some data can be stored in ESP32 preferences.
Provide functions to read and write data.

2023-12-18: Initial version
*/

#include "Definitions.h"
#include <EtherCard.h>
#include <Preferences.h>
#include "prefs.h"

// Handle BLuetooth serial logging to disk
void savePreferencesBTSerLogToSD(unsigned char logToSD) {
  Preferences preferences;
  preferences.begin("DAMPF", false);
  preferences.putUChar("BTLOGTOSD", logToSD);
  preferences.end();
}
unsigned char readPreferencesBTSerLogToSD() {
  Preferences preferences;
  unsigned char bt_logToSD = bt_SerLogNo;
  preferences.begin("DAMPF", true);
  if (preferences.isKey("BTLOGTOSD")) {
    bt_logToSD = preferences.getUChar("BTLOGTOSD", bt_SerLogNo);
    if ((bt_logToSD != bt_SerLogNo) && (bt_logToSD != bt_SerLogYes))
      bt_logToSD = bt_SerLogNo;
  }
  return bt_logToSD;
}

// Handle serial logging type
void savePreferencesSerLogType(unsigned char logType) {
  Preferences preferences;
  preferences.begin("DAMPF", false);
  preferences.putUChar("SERLOGTYPE", logType);
  preferences.end();
}
unsigned char readPreferencesSerLogType() {
  Preferences preferences;
  unsigned char ser_logType = ser_LogASCII;
  preferences.begin("DAMPF", true);
  if (preferences.isKey("SERLOGTYPE")) {
    ser_logType = preferences.getUChar("SERLOGTYPE", ser_LogASCII);
    if ((ser_logType != ser_LogASCII) && (ser_logType != ser_LogHEX))
      ser_logType = ser_LogASCII;
  }
  return ser_logType;
}

// Handle serial port speed / baud rate index
void savePreferencesSerSpeed(unsigned char serSpeedIdx) {
  Preferences preferences;
  preferences.begin("DAMPF", false);
  preferences.putUChar("SERSPEED", serSpeedIdx);
  preferences.end();
}
unsigned char readPreferencesSerSpeed() {
  Preferences preferences;
  unsigned char ser_speed = SER_DEFAULTSPEEDIDX;
  preferences.begin("DAMPF", true);
  if (preferences.isKey("SERSPEED")) {
    ser_speed = preferences.getUChar("SERSPEED", SER_DEFAULTSPEEDIDX);
    if (ser_speed >= ser_speedsCount)
      ser_speed = SER_DEFAULTSPEEDIDX;
  }
  return ser_speed;
}

// Handle serial configuration
void savePreferencesSerConfig(unsigned char serCfgIdx) {
  Preferences preferences;
  preferences.begin("DAMPF", false);
  preferences.putUChar("SERCONFIG", serCfgIdx);
  preferences.end();
}
unsigned char readPreferencesSerConfig() {
  Preferences preferences;
  unsigned char ser_cfg = SER_DEFAULTCONFIGURATIONIDX;
  preferences.begin("DAMPF", true);
  if (preferences.isKey("SERCONFIG")) {
    ser_cfg = preferences.getUChar("SERCONFIG", SER_DEFAULTCONFIGURATIONIDX);
    if (ser_cfg >= ser_configurationsCount)
      ser_cfg = SER_DEFAULTCONFIGURATIONIDX;
  }
  return ser_cfg;
}

// Handle default function
void savePreferencesDefaultFunction(unsigned char defaultFunction) {
  Preferences preferences;
  preferences.begin("DAMPF", false);
  preferences.putUChar("DEFFUNCTION", defaultFunction);
  preferences.end();
}
unsigned char readPreferencesDefaultFunction() {
  Preferences preferences;
  unsigned char gen_defFunction = fNone;
  preferences.begin("DAMPF", true);
  if (preferences.isKey("DEFFUNCTION")) {
    gen_defFunction = preferences.getUChar("DEFFUNCTION", fNone);
    if (gen_defFunction >= EFUNCTIONCOUNT)
      gen_defFunction = fNone;

#ifndef USE_BTSERIAL
    if (gen_defFunction == fBluetoothSerial)
      gen_defFunction = fNone;
#endif
#ifndef USE_SERIALLOGGER
    if (gen_defFunction == fSerialLogger)
      gen_defFunction = fNone;
#endif
  }
  return gen_defFunction;
}

// Handle screen orientation
void savePreferencesOrientation(unsigned char orientation) {
  Preferences preferences;
  preferences.begin("DAMPF", false);
  preferences.putUChar("ORIENTATION", orientation);
  preferences.end();
}
unsigned char readPreferencesOrientation() {
  Preferences preferences;
  unsigned char orientation = 0;
  preferences.begin("DAMPF", true);
  if (preferences.isKey("ORIENTATION")) {
    orientation = preferences.getUChar("ORIENTATION", 0);
    if (orientation > 3)
      orientation = 0;
  }
  return orientation;
}

// Handle auto switching delay
void savePreferencesDelay(unsigned char delay) {
  Preferences preferences;
  preferences.begin("DAMPF", false);
  preferences.putUChar("DELAY", delay);
  preferences.end();
}
unsigned char readPreferencesDelay() {
  Preferences preferences;
  uint8_t tempDelay = 10;
  preferences.begin("DAMPF", true);
  if (preferences.isKey("DELAY")) {
    tempDelay = preferences.getUChar("DELAY", 10);
    if (tempDelay > 30)
      tempDelay = 10;
  }
  return tempDelay;
}
