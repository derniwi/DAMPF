/*
Definitions.h

This file contains Global definitions for the project which might used in several files included.
2023-12-18: Initial version
*/

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// Language definition
// Since the preprocessor can not compare strings easily the workaround with the double
// definition is used. This is used in "Language.h".
// First define all supported languages and assign them a unique number
#define LANG_EN 0
#define LANG_DE 1

// Second, assign the language macro to the language shortcut
#define LANGUAGE LANG_DE
//#define LANGUAGE LANG_EN

// Debug send LLDP-MED packet
// If this is active the LLDP-MMED packet will handled also as incoming packet which will help for debugging the packet itself
//#define DEBUGSENDLLDP

// Debug using ESP32 serial interface
// Activate to use the USB-Serial connection for outputting data to the Arduino Serial Monitor
#define DEBUGSERIAL

// Use Bluetooth to hardware serial
// Activate for using BlueTooth serial connection (Serial A)
#define USE_BTSERIAL

// Activate to us a SD card / Micro SD card
#define USE_SDCARD

#ifdef USE_SDCARD
// Activate for logging data between two serial ports. This is only usefull if a SD card
// is used. If the SD card support is disabled above it makes no sense to have the
// serial logging eabled.
#define USE_SERIALLOGGER
#endif

// Activate to use a DS3231 battery buffer realt time clock
#define USE_RTCTIME

// Analog digital converter pin for battery monitoring (ADC, input only is ok)
// Use a voltage divider to get 3.3v from the 4.2v:
// Battery + connector to 27kOhm to ADC and 100 kOhm to ground
// https://esp32.com/viewtopic.php?t=769
#define BAT_ADC_PIN 34

// Buttons
#define BTN_BUTTON1 36
#define BTN_BUTTON2 39

// Serial A pins used by BlueTooth serial and/or serial data logging
#if defined(USE_BTSERIAL) || defined(USE_SERIALLOGGER)
#define SER_RXPIN1 9
#define SER_TXPIN1 10
#endif

// Serial data logger uses UART with this pins
#ifdef USE_SERIALLOGGER
#define SER_RXPIN2 16
#define SER_TXPIN2 17
#endif

// Other definitions
#if defined(USE_BTSERIAL) || defined(USE_SERIALLOGGER)
// Buffer for the UART to serial port(s)
#define SER_BUFFER 1278
#endif

// HSPI pins for SD card and TFT
#define HSPI_SCLK 14
#define HSPI_MISO 25
#define HSPI_MOSI 13
#ifdef USE_SDCARD
#define HSPI_SD_CS 26  // SD Card chip select
#endif

// CS pin for Ethernet
#define ETH_CS 5

// Maxiumum numbers of NTP sources to support/check
#define ETH_NTPMAXSOURCES 10

#ifdef USE_SDCARD
// Network log file
#define SD_LOGFILENAME "/dampf.log"

// VLAN name resolution file, file has to be ASCII/ANSI
//#define SD_VLANRESFILENAME "/vlan.csv"

// WiFi device name resolution file, file has to be ASCII/ANSI
#define SD_WIFIRESFILENAME "/wifimac.csv"

// Bluetooth Serial log file name
#define SD_BTSERLOGFILENAME "/btserial.log"

// Serial log file name
#define SD_SERLOGFILENAME "/serial.log"
#endif

// DAMPF functions
enum eFunction {
  fNone = 0,
  fEthernet,         // = 1
  fWiFi,             // = 2
  fBluetoothSerial,  // = 3
  fSerialLogger      // = 4
};
#define EFUNCTIONCOUNT 5

// Inline function for the enumeration so the ++ operator can be used.
inline eFunction& operator++(eFunction& _func, int) {
  int i = (static_cast<int>(_func) + 1) % EFUNCTIONCOUNT;
  // Handle not used functions
#ifndef USE_BTSERIAL
  if (i == fBluetoothSerial)
    i++;
#endif
#ifndef USE_SERIALLOGGER
  if (i == fSerialLogger)
    i++;
#endif
  _func = static_cast<eFunction>(i % EFUNCTIONCOUNT);
  return _func;
}

// Enumeration for Bluetooth serial logging
enum eBTSerLog {
  bt_SerLogNo = 0,
  bt_SerLogYes = 1
};

// Enumeration for serial port logging
enum eSerLog {
  ser_LogASCII = 0,
  ser_LogHEX = 1,
};


// Include HardwareSerial.h for configuration definitions
#if defined(USE_BTSERIAL) || defined(USE_SERIALLOGGER)
#include <HardwareSerial.h>
#endif

// Configuration options for serial connections
// Not everything has been tested, but this seems the maximum of possible and supported values
// const unsigned long ser_speeds[] = { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 74880, 115200, 230400, 256000, 460800, 921600, 1843200, 3686400 };
const unsigned long ser_speeds[] = { 2400, 4800, 9600, 19200, 38400, 57600 };
const unsigned char ser_speedsCount = sizeof(ser_speeds) / sizeof(ser_speeds[0]);
const unsigned char SER_DEFAULTSPEEDIDX = 2;  // Index of ser_speeds array for default value

// Enumeration for serial port configuration
struct sSerConfiguration {
  uint32_t serConfig;
  const char serName[4];
};

// Sources:
// \Arduino15\packages\esp32\hardware\esp32\2.0.14\cores\esp32\esp32-hal-uart.h
// https://en.wikipedia.org/wiki/Serial_port
// Data Bits:
// 5 to 8 are only supported by ESP32
// Parity:
// None (N) means that no parity bit is sent and the transmission is shortened.
// Odd (O) means that the parity bit is set so that the number of 1 bits is odd.
// Even (E) means that the parity bit is set so that the number of 1 bits is even.
// Stop Bits:
// 1 or 2 stop bits
// Why a 32 bit value is used is unknown. It is possible to calculate the value
// for the serial configuration but using an arry is easier so only needed
// configurations can be selected. This also is simpler for selecting the right
// configuration from the user menu.
// from esp32-hal-uart.c:
// uart_config.parity = (config & 0x3);          // 00=>N, 01=>-, 10=>E, 11=>O
// uart_config.data_bits = (config & 0xc) >> 2;  // 00=>5, 01=>6, 10=>7, 11=>8
// uart_config.stop_bits = (config & 0x30) >> 4; // 00=>-, 01=>1, 10=>-, 11=>2
// SERIAL_8N1: hex value 0x0800001c
// => binary value 0b0000 1000 0000 0000 0000 0000 0001 1100
//                                                   ss      stop bits
//                                                      dd   data bits
//                                                        pp paritiy bits
const sSerConfiguration ser_configurations[] = {
  //{ SERIAL_5N1, "5N1" },  // 0x08000010 = 0b0000 1000 0000 0000 0000 0000 0001 0000
  //{ SERIAL_5E1, "5E1" },  // 0x08000012 = 0b0000 1000 0000 0000 0000 0000 0001 0010
  //{ SERIAL_5O1, "5O1" },  // 0x08000013 = 0b0000 1000 0000 0000 0000 0000 0001 0011
  //{ SERIAL_6N1, "6N1" },  // 0x08000014 = 0b0000 1000 0000 0000 0000 0000 0001 0100
  //{ SERIAL_6E1, "6E1" },  // 0x08000016 = 0b0000 1000 0000 0000 0000 0000 0001 0110
  //{ SERIAL_6O1, "6O1" },  // 0x08000017 = 0b0000 1000 0000 0000 0000 0000 0001 0111
  { SERIAL_7N1, "7N1" },  // 0x08000018 = 0b0000 1000 0000 0000 0000 0000 0001 1000
  //{ SERIAL_7E1, "7E1" },  // 0x0800001a = 0b0000 1000 0000 0000 0000 0000 0001 1010
  //{ SERIAL_7O1, "7O1" },  // 0x0800001b = 0b0000 1000 0000 0000 0000 0000 0001 1011
  { SERIAL_8N1, "8N1" },  // 0x0800001c = 0b0000 1000 0000 0000 0000 0000 0001 1100
  //{ SERIAL_8E1, "8E1" },  // 0x0800001e = 0b0000 1000 0000 0000 0000 0000 0001 1110
  //{ SERIAL_8O1, "8O1" },  // 0x0800001f = 0b0000 1000 0000 0000 0000 0000 0001 1111
  //{ SERIAL_5N2, "5N2" },  // 0x08000030 = 0b0000 1000 0000 0000 0000 0000 0011 0000
  //{ SERIAL_5E2, "5E2" },  // 0x08000032 = 0b0000 1000 0000 0000 0000 0000 0011 0010
  //{ SERIAL_5O2, "5O2" },  // 0x08000033 = 0b0000 1000 0000 0000 0000 0000 0011 0011
  //{ SERIAL_6N2, "6N2" },  // 0x08000034 = 0b0000 1000 0000 0000 0000 0000 0011 0100
  //{ SERIAL_6E2, "6E2" },  // 0x08000036 = 0b0000 1000 0000 0000 0000 0000 0011 0110
  //{ SERIAL_6O2, "6O2" },  // 0x08000037 = 0b0000 1000 0000 0000 0000 0000 0011 0111
  //{ SERIAL_7N2, "7N2" },  // 0x08000038 = 0b0000 1000 0000 0000 0000 0000 0011 1000
  //{ SERIAL_7E2, "7E2" },  // 0x0800003a = 0b0000 1000 0000 0000 0000 0000 0011 1010
  //{ SERIAL_7O2, "7O2" },  // 0x0800003b = 0b0000 1000 0000 0000 0000 0000 0011 1011
  //{ SERIAL_8N2, "8N2" },  // 0x0800003c = 0b0000 1000 0000 0000 0000 0000 0011 1100
  //{ SERIAL_8E2, "8E2" },  // 0x0800003e = 0b0000 1000 0000 0000 0000 0000 0011 1110
  //{ SERIAL_8O2, "8O2" },  // 0x0800003f = 0b0000 1000 0000 0000 0000 0000 0011 1111
};
const unsigned char ser_configurationsCount = sizeof(ser_configurations) / sizeof(ser_configurations[0]);
const unsigned char SER_DEFAULTCONFIGURATIONIDX = 1;  // Index of ser_configurations array for default value
#endif
