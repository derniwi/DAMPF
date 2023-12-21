/*
Texts.h

This file contains universal text definitions as well as language dependent.
2023-12-18: Initial version
*/

#include "Definitions.h"

#ifndef TEXTS_H
#define TEXTS_H

// General text definitions
static const char* TXT_GEN_DEVNAME = "DAMPF";        // General device name
static const char* TXT_GEN_VERSION = "v1.0 (2023)";  // General version string
static const char* TXT_GEN_OWNER = "Nils Winkler";   // General owner

// Language dependent texts
#if LANGUAGE == LANG_DE
static const char* TXT_GEN_PROPERTYOF = "Eigentum von";
static const char* TXT_GEN_ON = "an";
static const char* TXT_GEN_OFF = "aus";
static const char* TXT_GEN_NONE = "Kein";

// User menu
static const char* TXT_GEN_ETHERNET = "Ethernet";
static const char* TXT_WIFI_NAME = "WLAN";
static const char* TXT_BT_SERIAL = "BT seriell";
static const char* TXT_BT_LOGTOSD = " auf SD prot.";
static const char* TXT_SER_SERLOGGING = "Ser prot.";
static const char* TXT_SER_LOGTYPE = " Typ";
static const char* TXT_SER_SPEED = "Baud";
static const char* TXT_SER_CONFIG = "SerKonf";
static const char* TXT_GEN_DEFAULTFUNCTION = "Std";
static const char* TXT_GEN_WRITETOLOG = "Protokoll speich.";
static const char* TXT_GEN_ROTATESCREEN = "Bildschirm drehen";
static const char* TXT_GEN_SCREENSWITCHDELAY = "Pause";

#ifdef USE_BTSERIAL
static const char* TXT_BT_CONNECTION = "Bluetooth-Verbindung";
static const char* TXT_BT_CONNACCEPT = "Button 1: annehmen";
static const char* TXT_BT_CONNDECLINE = "Button 2: ablehnen";
#endif

// Ethernet
static const char* TXT_ETH_NTPSERVER = "NTP Server";

// WiFi
static const char* TXT_WIFI_ENCRYPT = "Enc:";
static const char* TXT_WIFI_STRENGTH = "Staerke: ";
static const char* TXT_WIFI_NOTFOUND = "Nicht gefunden";

// Battery

// SD card
static const char* TXT_SD_CARD = "SD Karte";
static const char* TXT_SD_CARDFREE = "frei";
static const char* TXT_SD_NOTAVAILABLE = "Keine SD Karte";

// Serial logging
static const char* TXT_SERLOGGING_ASCII = "ASCII";
static const char* TXT_SERLOGGING_HEX = "HEX";

#else
//#elif LANGUAGE == LANG_EN
// use english as default if no other language is define
static const char* TXT_GEN_PROPERTYOF = "Property of";
static const char* TXT_GEN_ON = "on";
static const char* TXT_GEN_OFF = "off";
static const char* TXT_GEN_NONE = "None";

// User menu
static const char* TXT_GEN_ETHERNET = "Ethernet";
static const char* TXT_WIFI_NAME = "WiFi";
static const char* TXT_BT_SERIAL = "BT serial";
static const char* TXT_BT_LOGTOSD = " Log to SD";
static const char* TXT_SER_SERLOGGING = "Ser Logging";
static const char* TXT_SER_LOGTYPE = " Type";
static const char* TXT_SER_SPEED = "Baud";
static const char* TXT_SER_CONFIG = "Ser Cfg";
static const char* TXT_GEN_DEFAULTFUNCTION = "Def";
static const char* TXT_GEN_WRITETOLOG = "Write to log";
static const char* TXT_GEN_ROTATESCREEN = "Rotate screen";
static const char* TXT_GEN_SCREENSWITCHDELAY = "Delay";

#ifdef USE_BTSERIAL
static const char* TXT_BT_CONNECTION = "Bluetooth connection";
static const char* TXT_BT_CONNACCEPT = "Button 1: accept";
static const char* TXT_BT_CONNDECLINE = "Button 2: decline";
#endif

// Ethernet
static const char* TXT_ETH_NTPSERVER = "NTP Server";

// WiFi
static const char* TXT_WIFI_ENCRYPT = "Enc:";
static const char* TXT_WIFI_STRENGTH = "Strength: ";
static const char* TXT_WIFI_NOTFOUND = "not found";

// Battery

// SD card
static const char* TXT_SD_CARD = "SD Card";
static const char* TXT_SD_CARDFREE = "free";
static const char* TXT_SD_NOTAVAILABLE = "No SD card available";

// Serial logging
static const char* TXT_SERLOGGING_ASCII = "ASCII";
static const char* TXT_SERLOGGING_HEX = "HEX";

#endif

#endif
