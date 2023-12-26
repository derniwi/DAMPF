/*
Preferences.h

Some data can be stored in ESP32 preferences.
Provide functions to read and write data.

2023-12-18: Initial version
*/

#ifndef PREFS_H
#define PREFS_H

void savePreferencesBTSerLogToSD(unsigned char logToSD);
unsigned char readPreferencesBTSerLogToSD();

void savePreferencesSerLogType(unsigned char logType);
unsigned char readPreferencesSerLogType();

void savePreferencesSerSpeed(unsigned char serSpeedIdx);
unsigned char readPreferencesSerSpeed();

void savePreferencesSerConfig(unsigned char serCfgIdx);
unsigned char readPreferencesSerConfig();

void savePreferencesDefaultFunction(unsigned char defaultFunction);
unsigned char readPreferencesDefaultFunction();

void savePreferencesOrientation(unsigned char orientation);
unsigned char readPreferencesOrientation(void);

void savePreferencesDelay(unsigned char delay);
unsigned char readPreferencesDelay();
#endif
