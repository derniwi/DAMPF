/*
 * DAMPF = Des Armen Mannes Protokoll Freund
 * 
 * based on
 * https://modlog.net/esp32-cdp-lldp-dhcp-third-edition/
 * https://deepbluembedded.com/esp32-wifi-scanner-example-arduino/?utm_content=cmp-true
 *
 * Using Arduino IDE v2.2.1
 * Using ESP Arduino v2.0.14
 * Settings:
 * Board: ESP32 / Wemos D1 Mini ESP32
 * CPU frequency: 240 MHz (WiFi/BT)
 * Core Debung Level: "none"
 * Erase all Flash before sketch upload: disabled
 * Flash Frequency: 80MHz
 * Partition Scheme: No OTA (Large APP)
 * Upload Speed: 921600
 *
 * Used libraries:
 * - Button2 2.2.4 by Lennart
 * - ESP32Time 2.0.4 by fbiego
 * - EtherCard 1.1.0 by Jean-Claude Wippler, modified by Kristian an Nils
 * - LinkedList 1.3.3 by Ivan Seidel
 * - RTCLib 2.1.3 by Adafruit
 * - SD 1.2.4, by Arduino, Sparkfun
 * - TFT_eSPI 2.5.34 by Bodmer
 *
 * Hardware:
 * - 1x Wemos D1 Mini ESP32
 * - 1x Ethernet moule ENC28J60
 * - 1x 1.54 Inch Full Color IPS LCD Display Module 240x240 SPI Interface ST7789 8 Pin
 * - 1x 16850 LI Ion rechargable battery
 * - 1x 50-500mA adjustable charge controller LiPo/Li-Ion battery 3.7V-4.2V Micro USB charger
 * - 1x Pololu 3.3V Step-Up/Step-Down Voltage Regulator S8V9F3
 * - 2x UART to RS232 modules (used MAX3232 dual module and one DB9 female and one DB9 male connector)
 * - 1x DS3231 RTC clock
 * - 1x Micro SD card module
 * - 1x Micro SD card FAT32 formatted (max 32GB supported on most systems)
 * - 2x micro push buttons
 * - 1x micro 6-pole switch (switching the Li Ion battery between charger and DAMPF hardware so during
 *   the charge process no power will be supplied to DAMPF)
 *
 * ESP-WROOM32 pinout and usage:
 * https://www.electronicshub.org/esp32-pinout/
 *
 * Connections:
 * ESP32   ST7789 (Display)
 * IO27     BLK              (backlight control)
 * IO15     CS    HSPI:CS    (cable select)
 * IO32     DC               (data/command)
 * RST      RES              (reset)         
 * IO13     SDA   HSPI:MOSI
 * IO14     SCL   HSPI:SCLK  (clock)
 * IO25 not connected to TFT  HSPI:MISO
 * TFT display pins (left to right):
 * GND VCC SCL SDA RES DC CS BLK
 *
 * SD Card:
 * IO25     MISO 
 * IO14     SCLK
 * IO13     MOSI
 * IO26     CS (cable select)
 *
 * ESP32 for measuring voltage
 * IO34 is the ADC and reads the voltage so it should be connected to a voltage divider between the battery + and -
 * The used voltage divider is R1=300kOhm to R2=1MOhm
 *  /-----\
 *  |     |
 *  +     R1
 * Bat    +--- Uref => to IO34
 *  -     R2
 *  |     |
 *  \-----+--- Gnd ESP32
 *
 * Buttons (use an additional 10kOhm resistor for pullup):
 * Button 1: 36
 * Button 2: 39
 *
 * UART to Serial converter based on MAX3232
 * First hardware serial connection:
 * RX_1:  9
 * TX_1: 10
 *
 * Second hardware serial connection:
 * RX_2: 16
 * TX_2: 17
 *
 * RTC DS3231:
 * IO22: I2C0:SCL
 * IO21: I2C0:SDA
 *
 * ESP32   Enc28J60 (Ethernet module, uses HSPI)
 *  IO5     CS  (Cable Select)
 * IO23     SI  (MOSI)
 * IO18     SCK
 * IO19     SO  (MISO)
 *
 * D1 Mini ESP32 Pins (e: Ethernet, L: LCD, b: button, P: Power/ADC, R: RTC, 1: UART1, 2: UART2, s: SD-Card, !: reserved)
 *  GND       RST(L)         IO1(!)   GND
 *   NC(!)   IO36(b)         IO3(!)  IO27(L)
 * IO39(b)   IO26(s)        IO22(R)  IO25(s)
 * IO35      IO18(e)        IO21(R)  IO32(L)
 * IO33      IO19(e)        IO17(2)  IO12(!)
 * IO34(P)   IO23(e)        IO16(2)   IO4
 * IO14(L,s)  IO5(e)         GND      IO0
 *   NC(!)    3.3V            5V      IO2(!) => onboard LED
 *  IO9(1)   IO13(L,s)      IO15(L)   IO8(!)
 * IO11(!)   IO10(1)         IO7(!)   IO6(!)
 *
 * Other descriptions for pins:
 * GND = Ground
 * NC = not connected
 * IO39 = SVN (Sensor_VN) = ADC3 => input only
 * IO35 = ADC1 => input only
 * IO33 = ADC5 = Touch8
 * IO34 = ADC0 => input only
 * IO14 = TMS = HSPI:SCK/CLK
 * NC
 * IO9 = SD2 = UART1:RxD
 * IO11 = CMD
 *
 * RST = Reset
 * IO36 = SVP = ADC2
 * IO2
 * IO18 = SPI0:SCK/CLK
 * IO19 = SPI0:MISO
 * IO23 = SPI0:MOSI
 * IO5 = SPI0:CS0
 * IO13 = TCK = HSPI:MOSI
 * IO10 = SD3 = UART1:TxD
 *
 * IO1 = TXD = UART0:TxD
 * IO3 = RXD = UART0:RxD
 * IO22 = I2C0:SCL
 * IO21 = I2C0:SDA
 * IO17 = UART2:TxD
 * IO16 = UART2:RxD
 * GND
 * VCC = 5V
 * IO15 = TD0 = HSPI:CS
 * IO7 = SD0
 *
 * GND
 * IO27
 * IO25 = DAC0
 * IO32
 * IO12 = TDI = HSPI:MISO
 * IO4 = PWM0:2
 * IO0 = PWM0:1
 * IO2 = PWM0:0 = onboard LED
 * IO8 = SD1
 * IO6 = CLK
 *
 * Note:
 * Pins 34, 35, 36 and 39 are input only!
 * 
 * several modifictaions have been done:
 * - added compiler directive for debugging:
 *   #define DEBUGSERIAL
 * - added compiler directive for serial Bluetooth connection and hardware UART-seriell converter:
 *   #define USE_BTSERIAL
 * - code cleanup: comment/delete unused parts in Ethercard-ESP library
 * - added command line support for internal serial port to simulate button presses
 * - restructure menu and display options
 * - redesign TFT screen
 * - moved DHCP setup into the main file so it will not block receiving broadcast packets
 * - code cleanup (.ino and other files in this folder, EtherCard library)
 * - Updated Ethercard library to be closer to current version, split files into Ethercard, bufferfiller and stash
 *   replaced fixed value with defined so it should be updated like the latest official version
 * - Scroll through screens every 10 seconds, configurable
 * - added VLAN tagging
 * - added DHCP release
 * - automatic switching to tagged VLAN after receiving a DHCP address and a voice VLAN ID via CDP or LLDP
 * - added more lldp-med fields
 * - add function for creating a LLDP-MED packet
 * - Bluetooth: setPin doesn't seem to work. An alternative is to use the computer as initializer.
 *   Windows 11 displays an ID while trying to connect via BT. This ID can be displayed on the tool
 *   and a button press will either allow or deny the connection.
 *   If a BT connection is requested the display is prioritized
 * - added time receiving from network:
 *   - checking for DHCP option 42 (NTP) and option 3 (default gateway)
 *   - DHCP option 15 (Domain Name) for finding Active Directory Domain Controller, this needs name resoultion
 *   - LLDP and CDP seems not to pass information about a time server, but maybe the switch/router itself act as time server
 *   - currently several tries to get NTP packets reliable failed. Only using directly the NTP part with blocking data,
 *     so while waiting for NTP result, other packets might be dropped.
 *     Since NTP delivers only UTC time format time zones are not supported
 *  - added RTC DS3231 support
 *  - added support for SD card
 * - handle re-connection without reset (unplug und replug network)
 * - included original Button2 library, modified handling
 * - fixed TFT and SD configuration and initialization
 * - updated "TFT_eSPI_ttgo_t-display" version 2.3.54 to current "TFT_eSPI" library
 * - added time receiving from network:
 *   - option 119 (DNS domain search list), currently the DNS lookup is blocking other parts
 *     maybe wait a couple of seconds after connecting before this is evaluated?
 * - added timeout for Bluetooth connection request.
 * - added "Definitions.h" for global definitions
 * - added "Languages.h" for text constants
 * - added user menu
 * - Reconfiguration of pin usage, got SD, Eth and TFT work
 * - added WiFi scan option to scan for a single SSID pressing the second button while displaying WiFis
 *
 * Button 1:
 * short press:
 * - switch through screen
 * - witch to the next menu item
 * - accept incoming BLuetooth connection
 * long press:
 * - enter / leave user menu
 *
 * Button 2:
 * short press:
 * - on WiFi screen select the current and display all MAC addresses or device names providing this SSID
 * - select menu item and change either on / off or cycle thorugh avialable values
 * long press:
 *
 * User menu:
 * - Menu entries:
 *    1) Enable/disable Ethernet
 *    2) Enable/disable WiFi
 *    3) Enable/disable Bluetooth serial (BT serial <=> Serial A)
 *    4)   Log serial data to SD card yes / no
 *    5) Enable/disable serial port logging (Serial A <=> Serial B)
 *    6)   Log ASCII or HEX
 *    7) Serial configuration: Use 9600/8N1 or other
 *    8) Default function: None/Eth/WiFi/BTSerial/SerialLog
 *    9) Write received configuration to SD log file
 *   10) Screen rotation
 *   11) Screen switch delay (5s, 10s, 15s, 20s, 30s) $$$ 0 for turning off?
 * Only one function 1,2,3 or 5 should be active, nothing parallel
 *
 * Start screen:
 * - Tool name            (TXT_GEN_DEVNAME)
 * - Software version     (TXT_GEN_VERSION)
 * - Date and time if available (year > 2000 )
 * - owner of this device (TXT_GEN_PROPERTYOF and TXT_GEN_OWNER)
 * - Used host name
 * - Used MAC address
 * - SD card size if available
 *
 * DHCP screen:
 * - DHCP IP address: eth_dhcpInfo[  0 ], 0 is usually not used
 * - Network mask:    eth_dhcpInfo[  1 ]
 * - Default gateway: eth_dhcpInfo[  3 ]
 * - Server IP:       eth_dhcpInfo[ 54 ]
 * - Lease time:      eth_dhcpInfo[ 51 ]
 * - DNS Domain name: eth_dhcpInfo[ 15 ]
 * - DNS 1, 2, 3:     eth_dhcpInfo[  6 ]
 *
 * Discovery screen (data from both LLDP and CDP):
 * - Switch port number
 * - Switch port description
 * - VLAN ID
 * - Voice VLAN ID
 * - Switch name
 * - Switch domain
 * - Switch IP
 * - Switch MAC address
 * - PoE possible
 * - PoE consumed
 * Second page:  
 * - Switch capabilities
 * - Switch model
 * 
 * 
 * Since LLDP is capable of using several fields as text there is no exact method to
 * find or filter information. As example an AVM Fritz!box also sends LLDP packets
 * but uses the own MAC address as switch port and the connected port in the port
 * description field.
 * 
 * As I was reading many internet pages about LLDP I found that not only switches
 * will send LLDP packets but also a client might. So I connected a VoIP phone to this
 * tool and also saw several packets. I captured with a PC those packets and checked them.
 * Also I found informations in the internet that LLDP is not a handshake protocol so
 * both sides may send standard packets and might react on values or send informtion. So
 * the switch does not send LLDP-MED information if the client doesn't. Therefore I added
 * a function which will send some hard coded values and the switch currently reacts
 * sending such packets as "answer".
 * 
 * Execution sequence:
 * - wait until network connection is available
 * - start DHCP
 * - after DHCP is finished check:
 *   - add NTP server(s) to NTP source list if available => done
 *   - add default gateway to NTP source list => done
 *   - do a NSlookup on the DNS domain, possibly a Active Directory Domain Controller answers NTP requests => done
 *     - add IP addresses to NTP source list if available => the NS lookup from tcpip.cpp returns only one IP address
 *   - do a NSlookp on the Domain name search list if provided, for each entry
 *     - add IP addresses to NTP source list
 * - start NTP request for all IPs
 * - if an IP address replies on a NTP request abort NTP check and set RTCs
 * - release IP address and thy to change to VoiceVLAN / tagged VLAN for seond DHCP request
 *   - on the VoiceVLAN there will no NTP check be done
 *
 *   
 * Ideas / to check / to do:
 * - To Do:
 *   - check for usage of "millis()"
 *   - clean up loop() function an move ethernet parts into processEthernet() function or create several other
 *     new functions to reduce size
 *   - check header entries colors
 *   - check menu entries (which are enabled or disbaled)
 * - General: 
 *   - Use accelerometer to detect up and down side, rotating the display. Two input pins needed, available?
 *   - Use "/vlan.csv" file on SD card for checking the VLAN ID and returning an additional text labeling the VLANs. There
 *     is no system available which delivers a defined text for VLAN IDs.
 * - Power:
 *   - Check if battery / 16850 power is working => check ok
 *   - Use MOSFET for turning of Ethernet module? The Ethernet module can be put to standby
 *   - Use MOSFET for turning of MAX3232?
 * - Ethernet:
 *   - Add / check SPI beginTransaction / endTransaction
 *   - Get time using NTP is currently blocking other traffic
 *     => Rewrite the function either here or in Ethercard library
 *   - NSlookup may return more than one IP address, but this is currently not handled by dns.cpp / Ethercard library.
 *   - DHCP options 4 (Time Server) and 2 (Time Offset) use an other protocol as NTP, currently not in use, check?
 *    - need secondvariable dhcp receviced value for VLAN
 * - WiFI:
 *   - Will CDP / LLDP send informatin over WiFi? nothing found yet...
 *
 * The DHCPOptions, cdp_functions and lldp_functions have also been improved and updated. More data is
 * discovered but it is not used since this tool does not need to discover all of this data and it will
 * also not help a user if to much data is available.
 *
 * Other discovery protocols (currently not supported since I do not have any device):
 * - Extreme: EDP
 * - Alcatel-Lucent: AMAP
 * - Nortel: NDP
 *
 * Formating options for time:
 * http://www.cplusplus.com/reference/ctime/strftime/
 *
 */


// Global definitions used in this project
// This file has to be included first since it will also control the including of
// other libraries. Also shared enumerations are defined here.
#include "Definitions.h"

// Load texts and translations
#include "Texts.h"

// Generic libraries
#include <esp_adc_cal.h>  // Library for analog-digital conversion
#include <WiFi.h>         // WiFi

// Brownout detector
//#include <soc/soc.h>
//#include <soc/rtc_cntl_reg.h>

// Watchdog
#include <soc/rtc_wdt.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

// Additional loaded libraries
#include <Button2.h>     // Button handling
#include <LinkedList.h>  // Linked list for detected WFis
#include <EtherCard.h>   // Modified EtherCard library
#include <TFT_eSPI.h>    // LCD/TFT display
#include <ESP32Time.h>   // ESP32 internal RTC

// If Bluetooth serial should be used
#ifdef USE_BTSERIAL
#include <BluetoothSerial.h>
#include <HardwareSerial.h>
#include <esp_bt.h>
#endif

// If a SD card is used
#ifdef USE_SDCARD
#include <SD.h>
#include <bits/stdc++.h>
#endif

// If an external real time clock is used
#ifdef USE_RTCTIME
#include <RTClib.h>  // Use external battery buffered Real Time Clock
#endif

// Include local libraries / files
#include "lldp_functions.h"  // LLDP functions
#include "cdp_functions.h"   // CDP functions
#include "Packet_data.h"     // Generic packet data structure
#include "DHCPOptions.h"     // DHCP option structure
#include "prefs.h"           // Use ESP preferences for storing several configuration data

// Check if Bluetooth is enabled in default configuration. For Arduino IDE this
// should alway be true.
#ifdef USE_BTSERIAL
// Setup other core to handle Bluetooth Serial port
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif
#endif


// LCD/TFT display
static const uint8_t TFT_FONTNUMBER = 1;      // Used font number
static const uint16_t TFT_SIZESCALER = 2;     // Text size scaling
int tft_width = 240;                          // Display width in pixels
int tft_height = 240;                         // Display height in pixels
static uint16_t tft_fontHeight;               // Font height inn pixels, will be calculated
static uint16_t tft_userY = 0;                // Upper position of data area, this is the position below the header
static uint16_t tft_userHeight = tft_height;  // Lower position of data area

TFT_eSPI tft = TFT_eSPI(tft_width, tft_height);  // Invoke custom library

// General layout of header lines:
// 1. row: SD E BT A B RTC NTP Battery
// 2. row: ? DHCP vDHCP LLDP CDP
//
// Explanation first row:
// SD:   SD card
// E:    Ethernet
// BT:   Bluetooth
// A:    Serial port A
// B:    Serial port B
// RTC:  external DS3231 RTC
// W:    WiFi
// 3.3v: Battery voltage

// Explanation second row:
// ?:     Start / general information screen
// DHCP:  DHCP data for untagged VLAN
// vD:    DHCP data for VoiceVLAN
// LLDP:  Link Layer Discovery Protocol data
// CDP:   Cisco Discovery Protocol data
// N:     NTP time
// W:     WiFi data

// Header item colors:
// silver: hardware or data not available
// black:  available, hardware not active or data not visible. Will be set while processing.
// green:  available and visible or active. Will not be set in the header array, only while showing data
// blue:   hardware available and active, but not connected (Ethernet, Bluetooth)
// What if ethernet adapter is available? Then the text "E" will be displayed black instead of silver.
// If the ethernet function is selected from the menu and running, which color should be used than?
// Green if ethernet is used and a link is established, blue if the link is not up?

static byte tft_userMenuPos = 0;
static uint16_t tft_spaceWidth;  // Width of a blank, ASCII character 32 with the used font
static uint16_t tft_arrowWidth;  // Width of the arrow "> " and blank using the given font

// Header item structure
struct sHeaderData {
  String text;     // Text to display
  uint16_t xPos;   // x position
  uint16_t yPos;   // y position
  uint16_t color;  // Item color
};

static const byte TFT_IDISPLAYDATA1COUNT = 8;  // Item count first header row
static const byte TFT_IDISPLAYDATA2COUNT = 7;  // Item count second header row
static sHeaderData tft_displayData1[TFT_IDISPLAYDATA1COUNT] = {
  { "SD", 0, 0, TFT_SILVER },      // silver: not available or no SD card, black: SD card found
  { "E", 0, 0, TFT_SILVER },       // silver: not available, black: found, blue: active but no link, green: active and link
  { "BT", 0, 0, TFT_SILVER },      // silver: not available, black: found, blue: active but no link, green: active and link
  { "A", 0, 0, TFT_SILVER },       // silver: not available, black: configured, green: active
  { "B", 0, 0, TFT_SILVER },       // silver: not available, black: configured, green: active
  { "RTC", 0, 0, TFT_SILVER },     // silver: not available, black: availalbe
  { "W", 0, 0, TFT_SILVER },       // silver: not available, black: available, green: active
  { "3.3v", 0, 0, TFT_DARKGREEN }  // color depending on power level
};
static sHeaderData tft_displayData2[TFT_IDISPLAYDATA2COUNT] = {
  { "?", 0, 0, TFT_BLACK },
  { "DHCP", 0, 0, TFT_SILVER },
  { "vD", 0, 0, TFT_SILVER },
  { "LLDP", 0, 0, TFT_SILVER },
  { "CDP", 0, 0, TFT_SILVER },
  { "N", 0, 0, TFT_SILVER },
  { "W", 0, 0, TFT_SILVER }
};

// Header array entry numbers
// First row - devices
static const byte TFT_HEADERENTRY_SD = 0;    // SD card
static const byte TFT_HEADERENTRY_ETH = 1;   // Ethernet
static const byte TFT_HEADERENTRY_BT = 2;    // Bluetooth
static const byte TFT_HEADERENTRY_SERA = 3;  // Serial port A
static const byte TFT_HEADERENTRY_SERB = 4;  // Serial port B
static const byte TFT_HEADERENTRY_RTC = 5;   // Real Time Clock
static const byte TFT_HEADERENTRY_WIFI = 6;  // WiFi module
static const byte TFT_HEADERENTRY_BAT = 7;   // Battery charge state
// Second row - screen
static const byte TFT_HEADERENTRY_INFO = 0;      // Question mark for information screen
static const byte TFT_HEADERENTRY_DHCP = 1;      // DHCP data
static const byte TFT_HEADERENTRY_VDHCP = 2;     // vLAN DHCP data
static const byte TFT_HEADERENTRY_LLDP = 3;      // LLDP data
static const byte TFT_HEADERENTRY_CDP = 4;       // CDP data
static const byte TFT_HEADERENTRY_NTP = 5;       // NTP information
static const byte TFT_HEADERENTRY_WIFIDATA = 6;  // WiFi data

// Screen display constants
static const byte TFT_SCREEN_INFO = 0;
static const byte TFT_SCREEN_DHCP = 1;
static const byte TFT_SCREEN_DHCPVLAN = 2;
static const byte TFT_SCREEN_LLDP1 = 3;
static const byte TFT_SCREEN_LLDP2 = 4;
static const byte TFT_SCREEN_CDP1 = 5;
static const byte TFT_SCREEN_CDP2 = 6;
static const byte TFT_SCREEN_NTP = 7;
static const byte TFT_SCREEN_WIFIS = 8;


// User menu item structure
struct sMenuItem {
  String text;    // Text to display
  bool isActive;  // Is the entry selectable (yellow) or not (silver)?
  byte value;     // Value of the entry
};


// The default used font in this sketch has a height of eight pixels and six pixels width.
// Using the 240x240 pixel display and a font scale of two this results in 15 rows and
// 20 characters.
static sMenuItem tft_userMenu[] = {
  { TXT_GEN_ETHERNET, true, 0 },             //  1) Ethernet
  { TXT_WIFI_NAME, true, 0 },                //  2) WiFi
  { TXT_BT_SERIAL, false, 0 },               //  3) Bluetooth serial
  { TXT_BT_LOGTOSD, false, bt_SerLogNo },    //  4)   log BT serial to SD
  { TXT_SER_SERLOGGING, false, 0 },          //  5) Serial logging
  { TXT_SER_LOGTYPE, false, ser_LogASCII },  //  6)   logging type, ASCII oder HEX
  { TXT_SER_SPEED, false, 0 },               //  7) Serial connection speed
  { TXT_SER_CONFIG, false, 0 },              //  8) Serial connection configuration
  { TXT_GEN_DEFAULTFUNCTION, true, fNone },  //  9) Default function
  { TXT_GEN_WRITETOLOG, false, 0 },          // 10) Write all gathered information to log file
  { TXT_GEN_ROTATESCREEN, true, 0 },         // 11) Screen rotation
  { TXT_GEN_SCREENSWITCHDELAY, true, 0 },    // 12) Delay for autmatic screen switching
};
// Menu array entry numbers
static const byte TFT_MENUENTRY_ETHERNET = 0;
static const byte TFT_MENUENTRY_WIFI = 1;
static const byte TFT_MENUENTRY_BTSERIAL = 2;
static const byte TFT_MENUENTRY_BTSERIALLOGSD = 3;
static const byte TFT_MENUENTRY_SERIALLOGGING = 4;
static const byte TFT_MENUENTRY_SERIALLOGGINGMODE = 5;
static const byte TFT_MENUENTRY_SERIALSPEED = 6;
static const byte TFT_MENUENTRY_SERIALCONFIGURATION = 7;
static const byte TFT_MENUENTRY_DEFAULTFUNCTION = 8;
static const byte TFT_MENUENTRY_WRITETOLOG = 9;
static const byte TFT_MENUENTRY_ROTATESCREEN = 10;
static const byte TFT_MENUENTRY_SCREENSWITCHDELAY = 11;

// If SD card should be supported
#ifdef USE_SDCARD
uint32_t sd_cardSize = 0l;      // Card size in byte
uint32_t sd_cardSizeFree = 0l;  // Free bytes
bool sd_available = false;      // Is a SD card available? How to detect if a card is removed or inserted? $$$
static const char *TXT_SD_HEADERLINE = "MAC;DeviceName;SSID;Ignore";
static const uint32_t SD_SEMA_WAIT = 1000;

// File handle. Since only one file can be open simultaneous this is defined globally
File file;

// Semaphore for SD card locking
SemaphoreHandle_t xMutex_sd_card = NULL;  // Use mutex (semaphore) to handle SD card access
bool wifi_MACloaded;                      // has the WiFi MAC file been loaded?
#endif


// Buttons
Button2 btn_1(BTN_BUTTON1, INPUT);
Button2 btn_2(BTN_BUTTON2, INPUT);
int btn_click = false;

// The system has just been started or the Ethernet connection has
// been re-established
bool gen_justBooted = true;

// String for the used device name
// Do not change yet, this name is currently hard coded in the send_LLDP_MED() function in "lldp_functions.cpp".
String gen_DeviceName;

// Locally administered MAC address, do not change yet, this name is currently hard coded in the
// send_LLDP_MED() function in "lldp_functions.cpp".
byte eth_myMAC[] = { 0xCA, 0xFE, 0xC0, 0xFF, 0xEE, 0x00 };

#define UDP_SRC_PORT_H_P 0x22
#define UDP_SRC_PORT_L_P 0x23
#define DHCP_SERVER_PORT 67

PINFO eth_lldpPacket;
PINFO eth_cdpPacket;
String eth_myMACString;
static const uint16_t ETH_BUFFERSIZE = 1500;
//static const uint16_t ETH_BUFFERSIZE = 1522;  // Maximum Ethernet frame size with VLAN tag
byte Ethernet::buffer[ETH_BUFFERSIZE];
byte eth_buffcheck[ETH_BUFFERSIZE];
bool eth_ENCLink;
DHCP_DATA eth_dhcpInfo[2][255];
byte eth_vlanOption = 0;
unsigned long eth_dhcpStart;
static const unsigned long ETH_DHCPTIMEOUT = 60000l;
uint32_t eth_linkUpMillis = 0;

// Received packets
bool eth_dhcpReceived;
bool eth_vlanDhcpReceived;
bool eth_lldpPacketReceived;
bool eth_cdpPacketReceived;
bool eth_ntpReceived;
bool eth_nslookupDomainChecked;
bool eth_nslookupDNSserachlistChecked;


// VLAN support
uint16_t eth_voiceVLAN = 0;
bool eth_vLANTagging;

// Send LLDP Med packet
unsigned long eth_lastLLDPsent = 0;
const unsigned long ETH_LASTLLDPINTERVAL = 30000l;
//byte eth_lldpMEDReceivedCount = 0;

// Battery data
char bat_chargeLevelText[5];
static const unsigned long BAT_REQUESTINTERVAL = 6000l; //60000l;
unsigned long bat_lastRequestMillis = 0;
static const float BAT_MAXVOLT = 4.2;

// Display control
uint8_t disp_currentScreen = 0;
bool disp_bDisplayMenu = false;
bool disp_autoSwitch = true;
unsigned long disp_autoSwitchLastTime = 0l;


// NTP
byte eth_ntpIPs[ETH_NTPMAXSOURCES][IP_LEN];
byte eth_ntpSources = 0;
byte eth_currentNTPSource = 0;
String eth_ntpServer;
static const unsigned long ETH_NTPTIMEOUT = 1500ul;
long eth_timeZoneOffset = 0l;  // 3600L; // Winter (original) time Europe
enum { NTP_INIT,
       NTP_OK,
       NTP_FAILED } eth_ntpRequestStatus;
uint32_t eth_timeFromNTP;
unsigned long eth_ntpRequestStarted = 0l;
static const unsigned long GEN_SEVENTY_YEARS = 2208988800UL;

// ESP internal RTC
ESP32Time esprtc(eth_timeZoneOffset);  // Initialize ESP RTC with time zone offset, use 0 for UTC.

// External battery buffered real time clock
// If a RTC module should be used
#ifdef USE_RTCTIME
RTC_DS3231 rtc;
#endif
bool ertc_present = false;


#ifdef USE_BTSERIAL
byte bt_status = 0;
BluetoothSerial SerialBT;
static bool bt_isInitialized;                                   // Bluetooth initialized?
static bool bt_isConnected;                                     // Bluetooth connection established?
static bool bt_isConnectRequest;                                // Bluetooth connection requested?
static unsigned long bt_connectRequestStarttime;                // Bluetooth connection started at
static const unsigned long BT_CONNECTREQUESTTIMEOUT = 30000ul;  // Bluetooth connection timeout 30 seconds
static bool bt_isConnectRequestPageShow;                        // Is the connection request page to be shown?
static uint32_t bt_RequestedPin;                                // Bluetooth pin requested for client
#endif

#if defined(USE_BTSERIAL) || defined(USE_SERIALLOGGER)
HardwareSerial ser_HardwareA(2);
#endif

#ifdef USE_SERIALLOGGER
enum eLogMode {
  modeASCII = 0,
  modeHEX
};
HardwareSerial ser_HardwareB(1);
#endif

static unsigned long GEN_INTERVAL = 1000;
unsigned long gen_previousMillis = 0;
unsigned long gen_currentMillis;

// WiFi
/*
Structures for MAC address and WiFi device name resolution
To be more memory efficient no static two dimensonal array be used.
Since some WiFi SSID's are handled by different acceess points (Mesh)
there is no 1:1 relationship. On the other side an access point can
handle several different WiFis with also different security settings.
For this project an identified AP should only displayed once. Since
the network device name is not visible on WiFi side and it is also
not easy (currently I have no other solution found than use the MAC
address) to identify an AP the BSSID will be used. Therefore a CSV
file with the found MAC addresses and the SSID will be written on
the SD card whioch can be edited by the user and the second column
can be filled in with the device name.
So some lists will be used in memory:
a) ID and network device name of the access point
b) ID and MAC address
c) ID and SSID
D) relation between the IDs of a), b) and c)
 - a network device name (if set or an unknown entry) refer to several MAC
 addresses (1:n).
 - a SSID refer to several MAC addresses (1:m).
As an alternative: would it be possible to use only one list, reading the
CSV into the memory and link existing device names and WiFi SSIDs into new
list entries?
This would make the structure and handling a little bit easier:
*/
class wifiClass {
public:
  const char *mac;              // Pointer to the MAC address, unique
  const char *deviceName;       // Pointer to AP device name, used multiple times
  const char *SSID;             // Pointer to SSID for this MAC address, used multiple times
  int16_t strength;             // Current WiFi strength
  byte encryption;              // Used encryption for this WiFi
  bool written;                 // Entry has been written to disk
  bool found;                   // WiFi found during last scan
  unsigned long lastTimeFound;  // Last time found in millis
  bool ignore;                  // Ignore WiFi

  // Set the found member of a WiFi to false if it has not been found in the given time
  // range:
  static const unsigned long WIFINOTFOUNDAFTER = 30000;  // 120000;  // 2 * 60 * 1000ms;
};
LinkedList<wifiClass *> wifiList = LinkedList<wifiClass *>();
static const char *WIFI_AUTHENTICATIONMODE[] = {
  "open", "WEP", "WPA PSK", "WPA2 PSK", "WPA/WPA2 PSK", "WPA2 Enterpr.", "WPA3 PSK", "WPA2/WPA3 PSK", "WAPI PSK"
};


/*
The upper definition of the WiFi structure is simple. But does it really fit for the system?
I assume:
An access point (AP) has a network identifier ("Basic Service Set Identification") for each
WiFi SSID ("Service Set IDentifier") which is usually a modification of the base MAC address
or - in some cases - a random value. I haven't seen the random value yet. Additionally a MAC
address should be assigned to a hardware device name in a network. We can not access this
name through WiFi (usually for security reasons). Also in a mesh environment multiple WiFi
APs will serve the same SSID and switch a connected device to the best AP. In a non mesh
environment only SSID and the connection type (i.e. password) will connect to one AP. So
what do we have now?
a) one AP has one network name (device name) which should be managed by the owner
b) one AP may serve several WiFi SSIDs with one network BSSID for each
c) the BSSID is different for different APs
d) severl APs may serve the same SSID (i.e. for mesh)
While recording all available WiFis the BSSID and SSID will be recognized. For later use
a device name (for managment) and also a flag if this WiFi should be ignored will be
written to the SD card. While two functions should be possible for displaying data a better
internal manangement may be needed:
A) display all found WiFis if not ignored:
   - SSID
   - MAC address (BSSID)
   - device name if known from file (this has to be edited manually)
   - signal strength
   - encryption
B) display data for one selected WiFi SSID
   - device name if known
   - MAC address if device name is not known
   - signal strength
For A) it doesn't matter how the data is organized since all current found WiFis has to
be checked. For B) it is easier to have an other structure based on the SSID.
Alternative structure below.
The idea is to start with a linked list (llWiFis) containing the SSID for each WiFi.
The SSID can be fully ignored if each device distributing this SSID is ignored. This
should make the checking process a little bit faster. Additionally if one SSID should
be tracked for finding areas between APs where the signal is worse it is only necessary
to display all entries in the linked list for devices inside the SSID list entry.
"found" and "strength": since not all WiFis will be found during a scan, while moving
with DAMPF or the signal is weak, there are two possiblities to check the availability:
1) reset all "found" values to false before the last scan will be evaluated and set
   only the found WiFis to true. This will lead to miss some entries and find those 
   WiFi again. Since this is not the way a check should be done a second entry like
   "foundSinceWiFiTurnedOn" is needed. When turning WiFi on both values will be set
   to false, before a scan is evaluated only "found" is set to false. If an AP is
   not found during the last scan the value keeps on "false" but for displaying
   the value "foundSinceWiFiTurnedOn" is evaluated and in this case a bad or missing
   signal will be displayed.
2) "found" will be handled as "found" since WiFi has been enabled. The signal strength
   for all WiFis will be set to "INT16_MIN" (since RSSI is defined in
   "esp_wihi.h" for value in the range from -100 to 0 dBm) before the last scan will
   be evaluated and a strength value of "INT16_MIN" and found=true means the WiFi
   wasn't found in the last scan.
*/

/*
class clWiFiDevice {
public:
  const char *mac;         // Pointer to the MAC address, unique
  const char *deviceName;  // Pointer to AP device name, used multiple times
  int16_t strength;        // Current WiFi strength
  byte encryption;         // Used encryption for this WiFi
  bool found;              // WiFi found during last scan or since WiFi turned on?
  bool ignore;             // Ignore WiFi
  bool written;            // Entry has been written to disk
};
class clWiFiSSID {
public:
  const char *SSID;  // Pointer to SSID for this MAC address, used multiple times
  bool ignore;       // Ignore WiFis using this SSID
  uint16_t count;    // Number of devices serving this SSID
  LinkedList<clWiFiDevice *> llWiFiDevices = LinkedList<clWiFiDevice *>();
};
LinkedList<clWiFiSSID *> llWiFis = LinkedList<clWiFiSSID *>();

Maybe this will be changed in a future version
*/

static const unsigned long WIFI_SCANPERIOD = 10000l;
static const unsigned long WIFI_SCANCHECKPERIOD = 1000;
long wifi_lastScanMillis;
long wifi_lastCheckMillis;
int16_t wifi_currentCheck;
bool wifi_disableIfPossible = false;
bool wifi_isEnabled;
bool wifi_isScanComplete = true;
uint16_t wifi_CountFound = 0;
int16_t wifi_Current = -1;
static const byte WIFI_MAXDISPLAYPERPAGE = 4;
const char *wifi_scanForSSID;

// Current and default function
static eFunction gen_currentFunction = fNone;

// Get the ESP Arduino version in Format a.b.c
// ..\Arduino15\packages\esp32\hardware\esp32\2.0.14\cores\esp32\esp_arduino_version.h
const char *getArduinoPlatformVersion() {
  static char version[8];  // x.xx.xx\0
#ifdef ESP_ARDUINO_VERSION_MAJOR
  sprintf(version, "%d.%d.%d", ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH);
#endif
  return version;
}  // const char *getArduinoPlatformVersion()


// General Arduino setup
// Initialize system
void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // The following code removes background checks not needed.
  rtc_wdt_protect_off();
  rtc_wdt_disable();
  disableCore0WDT();
  disableLoopWDT();

  // Configure power analog to digital converter
  esp_adc_cal_characteristics_t adc_chars;
  //esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_2_5, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_11, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);


#ifdef DEBUGSERIAL
  // This serial is used for debug purposes only.
  Serial.begin(115200);
#endif

  // SPI chip select configuration
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);  // TFT Chip select high (inactive)
  pinMode(ETH_CS, OUTPUT);
  digitalWrite(ETH_CS, HIGH);  // Ethernet Chip select high (inactive)
#ifdef USE_SDCARD
  pinMode(HSPI_SD_CS, OUTPUT);
  digitalWrite(HSPI_SD_CS, HIGH);  // SDCard Chip select high (inactive)
#endif

  // Initialize data from preferences
  tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value = readPreferencesBTSerLogToSD();
  tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value = readPreferencesSerLogType();
  tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value = readPreferencesSerSpeed();
  tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value = readPreferencesSerConfig();
  tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value = readPreferencesDefaultFunction();
  tft_userMenu[TFT_MENUENTRY_ROTATESCREEN].value = readPreferencesOrientation();
  tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = readPreferencesDelay();

  // Disable WIFI Options to save power
  if (tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value != fWiFi)
    wifi_disable();

  // Create string of MAC address
  eth_myMACString = "";
  for (byte i = 0; i < 6; i++) {
    String tmp = "0" + String(eth_myMAC[i], HEX);
    tmp = tmp.substring(tmp.length() - 2);
    eth_myMACString += tmp;
    if (i < 5)
      eth_myMACString += ":";
  }

  // The maximum length for a name is defines in RFC 1035 with 63 bytes.
  gen_DeviceName = String(TXT_GEN_DEVNAME);

#ifdef DEBUGSERIAL
  Serial.printf("\n%s %s\n", TXT_GEN_DEVNAME, TXT_GEN_VERSION);
  Serial.printf("%s %s\n", TXT_GEN_PROPERTYOF, TXT_GEN_OWNER);
  Serial.println("MAC: %s" + eth_myMACString);

  Serial.println("Used IDF version for compilation: " + String(esp_get_idf_version()));
  Serial.printf("Used ESP arduino platform for compilation: %s\n", getArduinoPlatformVersion());
  Serial.println("Used IDE version for compilation: " + String(ARDUINO));
  Serial.printf("Compilation time: %s %s\n", __DATE__, __TIME__);
  Serial.printf("GNU C++ version: %s\n", __VERSION__);
  Serial.printf("C++ standard: %ld\n", __cplusplus);

  Serial.printf("Total heap:  %u\n", ESP.getHeapSize());
  Serial.printf("Free heap:   %u\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %u\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM:  %u\n", ESP.getFreePsram());
#endif

  // Initialize real time clock
  rtc_initialize();

  // Initialize ntp server array
  eth_inittializeNTPSources();

  // Initialize the screen
  tft_initialize();

#ifdef USE_SDCARD
  // Create a mutex to control SD card access
  xMutex_sd_card = xSemaphoreCreateMutex();
  assert(xMutex_sd_card != NULL);

  // Initialize SD card, sd_initialize depends on a initialized eTFT_SPI display
  sd_available = sd_initialize();
#endif

  // Starting
  tft_showPage();

  // Specify button options
  btn_initialize();

  // Initialize serial ports
  ser_initialize();

#ifdef USE_BTSERIAL
  bt_initialize();
#endif

  // Initialize ENC28J60
  eth_initialize();

  // Set auto scroll time to current millis
  disp_autoSwitchLastTime = millis();

  // Get battery voltage
  bat_getVoltage();

  // Start the default function
  gen_switchFunction((eFunction)tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value);

  tft_updateHeader(true);

#ifdef DEBUGSERIAL
  Serial.println("Initialized");
#endif
}  // void setup()


// Main loop
void loop() {
  //  gen_currentMillis = millis();
  bool isENCLinkUp = false;
  bool isVLANTaggingEnabled = ENC28J60::is_VLAN_tagging_enabled();
  bool receivedPacketWasTagged = false;

  while (1) {
    // Update current millis for this round
    gen_currentMillis = millis();

#ifdef USE_BTSERIAL
    // Check if a Bluetooth connection is requested
    if (bt_isConnectRequestPageShow == true) {
#ifdef DEBUGSERIAL
      Serial.println("loop(): bt_isConnectRequestPageShow");
#endif
      bt_isConnectRequestPageShow = false;
      tft_showPage();
    } else if ((disp_autoSwitch) && (!bt_isConnectRequest)) {
#else
    if (disp_autoSwitch) {
#endif
        if (gen_currentMillis > disp_autoSwitchLastTime + 1000ul * tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value) {
          tft_switchScreen();
        }
    }

    // Check battery state
    if (gen_currentMillis >= bat_lastRequestMillis + BAT_REQUESTINTERVAL)
      bat_getVoltage();

    // Do periodically checks and updates
    // Is it usefull to split, the voltage status has not
    // been to be updated like the link status
    if (gen_currentMillis >= gen_previousMillis + GEN_INTERVAL) {
      gen_previousMillis = gen_currentMillis;
      if (gen_currentFunction == fEthernet) {
        isENCLinkUp = eth_linkStatus();
        isVLANTaggingEnabled = ENC28J60::is_VLAN_tagging_enabled();
      }
    }

    // Check and process current selected/activated function
    if (gen_currentFunction == fEthernet) {
      if (isENCLinkUp)
        eth_process();
    } else if (gen_currentFunction == fWiFi) {
      wifi_process();
#ifdef USE_BTSERIAL
    } else if (gen_currentFunction == fBluetoothSerial) {
      bt_process();
#endif
#ifdef USE_SERIALLOGGER
    } else if (gen_currentFunction == fSerialLogger) {
      ser_process();
#endif
    }  // if( gen_currentFunction == fSerialLogger )

#ifdef DEBUGSERIAL
    // Handle serial input / output using the debugging console
    dbg_process();
#endif

    // Handle buttons
    btn_process();

    if (isENCLinkUp) {
      // Get time from network using NTP request
      if ((eth_ntpRequestStatus == NTP_INIT) && (eth_dhcpReceived) && (eth_nslookupDomainChecked) && (eth_nslookupDNSserachlistChecked)) {
        eth_startNTPRequests();
      }

      // Check if an ethernet packed has been received
      uint16_t plen = ether.packetReceive();
      if (plen > 0) {
        receivedPacketWasTagged = ENC28J60::packet_Received_Was_Tagged();

        // Check packet size and copy packet to ethernet buffer
        if (plen > ETH_BUFFERSIZE)
          plen = ETH_BUFFERSIZE;
        memcpy(eth_buffcheck, Ethernet::buffer, plen);
      }

      // Run the DHCP state machine
      if ((isENCLinkUp) && ((isVLANTaggingEnabled && receivedPacketWasTagged) || (!isVLANTaggingEnabled))) {
        plen = eth_callDhcpStateMachine(plen);
      }

      // If the last packet was not a DHCP packet process
      if (plen > 0) {
        if ((isVLANTaggingEnabled && !receivedPacketWasTagged) || (!isVLANTaggingEnabled)) {
          // Check if the packet is a LLDP broadcast
          unsigned int lldp_correct = lldp_check_Packet(eth_buffcheck, plen);
          if (lldp_correct > 1) {
            eth_lldpPacket = lldp_packet_handler(eth_buffcheck, plen);
            eth_lldpPacketReceived = true;
            tft_updateHeader(false);
            if (eth_lldpPacket.VoiceVLAN[1] != "-") {
              if (eth_voiceVLAN == 0) {
                eth_voiceVLAN = eth_lldpPacket.VoiceVLAN[1].toInt();
              }
            }
          }  // if( lldp_correct > 1 )
          else {
            // Check if the packet is a CDP broadcast
            unsigned int cdp_correct = cdp_check_Packet(eth_buffcheck, plen);
            if (cdp_correct > 1) {
              eth_cdpPacket = cdp_packet_handler(eth_buffcheck, plen);
              eth_cdpPacketReceived = true;
              tft_updateHeader(false);
              if (eth_cdpPacket.VoiceVLAN[1] != "-") {
                if (eth_voiceVLAN == 0) {
                  eth_voiceVLAN = eth_cdpPacket.VoiceVLAN[1].toInt();
                }
              }
            }  // if( cdp_correct > 1 )
            else {
              // any other protocol?
              //#ifdef DEBUGSERIAL
              /*
              if( ( eth_buffcheck[ 12 ] == 0x08 ) && ( eth_buffcheck[ 13 ] == 0x00 ) )
              {
//                Serial.println( F( "Type Ethernet Frame" ) );
//                if( eth_buffcheck[ IP_PROTO_P ] == IP_PROTO_ICMP_V )
//                  Serial.println( F( "ICMP packet" ) );
//                else if( eth_buffcheck[ IP_PROTO_P ] == IP_PROTO_TCP_V )
//                  Serial.println( F( "TCP packet" ) );
//                else if( eth_buffcheck[ IP_PROTO_P ] == IP_PROTO_UDP_V )
//                  Serial.println( F( "UDP packet" ) );

                if( eth_buffcheck[ IP_PROTO_P ] == IP_PROTO_UDP_V )
                {
                  if( ( eth_buffcheck[ 14 ] & 0b00001111 ) == 5 ) // IP header length / 4
                  {
                    Serial.println( F( "\nUnhandled packet received" ) );
                    if( ( ( eth_buffcheck[ UDP_SRC_PORT_H_P ] << 8 ) | eth_buffcheck[ UDP_SRC_PORT_L_P ] ) == 123 )
                    {
                    Serial.println( F( "\nUnhandled packet received" ) );
                      Serial.println( "Source IP:" + String( eth_buffcheck[ IP_SRC_P + 0 ] ) + "." + String( eth_buffcheck[ IP_SRC_P + 1 ] ) + "." + String( eth_buffcheck[ IP_SRC_P + 2 ] ) + "." + String( eth_buffcheck[ IP_SRC_P + 3 ] ) );
                      Serial.println( "Dest IP  :" + String( eth_buffcheck[ IP_DST_P + 0 ] ) + "." + String( eth_buffcheck[ IP_DST_P + 1 ] ) + "." + String( eth_buffcheck[ IP_DST_P + 2 ] ) + "." + String( eth_buffcheck[ IP_DST_P + 3 ] ) );
                      Serial.println( "Source Port:" + String( ( eth_buffcheck[ UDP_SRC_PORT_H_P ] << 8 ) | eth_buffcheck[ UDP_SRC_PORT_L_P ] ) );
                      Serial.println( "Dest Port  :" + String( ( eth_buffcheck[ UDP_DST_PORT_H_P ] << 8 ) | eth_buffcheck[ UDP_DST_PORT_L_P ] ) ) ;
//                if( ( eth_buffcheck[ 30 ] == ether.myip[ 0 ] ) && 
//                  ( eth_buffcheck[ 31 ] == ether.myip[ 1 ] ) && 
//                  ( eth_buffcheck[ 32 ] == ether.myip[ 2 ] ) && 
//                  ( eth_buffcheck[ 33 ] == ether.myip[ 3 ] ) )
//                {
                      String tmpHex;
                      for( uint16_t i = 0; i < plen; i++ )
                      {
                        tmpHex = "00" + String( eth_buffcheck[ i ], HEX );
                        tmpHex = "0x" + tmpHex.substring( tmpHex.length() - 2 );
                        Serial.print( tmpHex + " " );
                        if( ( ( i  + 1 ) % 8 ) == 0 ) Serial.println();
                      }
                      Serial.println();
                    }
                  }
                }
              } // if( ( eth_buffcheck[ 12 ] == 0x08 ) && ( eth_buffcheck[ 13 ] == 0x00 ) )
*/
              //#endif
            }  // else
          }    // else
        }      // if( ( ENC28J60::is_VLAN_tagging_enabled() && !ENC28J60::packetReceivedWasTagged() ) || ( !ENC28J60::is_VLAN_tagging_enabled() ) )

        // Set length of received packet to 0
        plen = 0;
      }  // if( plen > 0 )

      // Check if voice VLAN ID is available and if NTP is not in init state
      if ((eth_voiceVLAN > 0) && eth_ntpRequestStatus != NTP_INIT) {
        if (ether.dhcpState == EtherCard::DHCP_STATE_BOUND) {
          if (!ENC28J60::is_VLAN_tagging_enabled()) {
            EtherCard::dhcpRelease();
            ENC28J60::enable_VLAN_tagging(eth_voiceVLAN);
            eth_startDHCP();
            tft_updateHeader(false);
          } else {
            EtherCard::dhcpRelease();
            tft_updateHeader(false);
          }
        }
      }
    }

    // Check if there is a system handling the DNS domain name (DHCP option 15) which might be a domain controller
    //  if( ( eth_dhcpReceived ) && ( ether.dnsip[ 0 ] != 0 ) && ( !isVLANTaggingEnabled ) && ( !eth_nslookupDomainChecked ) )
    if ((eth_dhcpReceived == true) && (ether.dnsip[0] != 0) && (!isVLANTaggingEnabled) && (!eth_nslookupDomainChecked)) {
      if ((eth_ntpSources < ETH_NTPMAXSOURCES) && (eth_dhcpInfo[0][15].Option[1] != "-")) {
#ifdef DEBUGSERIAL
        Serial.print("NS lookpup for domain:");
        Serial.println(eth_dhcpInfo[0][15].Option[1]);
#endif
        if (ether.dnsLookup(eth_dhcpInfo[0][15].Option[1].c_str())) {
#ifdef DEBUGSERIAL
          Serial.println("IP: " + String(ether.hisip[0]) + "." + String(ether.hisip[1]) + "." + String(ether.hisip[2]) + "." + String(ether.hisip[3]));
#endif
          for (byte i = 0; i < IP_LEN; ++i)
            eth_ntpIPs[eth_ntpSources][i] = ether.hisip[i];
          eth_ntpSources++;
#ifdef DEBUGSERIAL
        } else {
          Serial.println("NS looup failed");
#endif
        }
      }  // if( ( eth_ntpSources < ETH_NTPMAXSOURCES ) && ( eth_dhcpInfo[ 0 ][ 15 ].Option[ 1 ] != "-" ) )
#ifdef DEBUGSERIAL
      else {
        Serial.println("No DNS name provided.");
      }
#endif
      eth_nslookupDomainChecked = true;
    }  // if( ( eth_dhcpReceived == true ) && ( ether.dnsip[ 0 ] != 0 ) && ( !isVLANTaggingEnabled ) && ( !eth_nslookupDomainChecked ) )

    // Check if there is a domain name search list (DHCP option 119), maybe there is a domain controller
    if ((eth_dhcpReceived) && (ether.dnsip[0] != 0) && (!isVLANTaggingEnabled) && (!eth_nslookupDNSserachlistChecked)) {
      String searchList = eth_dhcpInfo[0][119].Option[1];
      if ((eth_ntpSources < ETH_NTPMAXSOURCES) && (searchList != "-") && (searchList != "")) {
#ifdef DEBUGSERIAL
        Serial.print("NS lookpup for domain search list:");
        Serial.println(searchList);
#endif

        int16_t posStart = 0;
        int16_t posEnd = 0;
        String part;
        while ((posEnd != -1) && (eth_ntpSources < ETH_NTPMAXSOURCES)) {
          posEnd = searchList.indexOf('\n', posStart);
          if (posEnd > 0)
            part = searchList.substring(posStart, posEnd);
          else
            part = searchList.substring(posStart);

#ifdef DEBUGSERIAL
          Serial.print("looking up:");
          Serial.println(part);
#endif

          if (ether.dnsLookup(part.c_str())) {
#ifdef DEBUGSERIAL
            Serial.println("IP: " + String(ether.hisip[0]) + "." + String(ether.hisip[1]) + "." + String(ether.hisip[2]) + "." + String(ether.hisip[3]));
#endif
            for (byte i = 0; i < IP_LEN; ++i)
              eth_ntpIPs[eth_ntpSources][i] = ether.hisip[i];
            eth_ntpSources++;
#ifdef DEBUGSERIAL
          }  // if( ether.dnsLookup( part.c_str() ) )
          else {
            Serial.println("DNS failed");
#endif
          }

          posStart = posEnd + 1;
        }
      }  // if( ( eth_ntpSources < ETH_NTPMAXSOURCES ) && ( eth_dhcpInfo[ 0 ][ 15 ].Option[ 1 ] != "-" ) )
#ifdef DEBUGSERIAL
      else {
        Serial.println("No domain name search list provided.");
      }
#endif
      eth_nslookupDNSserachlistChecked = true;
    }  // if( ( eth_dhcpReceived == true ) && ( ether.dnsip[ 0 ] != 0 ) && ( !isVLANTaggingEnabled ) && ( !eth_nslookupDNSserachlistChecked ) )

    vTaskDelay(1);
  }
}  // void loop()


// *************************************************************************
// Ethernet functions
// Set all information about received ethernet packets to false
void eth_initalizeReceivedPackets() {
  // No packets for given protocols received
  eth_dhcpReceived = false;
  eth_vlanDhcpReceived = false;
  eth_lldpPacketReceived = false;
  eth_cdpPacketReceived = false;
  eth_ntpReceived = false;
  eth_nslookupDomainChecked = false;
  eth_nslookupDNSserachlistChecked = false;
}  // void eth_initalizeReceivedPackets()

// Initialie Ethernet hardware and connection
bool eth_initialize() {
#ifdef DEBUGSERIAL
  Serial.println("eth_initialize(): initiaizing...");
#endif
  eth_initalizeReceivedPackets();

  // Return value seems to be chip revision of ENC28J60
  byte initENC = ether.begin(ETH_BUFFERSIZE, eth_myMAC, ETH_CS);

  if (initENC != 0) {
#ifdef DEBUGSERIAL
    Serial.printf("Revision of ENC26j80 chip detected: %d\n", initENC);
#endif
    ether.dhcpAddOptionCallback(15, DHCPOption);
  } else {
#ifdef DEBUGSERIAL
    Serial.printf("Revision of ENC26j80 chip detection returned 0.\n");
#endif
    return false;
  }

  // Set ENC to promiscuous mode
  ENC28J60::enablePromiscuous();

  // Start with disabled VLAN tagging
  ENC28J60::disable_VLAN_tagging();

  // If the current used function is not Ethernet while initializing, disable
  if (gen_currentFunction != fEthernet) {
    tft_displayData1[TFT_HEADERENTRY_ETH].color = TFT_BLACK;
    //ENC28J60::powerDown();
    eth_stop();
  } else {
    tft_displayData1[TFT_HEADERENTRY_ETH].color = TFT_BLUE;
  }

  return true;
}  // bool eth_initialize()


// Stop Ethernet module and putting to sleep
void eth_stop() {
  bool enc_powerdUp;
  enc_powerdUp = ENC28J60::isPoweredUp();
  if (enc_powerdUp) {
#ifdef DEBUGSERIAL
    Serial.println("eth_stop(): ENC28J60 is powered up, sending to sleep.");
#endif
    ENC28J60::powerDown();
    tft_displayData1[TFT_HEADERENTRY_ETH].color = TFT_BLACK;
  }
#ifdef DEBUGSERIAL
  else
    Serial.println("eth_stop(): ENC28J60 already powered down.");
#endif
}  // void eth_stop()


// Restart Ethernet module
void eth_restart() {
  bool enc_powerdUp;
  enc_powerdUp = ENC28J60::isPoweredUp();
  if (!enc_powerdUp) {
#ifdef DEBUGSERIAL
    Serial.println("eth_restart(): ENC28J60 is sleepig, waking up.");
#endif
    ENC28J60::powerUp(false);
  }
#ifdef DEBUGSERIAL
  else
    Serial.println("eth_restart(): ENC28J60 already awake.");
#endif
  tft_displayData1[TFT_HEADERENTRY_ETH].color = TFT_BLUE;
}  // void eth_restart()


// Process ethernet part
void eth_process() {
  //  static unsigned long lastENCLinkCheck = gen_currentMillis;
  //  static bool isENCLinkUp = false; // = ENC28J60::isLinkUp(); // $$$ wird wohl zu oft hintereinander aufgerufen
  //  static bool isVLANTaggingEnabled = ENC28J60::is_VLAN_tagging_enabled();
  //  static bool receivedPacketWasTagged = false;

  // Periodically send LLDP-MED packets
  if ((gen_currentMillis >= (eth_lastLLDPsent + ETH_LASTLLDPINTERVAL))) {
    send_LLDP_MED(ETH_BUFFERSIZE, eth_voiceVLAN, &eth_lastLLDPsent, &eth_myMAC[0]);
  }
}  // void eth_process( void )


// Check link status for changes
bool eth_linkStatus() {
  bool eth_currentLinkStatus = ENC28J60::isLinkUp();
  if (eth_ENCLink != eth_currentLinkStatus || gen_justBooted == true) {
    eth_ENCLink = eth_currentLinkStatus;
    eth_linkUpMillis = millis();
    if (eth_currentLinkStatus) {
      // Reset DHCP array to defaults
      for (byte i = 0; i < 1; i++) {
        for (byte j = 0; j < 255; ++j) {
          eth_dhcpInfo[i][j].Option[0] = "-";
          eth_dhcpInfo[i][j].Option[1] = "-";
        }
      }

      eth_resetPinfo(&eth_lldpPacket);
      eth_resetPinfo(&eth_cdpPacket);
      eth_initalizeReceivedPackets();

      gen_justBooted = false;
      eth_lastLLDPsent = 0;
      tft_displayData1[TFT_HEADERENTRY_ETH].color = TFT_DARKGREEN;  // Ethernet active and link detected
#ifdef DEBUGSERIAL
      Serial.println("Eth_linkStatus(): ETH TFT_DARKGREEN");
#endif
      disp_currentScreen = TFT_SCREEN_INFO;
      ENC28J60::disable_VLAN_tagging();
      eth_inittializeNTPSources();
      eth_startDHCP();
      tft_showPage();

      // Send LLDP-MED packet
      send_LLDP_MED(ETH_BUFFERSIZE, eth_voiceVLAN, &eth_lastLLDPsent, &eth_myMAC[0]);
    }  // if (eth_currentLinkStatus)
    else {
      tft_displayData1[TFT_HEADERENTRY_ETH].color = TFT_BLUE;
#ifdef DEBUGSERIAL
      Serial.println("eth_linkStatus(): ETH TFT_BLUE");
#endif
      tft_updateHeader(false);
      memset(Ethernet::buffer, 0, ETH_BUFFERSIZE);
      memset(eth_buffcheck, 0, ETH_BUFFERSIZE);
      gen_justBooted = false;
    }
  }

  return eth_currentLinkStatus;
}  // bool eth_linkStatus()

// Start DHCP request
void eth_startDHCP() {
  eth_vlanOption = 0;
  if (ENC28J60::is_VLAN_tagging_enabled())
    eth_vlanOption = 1;
  //  if( ENC28J60::is_VLAN_tagging_enabled() )
  //  {
  //    eth_vlanOption = 1;
  //    eth_vlanDhcpReceived = false;
  //  }
  //  else
  //  {
  //    eth_vlanOption = 0;
  //    eth_dhcpReceived = false;
  //  }

  // Reset Array to defaults.
  for (int j = 0; j < 255; ++j) {
    eth_dhcpInfo[eth_vlanOption][j].Option[0] = "-";
    eth_dhcpInfo[eth_vlanOption][j].Option[1] = "-";
  }

  eth_dhcpReceived = false;
  //ether.dhcpSetup(eth_dhcpName);
  ether.dhcpSetup(TXT_GEN_DEVNAME);

  eth_dhcpStart = millis();
}  // void eth_startDHCP()

// Check current package for DHCP ports
unsigned int eth_dhcpCheckPacket(byte EthBuffer[], unsigned int length) {
  if (length >= 60) {
    uint16_t srcPort = (EthBuffer[UDP_SRC_PORT_H_P] << 8) | (EthBuffer[UDP_SRC_PORT_H_P]);
    if (srcPort == DHCP_SERVER_PORT) {
      // CDP Packet found and is now getting processed
      tft_updateHeader(false);
      return length;
    }
  } // if (length >= 60)

  return 0;
} // unsigned int eth_dhcpCheckPacket(byte EthBuffer[], unsigned int length)

// Check packet data and run the DHCP state machine if necessary
uint16_t eth_callDhcpStateMachine(uint16_t plen) {
  if (!ENC28J60::isLinkUp())
    return plen;

  unsigned int dhcp_correct = 0;
  if ((ether.dhcpState != EtherCard::DHCP_STATE_BOUND) && ((millis() - eth_dhcpStart) < ETH_DHCPTIMEOUT) && (ENC28J60::isLinkUp()) && ((millis() - eth_dhcpStart) > 0l)) {
    dhcp_correct = eth_dhcpCheckPacket(eth_buffcheck, plen);
    ether.DhcpStateMachine(plen);
  }

  if (dhcp_correct > 0)
    plen = 0;  // DHCP packet was processed so no need to do CDP or LLDP checks

  if ((ether.dhcpState == EtherCard::DHCP_STATE_BOUND) && (eth_dhcpReceived == false))
  //  if( ( ether.dhcpState == EtherCard::DHCP_STATE_BOUND ) &&
  //    ( ( !eth_dhcpReceived && !ENC28J60::is_VLAN_tagging_enabled() ) || ( !eth_vlanDhcpReceived && ENC28J60::is_VLAN_tagging_enabled() ) )
  //  )
  {
    // DHCP has entered into bound state
    eth_dhcpReceived = true;
    ether.updateBroadcastAddress();
    ether.delaycnt = 0;

    // Convert received UIP address into a string for the eth_dhcpInfo array
    String ipaddy;
    for (byte j = 0; j < 4; ++j) {
      ipaddy += String(ether.myip[j]);
      if (j < 3) {
        ipaddy += ".";
      }
    }

    eth_vlanOption = 0;
    if (ENC28J60::is_VLAN_tagging_enabled())
      eth_vlanOption = 1;
    //    if( ENC28J60::is_VLAN_tagging_enabled() )
    //    {
    //      eth_vlanOption = 1;
    //      eth_vlanDhcpReceived = true;
    //    }
    //    else
    //    {
    //      eth_vlanOption = 0;
    //      eth_dhcpReceived = true;
    //    }

    eth_dhcpInfo[eth_vlanOption][0].Option[0] = "IP";  // Option 0 is not used
    eth_dhcpInfo[eth_vlanOption][0].Option[1] = ipaddy;

    // Update header
    tft_updateHeader(false);

#ifdef DEBUGSERIALx
    Serial.println(" \nDHCP address and options received:");
    // Write all received options to the serial console
    for (byte i = 0; i < 254; i++) {
      if (eth_dhcpInfo[eth_vlanOption][i].Option[1] != "-")
        Serial.println(String(i, DEC) + ": " + eth_dhcpInfo[eth_vlanOption][i].Option[0] + "=" + eth_dhcpInfo[eth_vlanOption][i].Option[1]);
    }
#endif
  } // if ((ether.dhcpState == EtherCard::DHCP_STATE_BOUND) && (eth_dhcpReceived == false))

  return plen;
}  // uint16_t eth_callDhcpStateMachine(uint16_t plen)

// Initialize the eth_ntpSources value and the IP array
void eth_inittializeNTPSources() {
  for (byte i = 0; i < ETH_NTPMAXSOURCES; i++) {
    for (byte j = 0; j < IP_LEN; j++) {
      eth_ntpIPs[i][j] = 0;
    }
  }
  eth_ntpSources = 0;
  eth_currentNTPSource = 0;
  eth_ntpRequestStarted = 0l;
  eth_timeFromNTP = 0l;
  eth_ntpRequestStatus = NTP_INIT;
  eth_ntpServer = "";
} // void eth_inittializeNTPSources()

// Start NTP request
void eth_startNTPRequests() {
#ifdef DEBUGSERIAL
  Serial.println("eth_startNTPRequests()");
#endif
  bool isENCLinkUp = ENC28J60::isLinkUp();

  // Get time from network
  if ((isENCLinkUp) && (eth_ntpRequestStatus == NTP_INIT) && (eth_dhcpReceived))
  //  if( ( isENCLinkUp ) && ( eth_ntpRequestStatus == NTP_INIT ) &&
  //    ( ( eth_dhcpReceived && !ENC28J60::is_VLAN_tagging_enabled() ) || ( eth_vlanDhcpReceived && ENC28J60::is_VLAN_tagging_enabled() ) )
  //  )
  {
    if ((eth_ntpRequestStarted == 0l) || (eth_ntpRequestStatus == NTP_INIT)) {
      byte i = 0;
      eth_timeFromNTP = 0l;
      while ((i < eth_ntpSources) && (eth_timeFromNTP == 0l)) {
#ifdef DEBUGSERIAL
        Serial.print("Start NTP request source=");
        Serial.println(String(i));
#endif
        eth_timeFromNTP = eth_getNtpTime(i);
        if (eth_timeFromNTP != 0l) {
          eth_ntpRequestStatus = NTP_OK;
          eth_timeFromNTP += eth_timeZoneOffset;
#ifdef DEBUGSERIAL
          Serial.print("NTP reply received time");
          if (eth_timeZoneOffset == 0l)
            Serial.print("(UTC) ");
          Serial.print(": " + String(eth_timeFromNTP));
#endif
          esprtc.setTime(eth_timeFromNTP);
          tft_displayData1[TFT_HEADERENTRY_RTC].color = TFT_DARKGREEN;
#ifdef DEBUGSERIAL
          Serial.println("eth_startNTPRequests(): RTC TFT_DARKGREEN");
#endif

#ifdef USE_RTCTIME
          if (ertc_present)
            rtc.adjust(DateTime(eth_timeFromNTP));
#endif
#ifdef DEBUGSERIAL
          // formating options  http://www.cplusplus.com/reference/ctime/strftime/
          Serial.println(esprtc.getTime("%A, %B %d %Y %H:%M:%S")) + String(" (UTC)");  // (String) returns time with specified format
#endif
          eth_currentNTPSource = i;
          eth_ntpReceived = true;
          eth_ntpServer = "";
          for (byte j = 0; j < IP_LEN; j++) {
            eth_ntpServer += String(eth_ntpIPs[eth_currentNTPSource][j]);
            if (j < (IP_LEN - 1))
              eth_ntpServer += ".";
          }
          tft_updateHeader(false);
          break;
        } // if (eth_timeFromNTP != 0l)
        i++;
      } // while ((i < eth_ntpSources) && (eth_timeFromNTP == 0l))

      // still in INIT state?
      if (eth_ntpRequestStatus == NTP_INIT)
        eth_ntpRequestStatus = NTP_FAILED;

#ifdef DEBUGSERIAL
      Serial.print("NTP request finished: ");
      Serial.println(eth_ntpRequestStatus);
      Serial.println("Time: " + String(eth_timeFromNTP));
#endif
    }  // if ((eth_ntpRequestStarted == 0l) || (eth_ntpRequestStatus == NTP_INIT))
  }    // if ((isENCLinkUp) && (eth_ntpRequestStatus == NTP_INIT) && (eth_dhcpReceived))
}  // void eth_startNTPRequests()

unsigned long eth_getNtpTime(byte src) {
  ether.ntpRequest(eth_ntpIPs[src], NTP_PORT);
  unsigned long beginWait = millis();
  while (millis() - beginWait < ETH_NTPTIMEOUT) {
    word length = ether.packetReceive();
    ether.packetLoop(length);
    if (length > 0 && ether.ntpProcessAnswer(&eth_timeFromNTP, NTP_PORT)) {
      Serial.print("Time from NTP: ");
      Serial.println(eth_timeFromNTP);
      return eth_timeFromNTP - GEN_SEVENTY_YEARS;
    }
  }
  return 0;
}  // unsigned long eth_getNtpTime(byte src)

// Reset PINFO structures / values. Modev this into Ethernet part because the data is collected
// by Ethernet only, neither BLuetooth nor WiFi.
void eth_resetPinfo(PINFO *info) {
  info->ChassisID[1] = "-";
  info->Proto[1] = "-";
  info->ProtoVer[1] = "-";
  info->SWName[1] = "-";
  info->SWDomain[1] = "-";
  info->MAC[1] = "-";
  info->Port[1] = "-";
  info->PortDesc[1] = "-";
  info->Model[1] = "-";
  info->VLAN[1] = "-";
  info->IP[1] = "-";
  info->VoiceVLAN[1] = "-";
  info->Cap[1] = "-";
  info->SWver[1] = "-";
  info->TTL[1] = "-";
  info->VTP[1] = "-";
  info->Dup[1] = "-";
  info->PoEAvail[1] = "-";
  info->PoECons[1] = "-";
  info->MgmtIP[1] = "-";
  info->MgmtVLAN[1] = "-";
  info->Checksum[1] = "-";
}  // void eth_resetPinfo( PINFO *info )


// *************************************************************************
// Serial console and debug functions
#ifdef DEBUGSERIAL
// Process serial interface, used for controlling using the serial monitor
void dbg_process() {
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();  // remove any \r \n whitespace at the end of the String
    command.toLowerCase();
    if (command == "?") {
      Serial.println("Help:");
      Serial.println("Current supported commands:");
      Serial.println("?: this help");
      Serial.println("a: Button 1 short press");
      Serial.println("aa: Button 1 long press");
      Serial.println("b: Button 2 short press");
      Serial.println("bb: Button 2 long press");
      Serial.println("r: rotate screen");
      Serial.println("v: switch VLAN tagging");

      //Serial.println( F( "startdhcp: start DHCP and waiting for an IP address" ) );
    }  // if( command == "?" )
    else if (command == "a")
      btn_btn1ShortClick();
    else if (command == "aa")
      btn_btn1LongClick();
    else if (command == "b")
      btn_btn2ShortClick();
    //  else if (command == "bb")
    //      btn_btn2LongClick();
    else if (command == "r") {
      tft_rotateScreen();
    } else if (command == "v") {
      if (eth_voiceVLAN != 0) {
        Serial.print("VLAN tagging has been ");
        if (ENC28J60::is_VLAN_tagging_enabled()) {
          ENC28J60::disable_VLAN_tagging();
          eth_vLANTagging = false;
          Serial.println("disabled.\n");
        }  // if( ENC28J60::is_VLAN_tagging_enabled() )
        else {
          ENC28J60::enable_VLAN_tagging(eth_voiceVLAN);
          eth_vLANTagging = true;
          Serial.print("enabled. ID:");
          Serial.print(String(eth_voiceVLAN, DEC));
          Serial.println("\n");
        }  // else
      }    // if( eth_voiceVLAN != 0 )
      else {
        Serial.println("VLAN tagging can't be enabled, no voice VLAN information received yet.");
      }
    }  // else if(command == "v" )
    else {
#ifdef USE_BTSERIAL
      ser_HardwareA.printf("%s", command.c_str());
#endif
    }  // else
  }    // if( Serial.available() )
}  // void dbg_process()
#endif


// *************************************************************************
// WiFi functions
void wifi_process() {
  if (wifi_isEnabled) {
    if ((wifi_currentCheck != WIFI_SCAN_RUNNING) && (wifi_disableIfPossible)) {
      wifi_disableIfPossible = false;
      wifi_disable();
      wifi_currentCheck = 0;
    } else {
      if ((gen_currentMillis - wifi_lastScanMillis > WIFI_SCANPERIOD) && (wifi_isScanComplete)) {
        tft_updateHeader(false);
        // Trigger Wi-Fi network scan
        wifi_isScanComplete = false;
        WiFi.scanNetworks(true);
        Serial.print("\nScan start ... ");
        wifi_lastScanMillis = gen_currentMillis;
        wifi_lastCheckMillis = gen_currentMillis;
      }

      if ((gen_currentMillis - wifi_lastCheckMillis > WIFI_SCANCHECKPERIOD) && (!wifi_isScanComplete)) {
        wifi_currentCheck = wifi_checkScanState();
        wifi_lastCheckMillis = gen_currentMillis;
      }
    }
  }
}  // void wifi_process( void )


// *************************************************************************
// Bluetooth Serial functions
#ifdef USE_BTSERIAL
void bt_process() {
  // Check BT connection request timeout
  if ((bt_isConnectRequest) && (gen_currentMillis > bt_connectRequestStarttime + BT_CONNECTREQUESTTIMEOUT)) {
    SerialBT.confirmReply(false);
    bt_isConnectRequest = false;
    bt_isConnected = false;
    bt_connectRequestStarttime = 0;
    tft_displayData1[TFT_HEADERENTRY_BT].color = TFT_BLACK;
    tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_BLACK;
    tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = false;  // Disable logging to SD selection
#ifdef DEBUGSERIAL
    Serial.println("bt_process(): BT TFT_BLACK");
    Serial.println("bt_process(): SERA TFT_BLACK");
    Serial.println("bt_process(): tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = false");
#endif
    tft_showPage();
#ifdef DEBUGSERIAL
    Serial.println("Bluetooth conection aborted due to timeout.");
#endif
    return;
  } // if ((bt_isConnectRequest) && (gen_currentMillis > bt_connectRequestStarttime + BT_CONNECTREQUESTTIMEOUT))

  // Only transfer data between Serial port A and Bluetooth serial.
  enum SerialSource { none,
                      serA,
                      serBT };
  static SerialSource lastSource = none;
  byte count = 0;
  byte incomingByte = 0;  // for incoming serial data
  String BTcommand;       // Command read from BT serial

  // Check if data von Serial port A is available
  if (ser_HardwareA.available()) {
    if ((lastSource != serA) || (count > 15)) {
#ifdef DEBUGSERIAL
      Serial.printf("\n%ld SerA:", millis());
#endif
      lastSource = serA;
      count = 0;
    } // if ((lastSource != serA) || (count > 15))
    incomingByte = ser_HardwareA.read();
    SerialBT.write(incomingByte);
#ifdef DEBUGSERIAL
    Serial.printf(" %0x", incomingByte);
#endif
#ifdef USE_SDCARD
    if (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value == bt_SerLogYes) {
#ifdef DEBUGSERIAL
      if (!file.print(incomingByte))
        Serial.println("bt_process(): writing to SD failed.");
#else
      file.print(incomingByte);
#endif
    } // if (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value == bt_SerLogYes)
#endif
    count++;

    if (SerialBT.available()) {
      incomingByte = SerialBT.read();
#ifdef DEBUGSERIAL
      Serial.write(incomingByte);
#endif
#ifdef USE_SDCARD
      if (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value == bt_SerLogYes) {
#ifdef DEBUGSERIAL
        if (!file.print(incomingByte))
          Serial.println("bt_process(): writing to SD failed.");
#else
        file.print(incomingByte);
#endif
      } // if (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value == bt_SerLogYes)
#endif
    } // if (SerialBT.available())
  } // if (ser_HardwareA.available())
}  // void bt_process( void )
#endif


// *************************************************************************
// Serial logger functions
// Initialize serial ports if configured to use
void ser_initialize() {
  // Setup custom serial port for TTL.
#if defined(USE_BTSERIAL) || defined(USE_SERIALLOGGER)
#ifdef DEBUGSERIAL
  Serial.println("Initialize COM A");
#endif
  ser_HardwareA.setRxBufferSize(SER_BUFFER);
  tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_BLACK;
  tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].isActive = true;
  tft_userMenu[TFT_MENUENTRY_SERIALSPEED].isActive = true;
#endif

#ifdef USE_SERIALLOGGER
#ifdef DEBUGSERIAL
  Serial.println("Initialize COM B");
#endif
  ser_HardwareB.setRxBufferSize(SER_BUFFER);
  tft_displayData1[TFT_HEADERENTRY_SERB].color = TFT_BLACK;
#endif

#if defined(USE_BTSERIAL) || defined(USE_SERIALLOGGER)
  pinMode(SER_RXPIN1, INPUT);
  pinMode(SER_TXPIN1, OUTPUT);
  //ser_HardwareA.begin(ser_Speed, SERIAL_8N1, SER_RXPIN1, SER_TXPIN1);  // RX, TX
  ser_HardwareA.begin(ser_speeds[tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value], ser_configurations[tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value].serConfig, SER_RXPIN1, SER_TXPIN1);  // RX, TX
#endif

#ifdef USE_SERIALLOGGER
  pinMode(SER_RXPIN2, INPUT);
  pinMode(SER_TXPIN2, OUTPUT);
  //ser_HardwareB.begin(ser_Speed, SERIAL_8N1, SER_RXPIN2, SER_TXPIN2);  // RX, TX
  ser_HardwareB.begin(ser_speeds[tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value], ser_configurations[tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value].serConfig, SER_RXPIN2, SER_TXPIN2);  // RX, TX
#endif
}  // void ser_initialize()


#ifdef USE_SERIALLOGGER
// Enable Serial logger
void ser_enableLogger() {
  // Take the mutex for writing to SD card
  if (xSemaphoreTake(xMutex_sd_card, pdMS_TO_TICKS(SD_SEMA_WAIT)) == pdTRUE) {
    file = SD.open(SD_SERLOGFILENAME, FILE_APPEND);
    if (!file) {
      xSemaphoreGive(xMutex_sd_card);
#ifdef DEBUGSERIAL
      Serial.println("Failed to open file for appending");
#endif
    }  // if( !file )
    else {
      tft_displayData1[TFT_HEADERENTRY_SD].color = TFT_DARKGREEN;
      tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_DARKGREEN;
      tft_displayData1[TFT_HEADERENTRY_SERB].color = TFT_DARKGREEN;
      tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].isActive = true;
      tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive = false;
#ifdef DEBUGSERIAL
      Serial.println("ser_enableLogger(): SD TFT_DARKGREEN");
      Serial.println("ser_enableLogger(): SERA TFT_DARKGREEN");
      Serial.println("ser_enableLogger(): SERB TFT_DARKGREEN");
#endif
      // Create a long string first, than write it in one step to the SD card
      String separatorStr = "----------------------------------------\nSerial logging started: ";

#ifdef USE_RTCTIME
      if (ertc_present) {
        // Read current time from RTC
        char curDat[30];  // "HH:MM yyyy-mm-dd0"
        DateTime now = rtc.now();
        sprintf(curDat, "%02u:%02u %04u-%02u-%02u\n", now.hour(), now.minute(), now.year(), now.month(), now.day());
        separatorStr += String(curDat) + "\n";
      }  // if( ertc_present )
      else {
        separatorStr += esprtc.getTime("%A, %B %d %Y %H:%M:%S") + " (UTC)\n";
      }
#else
      separatorStr += esprtc.getTime("%A, %B %d %Y %H:%M:%S") + " (UTC)\n";
#endif

      // Write separator
#ifdef DEBUGSERIAL
      if (file.println(separatorStr))
        Serial.println("ser_enableLogger(): separator written.");
      else
        Serial.println("ser_enableLogger(): writing separator failed.");
#else
      file.println(separatorStr);
#endif
    }  // else
  }    // if (xSemaphoreTake(xMutex_sd_card, pdMS_TO_TICKS(SD_SEMA_WAIT)) == pdTRUE)
}  // void ser_enableLogger()

// Disable Serial logger
void ser_disableLogger() {
  file.close();
  xSemaphoreGive(xMutex_sd_card);
  tft_displayData1[TFT_HEADERENTRY_SD].color = TFT_BLACK;
  tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_BLACK;
  tft_displayData1[TFT_HEADERENTRY_SERB].color = TFT_BLACK;
  tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].isActive = false;  // Disable logging mode selection
  tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive = true;
#ifdef DEBUGSERIAL
  Serial.println("ser_disableLogger(): SD TFT_BLACK");
  Serial.println("ser_disableLogger(): SERA TFT_BLACK");
  Serial.println("ser_disableLogger(): SERB TFT_BLACK");
  Serial.println("ser_disableLogger(): tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].isActive = false");
#endif
}  // void ser_disableLogger()

// Log Com data to SD
void ser_process(void) {
  static uint8_t buffer[512];
  size_t bytesRead;
  uint16_t i;
  static byte count = 0;
  enum SerialSource { none,
                      serA,
                      serB,
                      serBT };
  static SerialSource lastSource = none;

  if (ser_HardwareA.available()) {
    bytesRead = ser_HardwareA.read(buffer, sizeof(buffer));
    ser_HardwareB.write(buffer, bytesRead);

    if ((lastSource != serA) || (count > 15)) {
#ifdef DEBUGSERIAL
      Serial.print("\n" + String(millis()) + " SerA: ");
      if (!file.printf("\n%lu SerA: ", millis()))
        Serial.println("ser_process(): writing to SD failed.");
#else
      file.printf("\n%lu SerA: ", millis());
#endif
      lastSource = serA;
      count = 0;
    }  // if( ( lastSource != serA ) || ( count > 15 ) )

    if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeASCII) {
#ifdef DEBUGSERIAL
      Serial.write(buffer, bytesRead);
      if (!file.write(buffer, bytesRead))
        Serial.println("ser_process(): writing to SD failed.");
#else
      file.write(buffer, bytesRead);
#endif
    }  // if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeASCII)
    else if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeHEX) {
      for (i = 0; i < bytesRead; i++) {
#ifdef DEBUGSERIAL
        Serial.printf("%x ", buffer[i]);
        if (!file.printf("%x ", buffer[i]))
          Serial.println("ser_process(): writing to SD failed.");
#else
        file.printf("%x ", buffer[i]);
#endif
        count++;
        if (count > 15) {
#ifdef DEBUGSERIAL
          Serial.printf("\n");
          if (!file.printf("\n"))
            Serial.println("ser_process(): writing to SD failed.");
#else
          file.printf("\n");
#endif
          count = 0;
        }  // if( count > 15 )
      }    // for( i = 0; i < bytesRead, i++ )
    }      // else if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeHEX)
  }        // if( ser_HardwareA.available() )

  if (ser_HardwareB.available()) {
    bytesRead = ser_HardwareB.read(buffer, sizeof(buffer));
    ser_HardwareA.write(buffer, bytesRead);

    if ((lastSource != serB) || (count > 15)) {
#ifdef DEBUGSERIAL
      Serial.print("\n" + String(millis()) + " SerB: ");
      if (!file.printf("\n%lu SerB: ", millis()))
        Serial.println("ser_process(): writing to SD failed.");
#else
      file.printf("\n%lu SerB: ", millis());
#endif
      lastSource = serB;
      count = 0;
    }  // if( ( lastSource != serB ) || ( count > 15 ) )

    if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeASCII) {
#ifdef DEBUGSERIAL
      Serial.write(buffer, bytesRead);
      if (!file.write(buffer, bytesRead))
        Serial.println("ser_process(): writing to SD failed.");
#else
      file.write(buffer, bytesRead);
#endif
    }  // if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeASCII)
    else if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeHEX) {
      for (i = 0; i < bytesRead; i++) {
#ifdef DEBUGSERIAL
        Serial.printf("%x ", buffer[i]);
        if (!file.printf("%x ", buffer[i]))
          Serial.println("ser_process(): writing to SD failed.");
#else
        file.printf("%x ", buffer[i]);
#endif
        count++;
        if (count > 15) {
#ifdef DEBUGSERIAL
          Serial.printf("\n");
          if (!file.printf("\n"))
            Serial.println("ser_process(): writing to SD failed.");
#else
          file.printf("\n");
#endif
          count = 0;
        }  // if( count > 15 )
      }    // for( i = 0; i < bytesRead, i++ )
    }      // if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == modeHEX)
  }        // if( ser_HardwareB.available() )
}  // void ser_process( void )
#endif


// *************************************************************************
// TFT functions
// Initialize TFT
void tft_initialize() {
  tft.init();
  tft.setTextFont(TFT_FONTNUMBER);
  tft_fontHeight = tft.fontHeight();  // get used font height in pixels
  tft.setTextSize(TFT_SIZESCALER);
  //tft.setRotation(tft_rotation);
  tft.setRotation(tft_userMenu[TFT_MENUENTRY_ROTATESCREEN].value);
  tft_spaceWidth = (tft.textWidth(" ", TFT_FONTNUMBER) >> 1) - 1;
  tft_arrowWidth = tft.textWidth("> ", TFT_FONTNUMBER);

  // Calculate the area used for displaying data
  tft_userY = (2 * tft_fontHeight * TFT_SIZESCALER) + (tft_fontHeight / 2) + 1;
  tft_userHeight = tft_height;

  // Clear the status area
  tft.fillRect(0, 0, tft_width, tft_userY - 1, TFT_WHITE);

  // Calculate positions for the top rows
  uint16_t tmpXPos = 0;
  for (byte i = 0; i < TFT_IDISPLAYDATA1COUNT; i++) {
    tft_displayData1[i].yPos = 1;
    tft_displayData1[i].xPos = tmpXPos;
    tmpXPos += tft_spaceWidth + tft.textWidth(tft_displayData1[i].text);
  }
  tmpXPos = 0;
  for (byte i = 0; i < TFT_IDISPLAYDATA2COUNT; i++) {
    tft_displayData2[i].yPos = tft_fontHeight * 2 + 2;
    tft_displayData2[i].xPos = tmpXPos;
    tmpXPos += tft_spaceWidth + tft.textWidth(tft_displayData2[i].text);
  }
}

// Top rows / header of TFT
void tft_updateHeader(bool fillrect) {
  // Clear the status area
  if (fillrect)
    tft.fillRect(0, 0, tft_width, tft_userY - 1, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  // First row
  // SD card - the header wont be updated while the SD card is accessed. Usually the access is less
  // than a second so we won't see anything in the header.
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_SD].color, TFT_WHITE);
  tft.drawString(tft_displayData1[TFT_HEADERENTRY_SD].text, tft_displayData1[TFT_HEADERENTRY_SD].xPos, tft_displayData1[TFT_HEADERENTRY_SD].yPos);

  // Ethernet connection
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_ETH].color, TFT_WHITE);
  tft.drawString(tft_displayData1[TFT_HEADERENTRY_ETH].text, tft_displayData1[TFT_HEADERENTRY_ETH].xPos, tft_displayData1[TFT_HEADERENTRY_ETH].yPos);

  // BT connection
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_BT].color, TFT_WHITE);
  tft.drawString(tft_displayData1[TFT_HEADERENTRY_BT].text, tft_displayData1[TFT_HEADERENTRY_BT].xPos, tft_displayData1[TFT_HEADERENTRY_BT].yPos);

  // Com A
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_SERA].color, TFT_WHITE);
  tft.drawString(tft_displayData1[TFT_HEADERENTRY_SERA].text, tft_displayData1[TFT_HEADERENTRY_SERA].xPos, tft_displayData1[TFT_HEADERENTRY_SERA].yPos);

  // Com B
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_SERB].color, TFT_WHITE);
  tft.drawString(tft_displayData1[TFT_HEADERENTRY_SERB].text, tft_displayData1[TFT_HEADERENTRY_SERB].xPos, tft_displayData1[TFT_HEADERENTRY_SERB].yPos);

  // RTC
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_RTC].color, TFT_WHITE);
  tft.drawString(tft_displayData1[TFT_HEADERENTRY_RTC].text, tft_displayData1[TFT_HEADERENTRY_RTC].xPos, tft_displayData1[TFT_HEADERENTRY_RTC].yPos);

  // WiFi device
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_WIFI].color, TFT_WHITE);
  tft.drawString(tft_displayData1[TFT_HEADERENTRY_WIFI].text, tft_displayData1[TFT_HEADERENTRY_WIFI].xPos, tft_displayData1[TFT_HEADERENTRY_WIFI].yPos);

  // Battery
  tft.setTextColor(tft_displayData1[TFT_HEADERENTRY_BAT].color, TFT_WHITE);
  tft.setCursor(tft_displayData1[TFT_HEADERENTRY_BAT].xPos, tft_displayData1[TFT_HEADERENTRY_BAT].yPos);
  tft.print(bat_chargeLevelText);

  // Second row
  // Information screen
  if (disp_currentScreen == TFT_SCREEN_INFO)
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);
  else
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString(tft_displayData2[TFT_HEADERENTRY_INFO].text, tft_displayData2[TFT_HEADERENTRY_INFO].xPos, tft_displayData2[TFT_HEADERENTRY_INFO].yPos);

  // DHCP screen
  if (disp_currentScreen == TFT_SCREEN_DHCP)
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);
  else {
    if (eth_dhcpReceived)
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    else
      tft.setTextColor(TFT_SILVER, TFT_WHITE);
  }
  tft.drawString(tft_displayData2[TFT_HEADERENTRY_DHCP].text, tft_displayData2[TFT_HEADERENTRY_DHCP].xPos, tft_displayData2[TFT_HEADERENTRY_DHCP].yPos);

  // vDHCP screen
  if (disp_currentScreen == TFT_SCREEN_DHCPVLAN)
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);
  else {
    if (eth_vlanDhcpReceived)
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    else
      tft.setTextColor(TFT_SILVER, TFT_WHITE);
  }
  tft.drawString(tft_displayData2[TFT_HEADERENTRY_VDHCP].text, tft_displayData2[TFT_HEADERENTRY_VDHCP].xPos, tft_displayData2[TFT_HEADERENTRY_VDHCP].yPos);

  // LLDP screens
  if ((disp_currentScreen == TFT_SCREEN_LLDP1) || (disp_currentScreen == TFT_SCREEN_LLDP2))
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);
  else {
    if (eth_lldpPacketReceived)
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    else
      tft.setTextColor(TFT_SILVER, TFT_WHITE);
  }
  tft.drawString(tft_displayData2[TFT_HEADERENTRY_LLDP].text, tft_displayData2[TFT_HEADERENTRY_LLDP].xPos, tft_displayData2[TFT_HEADERENTRY_LLDP].yPos);

  // CDP screens
  if ((disp_currentScreen == TFT_SCREEN_CDP1) || (disp_currentScreen == TFT_SCREEN_CDP2))
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);
  else {
    if (eth_cdpPacketReceived)
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    else
      tft.setTextColor(TFT_SILVER, TFT_WHITE);
  }
  tft.drawString(tft_displayData2[TFT_HEADERENTRY_CDP].text, tft_displayData2[TFT_HEADERENTRY_CDP].xPos, tft_displayData2[TFT_HEADERENTRY_CDP].yPos);

  // NTP
  if (disp_currentScreen == TFT_SCREEN_NTP)
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);
  else {
    if (eth_cdpPacketReceived)
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    else
      tft.setTextColor(TFT_SILVER, TFT_WHITE);
  }
  tft.drawString(tft_displayData2[TFT_HEADERENTRY_NTP].text, tft_displayData2[TFT_HEADERENTRY_NTP].xPos, tft_displayData2[TFT_HEADERENTRY_NTP].yPos);

  // WiFi screens
  if ((disp_currentScreen == TFT_SCREEN_WIFIS) && (wifiList.size() > 0))
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);
  else {
    if (wifiList.size() > 0)
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    else
      tft.setTextColor(TFT_SILVER, TFT_WHITE);
  }
  tft.drawString(tft_displayData2[TFT_HEADERENTRY_WIFIDATA].text, tft_displayData2[TFT_HEADERENTRY_WIFIDATA].xPos, tft_displayData2[TFT_HEADERENTRY_WIFIDATA].yPos);
}  // void tft_updateHeader( bool fillrect )

// Print text array on TFT, separated by colon
void tft_drawText(String value[2]) {
  if (value[1] != "-") {
    tft.setTextColor(TFT_GREEN);
    tft.print(value[0] + ":");
    tft.setTextColor(TFT_WHITE);
    tft.println(value[1]);
  }
}  // void tft_drawText(String value[2])

// Rotate the screen and store the setting
void tft_rotateScreen() {
  ++tft_userMenu[TFT_MENUENTRY_ROTATESCREEN].value %= 4;
  savePreferencesOrientation(tft_userMenu[TFT_MENUENTRY_ROTATESCREEN].value);
  tft.setRotation(tft_userMenu[TFT_MENUENTRY_ROTATESCREEN].value);
  tft_updateHeader(true);
  tft_showPage();
}  // void tft_rotateScreen()

// Change screen delay and store the setting
void tft_changeDisplayAutoSwitchDelay() {
  disp_autoSwitch = true;
  // Switch between those values: 0=off, 5s, 10s, 15s, 20s, 30s
  switch (tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value) {
    case 0:
      tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = 5;
      break;
    case 5:
      tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = 10;
      break;
    case 10:
      tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = 15;
      break;
    case 15:
      tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = 20;
      break;
    case 20:
      tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = 30;
      break;
    case 30:
      tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = 0;
      disp_autoSwitch = false;
      break;
    default:
      tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value = 10;
      break;
  }
}  // void tft_changeDisplayAutoSwitchDelay()

// Switch through screens
void tft_switchScreen() {
  // Don't switch screen if menu is displayed
#ifdef USE_BTSERIAL
  if ((disp_bDisplayMenu) || (bt_isConnectRequest))
    return;
#else
  if (disp_bDisplayMenu)
    return;
#endif

  gen_previousMillis = millis();
  uint8_t displayedScreenCurrent = disp_currentScreen;
  int16_t displayedWifi = wifi_Current;
  disp_autoSwitchLastTime = gen_previousMillis;

  if (disp_currentScreen == TFT_SCREEN_WIFIS)  // WiFi
  {
    if (wifi_scanForSSID == NULL) {
      if (wifi_getNextWifiToDisplay()) {
        // A WiFi has been found for displaying
        tft_showPage();
        return;
      }
    }  // if (wifi_scanForSSID == NULL)
    else {
      tft_showPage();
      return;
    }
  }

  disp_currentScreen++;
  if (disp_currentScreen > TFT_SCREEN_WIFIS)
    disp_currentScreen = TFT_SCREEN_INFO;

  // Check if data for displaying is available
  if ((disp_currentScreen == TFT_SCREEN_DHCP) && (eth_dhcpInfo[0][0].Option[1] == "-"))  // IP address in option field 0
    disp_currentScreen++;
  if ((disp_currentScreen == TFT_SCREEN_DHCPVLAN) && (eth_dhcpInfo[1][0].Option[1] == "-"))  // IP address in option field 0 for voice VLAN?
    disp_currentScreen++;
  if ((disp_currentScreen == TFT_SCREEN_LLDP1) && (!eth_lldpPacketReceived))
    disp_currentScreen++;
  if ((disp_currentScreen == TFT_SCREEN_LLDP2) && (!eth_lldpPacketReceived))
    disp_currentScreen++;
  if ((disp_currentScreen == TFT_SCREEN_CDP1) && (!eth_cdpPacketReceived))
    disp_currentScreen++;
  if ((disp_currentScreen == TFT_SCREEN_CDP2) && (!eth_cdpPacketReceived))
    disp_currentScreen++;
  if ((disp_currentScreen == TFT_SCREEN_NTP) && (!eth_ntpReceived))
    disp_currentScreen++;
  if (disp_currentScreen == TFT_SCREEN_WIFIS)  // WiFi
  {
    if (wifi_scanForSSID == NULL) {
      if (!wifi_getNextWifiToDisplay()) {
        // No (more) WiFi found to display so change the screen else stay at the WiFi screen
        disp_currentScreen++;
      }
    }
  }

  if (disp_currentScreen > TFT_SCREEN_WIFIS)
    disp_currentScreen = TFT_SCREEN_INFO;

  // Only update screen if it has changed
  if ((displayedScreenCurrent != disp_currentScreen) || (displayedWifi != wifi_Current)) {
    tft_showPage();
  }
}  // void tft_switchScreen()

// Clear the user data area
void tft_clearUserArea() {
  tft.fillRect(0, tft_userY, tft_width, tft_userHeight, TFT_BLACK);
  tft.setCursor(0, tft_userY + 1, 1);
}  // void tft_clearUserArea()

// Start screen with general information
void tft_startScreen() {
  uint8_t row = 0;
  tft.setTextColor(TFT_YELLOW);

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_GEN_DEVNAME);

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_GEN_VERSION);

#ifdef USE_RTCTIME
  if ((ertc_present) && (esprtc.getYear() > 2000)) {
    tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row);
    tft.print(esprtc.getTime("%F %R") + "UTC");
  }
#endif
  row++;

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_GEN_PROPERTYOF);

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_GEN_OWNER);

  row++;
  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(String(TXT_GEN_DEVNAME));

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(eth_myMACString);

#ifdef USE_SDCARD
  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  if (sd_available) {
    tft.print(TXT_SD_CARD);
    tft.print(": " + String(sd_cardSize) + "MB\n");
    tft.print(TXT_SD_CARDFREE);
    tft.print(": " + String(sd_cardSizeFree) + "MB");
  } else {
    tft.print(TXT_SD_NOTAVAILABLE);
  }
#endif
} // void tft_startScreen()

// DHCP screen
void tft_dhcpScreen(byte selection) {
  tft.setCursor(0, tft_userY);

  // IP address received using DHCP
  if (eth_dhcpInfo[selection][0].Option[1] != "-")
    tft_drawText(eth_dhcpInfo[selection][0].Option);

  // Network mask
  if (eth_dhcpInfo[selection][1].Option[1] != "-")
    tft_drawText(eth_dhcpInfo[selection][1].Option);

  // Default gateway
  if (eth_dhcpInfo[selection][3].Option[1] != "-")
    tft_drawText(eth_dhcpInfo[selection][3].Option);

  // Server IP
  if (eth_dhcpInfo[selection][54].Option[1] != "-")
    tft_drawText(eth_dhcpInfo[selection][54].Option);

  // Lease time
  if (eth_dhcpInfo[selection][51].Option[1] != "-")
    tft_drawText(eth_dhcpInfo[selection][51].Option);

  // DNS Domain name
  if (eth_dhcpInfo[selection][15].Option[1] != "-")
    tft_drawText(eth_dhcpInfo[selection][15].Option);

  // DNS
  if (eth_dhcpInfo[selection][6].Option[1] != "-")
    tft_drawText(eth_dhcpInfo[selection][6].Option);
} // void tft_dhcpScreen(byte selection)

// Display info data for LLDP or CDP
void tft_discoveryScreen(PINFO *info) {
  tft.setCursor(0, tft_userY);

  // Print used port
  if (info->Port[1] != "-")
    tft_drawText(info->Port);

  // Print port description
  if (info->PortDesc[1] != "-")
    tft_drawText(info->PortDesc);

  // Print VLAN
  if (info->VLAN[1] != "-")
    tft_drawText(info->VLAN);

  // Print Voice VLAN
  if (info->VoiceVLAN[1] != "-")
    tft_drawText(info->VoiceVLAN);

  // Print Switch name
  if (info->SWName[1] != "-")
    tft_drawText(info->SWName);

  // Print Switch domain
  if (info->SWDomain[1] != "-")
    tft_drawText(info->SWDomain);

  // Print Switch IP
  if (info->IP[1] != "-")
    tft_drawText(info->IP);

  // Print Switch MAC
  if (info->MAC[1] != "-")
    tft_drawText(info->MAC);

  // Print Power over Ethernet available
  if (info->PoEAvail[1] != "-")
    tft_drawText(info->PoEAvail);

  // Print Power over Ethernet consumed
  if (info->PoECons[1] != "-")
    tft_drawText(info->PoECons);
} // void tft_discoveryScreen(PINFO *info)

// Display info data for LLDP or CDP
void tft_discoveryScreen2(PINFO *info) {
  tft.setCursor(0, tft_userY);

  // Print capabilities
  if (info->Cap[1] != "-")
    tft_drawText(info->Cap);

  // Print model
  if (info->Model[1] != "-")
    tft_drawText(info->Model);
} // void tft_discoveryScreen2(PINFO *info)

// Display info data for NTP
void tft_ntpScreen() {
  uint8_t row = 0;
  tft.setTextColor(TFT_YELLOW);

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_ETH_NTPSERVER);
  tft.print(":");

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(eth_ntpServer);
} // void tft_ntpScreen()

// Display user menu
void tft_displayMenu() {
  uint8_t row = 0;
  tft.setTextWrap(false);
  tft.setTextColor(TFT_YELLOW);

  // 1st row: Ethernet
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_ETHERNET].text);
  tft.print(":");
  if (gen_currentFunction == fEthernet)
    tft.print(TXT_GEN_ON);
  else
    tft.print(TXT_GEN_OFF);

  // -----
  // 2nd row: WiFi
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_WIFI].text);
  tft.print(":");
  if (gen_currentFunction == fWiFi)
    tft.print(TXT_GEN_ON);
  else
    tft.print(TXT_GEN_OFF);

    // -----
    // 3rd row: Bluetooth serial
#ifdef USE_BTSERIAL
  tft.setTextColor(TFT_YELLOW);
#else
  tft.setTextColor(TFT_SILVER);
#endif
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_BTSERIAL].text);
  tft.print(":");
#ifdef USE_BTSERIAL
  if (gen_currentFunction == fBluetoothSerial)
    tft.print(TXT_GEN_ON);
  else
    tft.print(TXT_GEN_OFF);
#else
  tft.print(TXT_GEN_OFF);
#endif

  // 4th row: Bluetooth serial logging to SD card
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  // Set color to yellow if Bluetooth serial is enabled
  if (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive)
    tft.setTextColor(TFT_YELLOW);
  else
    tft.setTextColor(TFT_SILVER);
  tft.print(tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].text);
  tft.print(":");
  if ((gen_currentFunction == fBluetoothSerial) && (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive) && (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value == bt_SerLogYes))
    tft.print(TXT_GEN_ON);
  else
    tft.print(TXT_GEN_OFF);

  // -----
  // 5th row: Serial Logging
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_SERIALLOGGING].text);
  tft.print(":");
  if (gen_currentFunction == fSerialLogger)
    tft.print(TXT_GEN_ON);
  else
    tft.print(TXT_GEN_OFF);

  // 6th row: Serial Logging type
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].isActive)
    tft.setTextColor(TFT_YELLOW);
  else
    tft.setTextColor(TFT_SILVER);
  tft.print(tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].text);
  tft.print(":");
  if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == ser_LogASCII)
    tft.print(TXT_SERLOGGING_ASCII);
  else
    tft.print(TXT_SERLOGGING_HEX);

  // -----
  // 7th row: Serial configuration
  if (tft_userMenu[TFT_MENUENTRY_SERIALSPEED].isActive)
    tft.setTextColor(TFT_YELLOW);
  else
    tft.setTextColor(TFT_SILVER);
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_SERIALSPEED].text);
  tft.print(":");
  if (tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value < sizeof(ser_speeds) / sizeof(ser_speeds[0])) {
    tft.print(ser_speeds[tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value]);
  } else {
    tft.print("-");
  }

  // 8th row: Serial configuration
  if (tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].isActive)
    tft.setTextColor(TFT_YELLOW);
  else
    tft.setTextColor(TFT_SILVER);
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].text);
  tft.print(":");
  tft.print(ser_configurations[tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value].serName);


  // -----
  // 9th row: Default function: none, Ethernet, WiFi, BT serial, Serial logging
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].text);
  tft.print(":");
  switch (tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value) {
    case fEthernet:
      tft.print(TXT_GEN_ETHERNET);
      break;
    case fWiFi:
      tft.print(TXT_WIFI_NAME);
      break;
#ifdef USE_BTSERIAL
    case fBluetoothSerial:
      tft.print(TXT_BT_SERIAL);
      break;
#endif
#ifdef USE_SERIALLOGGER
    case fSerialLogger:
      tft.print(TXT_SER_SERLOGGING);
      break;
#endif
    case fNone:
      tft.print(TXT_GEN_NONE);
      break;
  }

    // -----
    // 10th row: Write current collected data to log
#ifdef USE_SDCARD
  if (tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive)
    tft.setTextColor(TFT_YELLOW);
  else
    tft.setTextColor(TFT_SILVER);
#else
  tft.setTextColor(TFT_SILVER);
#endif
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_WRITETOLOG].text);

  // -----
  // 11th row: Rotate screen
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_ROTATESCREEN].text);

  // -----
  // 12th row: Switch delay
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(tft_arrowWidth, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].text);
  tft.print(":");
  if(tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value != 0)
    tft.print(tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].value);
  else
    tft.print(TXT_GEN_OFF);


  // Print arrow at the current position
  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * tft_userMenuPos);
  tft.print("> ");

  tft.setTextWrap(true);
} // void tft_displayMenu()

// Display the current page
void tft_showPage() {
  tft_clearUserArea();

#ifdef USE_BTSERIAL
  // Handle Bluetooth connection request with priority
  if (bt_isConnectRequest) {
#ifdef DEBUGSERIAL
    Serial.println("handle Bluetooth request");
    Serial.flush();
#endif
    bt_connectScreen();
  } // if (bt_isConnectRequest)
  else
#endif
  {
    if (disp_bDisplayMenu) {
      tft_displayMenu();
    } else {
      switch (disp_currentScreen) {
        case TFT_SCREEN_INFO:
          // Start screen
          tft_startScreen();
          break;

        case TFT_SCREEN_DHCP:
          // DHCP screen
          tft_dhcpScreen(0);
          break;

        case TFT_SCREEN_DHCPVLAN:
          // DHCP screen for VoiceVLAN
          tft_dhcpScreen(1);
          break;

        case TFT_SCREEN_LLDP1:
          // LLDP Discovery screen
          if (eth_lldpPacketReceived)
            tft_discoveryScreen(&eth_lldpPacket);
          break;

        case TFT_SCREEN_LLDP2:
          // LLDP Discovery screen 2
          if (eth_lldpPacketReceived)
            tft_discoveryScreen2(&eth_lldpPacket);
          break;

        case TFT_SCREEN_CDP1:
          // CDP Discovery screen
          if (eth_cdpPacketReceived)
            tft_discoveryScreen(&eth_cdpPacket);
          break;

        case TFT_SCREEN_CDP2:
          // CDP Discovery screen 2
          if (eth_cdpPacketReceived)
            tft_discoveryScreen2(&eth_cdpPacket);
          break;

        case TFT_SCREEN_NTP:
          // NTP Discovery screen
          if (eth_ntpReceived)
            tft_ntpScreen();
          break;

        case TFT_SCREEN_WIFIS:
          // Discovered WiFis
          if (wifi_Current != -1)
            tft_wifiScreen();
          break;

        default:
          break;
      }  // switch( disp_currentScreen )
    } // else
  } // else

  tft_updateHeader(false);
} // void tft_showPage()

// Display current WiFi information on TFT
void tft_wifiScreen() {
  wifiClass *tmpWifi;
  uint8_t row = 0;
  int16_t strength = 0;

  // Check if a selected SSID should be displayed
  if (wifi_scanForSSID == NULL) {
    // $$$ to do: check
    tmpWifi = wifiList.get(wifi_Current);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
    tft.print("MAC:" + String(tmpWifi->mac));

    if (tmpWifi->deviceName != NULL) {
      tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
      tft.print(String(tmpWifi->deviceName));
    }

    tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
    tft.print(String(tmpWifi->SSID));
    row++;

    tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
    tft.print(TXT_WIFI_ENCRYPT);  // "Encrypt: "
    tft.print(WIFI_AUTHENTICATIONMODE[tmpWifi->encryption]);

    strength = tmpWifi->strength;
    wifi_setTextColorForStrength(tmpWifi->strength);
    tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
    if (strength > INT16_MIN) {
      tft.print(TXT_WIFI_STRENGTH);  // "Strength: "
      tft.print(String(strength));
      tft.print(" dbm");
    } else {
      tft.print(TXT_WIFI_NOTFOUND);  // "Strength: WiFi not found during the las search"
    }

#ifdef DEBUGSERIAL
    Serial.printf("\nWIFI number: %d of [0 .. %d]\n", wifi_Current, wifiList.size() - 1);
    Serial.printf("MAC: %s\n", tmpWifi->mac);
    if (tmpWifi->deviceName != NULL)
      Serial.printf("Devicename: %s\n", tmpWifi->deviceName);
    Serial.printf("SSID: %s\n", tmpWifi->SSID);
    Serial.printf("Strength: %d dbm\n", tmpWifi->strength);
    Serial.printf("Encryption: %s\n", WIFI_AUTHENTICATIONMODE[tmpWifi->encryption]);
#endif
  }  // if (wifi_scanForSSID == NULL)
  else {
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
    tft.println(String(wifi_scanForSSID));
    row++;
#ifdef DEBUGSERIAL
    Serial.printf("SSID: %s\n", wifi_scanForSSID);
#endif

    for (uint16_t i = 0; i < wifiList.size(); i++) {
      tmpWifi = wifiList.get(i);
      if (strcmp(wifi_scanForSSID, tmpWifi->SSID) == 0) {
        if (tmpWifi->found) {

          tft.setTextColor(TFT_YELLOW);
          if (tmpWifi->deviceName != NULL) {
            tft.println(String(tmpWifi->deviceName));
#ifdef DEBUGSERIAL
            Serial.printf("Devicename: %s\n", tmpWifi->deviceName);
#endif
          } else {
            tft.println("MAC:" + String(tmpWifi->mac));
#ifdef DEBUGSERIAL
            Serial.printf("MAC: %s\n", tmpWifi->mac);
#endif
          }

          strength = tmpWifi->strength;
          wifi_setTextColorForStrength(strength);
          if (strength > INT16_MIN) {
            tft.print(TXT_WIFI_STRENGTH);  // "Strength: "
            tft.print(String(strength));
            tft.println(" dbm");
          } else {
            tft.println(TXT_WIFI_NOTFOUND);  // "Strength: WiFi not found during the las search"
          }
#ifdef DEBUGSERIAL
          Serial.printf("Strength: %d dbm\n", tmpWifi->strength);
#endif
        }
      }
    }
  }
}  // void tft_wifiScreen()


// *************************************************************************
// Battery functions
// Using a voltage divider of 0.3mOhm : 1.0mOhm
// So the input voltage of 4.2v should calculate to:
// v_max=4.2v * (1.0mOhm) / (1.0mOhm + 0.3mOhm) = 4.2v * 1/1.3 = 3,23v
// v_min=3.1v * (1.0mOhm) / (1.0mOhm + 0.3mOhm) = 3.1v * 1/1.3 = 2,38v
// The range up to 3.23v should be fine using max 4.2v of power source.
void bat_getVoltage() {
  uint32_t v = analogReadMilliVolts(BAT_ADC_PIN);
  float _volt = (float)v / 1000.0 * 1.3;   // Calculate back from the voltage divider and milli volt

#ifdef DEBUGSERIALx
  Serial.printf("v=%d\n", v);
  Serial.printf("volt=%f\n", _volt);
  Serial.printf("volt=%1.1fv\n", _volt);
#endif

  if (_volt > 3.7)
    tft_displayData1[TFT_HEADERENTRY_BAT].color = TFT_DARKGREEN;
  else if (_volt > 3.4)
    tft_displayData1[TFT_HEADERENTRY_BAT].color = TFT_ORANGE;
  else if (_volt > 3.2)
    tft_displayData1[TFT_HEADERENTRY_BAT].color = TFT_RED;
  else {
    //Battery too low, so turn off everything and go to sleep!
    gen_hibernate();
  }

  // Update char array
  sprintf(bat_chargeLevelText, "%1.1fv", _volt);

  // Update last request
  bat_lastRequestMillis = gen_currentMillis;
}  // void bat_getVoltage()


// *************************************************************************
// Button functions
// Initialize button handler
void btn_initialize() {
  btn_1.setDebounceTime(50);
  btn_1.setLongClickTime(300);
  btn_1.setDoubleClickTime(400);

  btn_1.setLongClickHandler([](Button2 &b) {
    btn_btn1LongClick();
  });

  btn_1.setClickHandler([](Button2 &b) {
    btn_btn1ShortClick();
  });

  btn_2.setDebounceTime(50);
  btn_2.setLongClickTime(300);
  btn_2.setDoubleClickTime(400);

  //btn_2.setLongClickHandler([](Button2 &b) {
  //  btn_btn2LongClick();
  //});

  btn_2.setClickHandler([](Button2 &b) {
    btn_btn2ShortClick();
  });
}  // void btn_initialize()

// Process button loops
void btn_process() {
  btn_1.loop();
  btn_2.loop();
}  // void btn_process()

// Button 1 long click
// Enter or leave user menu
void btn_btn1LongClick() {
  // Variables to store current configuration before entering the user menu.
  // After leaving compare the values. Update preferences if necessary.
#ifdef USE_BTSERIAL
  static byte bt_currentSerialLogToSD;
#endif
  static byte ser_currentLogMode;
  static byte ser_currentSpeed;
  static byte ser_currentConfiguration;
  static byte gen_currentDefaultFunction;
#ifdef DEBUGSERIAL
  Serial.println("Button 1 long click");
  Serial.flush();
#endif
  btn_click = false;

  // Check current display
  if (!disp_bDisplayMenu) {
    // Switch to displaying the user menu so store the current data
#ifdef USE_BTSERIAL
    bt_currentSerialLogToSD = tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value;
#endif
    ser_currentLogMode = tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value;
    ser_currentSpeed = tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value;
    ser_currentConfiguration = tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value;
    gen_currentDefaultFunction = tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value;
  } else {
    // Leaving the menu
    // Compare value and store modifications to the preferenes
    // Log Bluetooth Serial data to SD card
#ifdef USE_BTSERIAL
    if (bt_currentSerialLogToSD != tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value)
      savePreferencesBTSerLogToSD(tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value);
#endif

    // Serial logging type, ASCII or HEX
    if (ser_currentLogMode != tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value)
      savePreferencesSerLogType(tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value);

    // General serial connection speed index from speed array
    if (ser_currentSpeed != tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value)
      savePreferencesSerSpeed(tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value);

    // General serial connection configuration index
    if (ser_currentConfiguration != tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value)
      savePreferencesSerConfig(tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value);

      // Re-initialize serial ports
#if defined(USE_BTSERIAL) || defined(USE_SERIALLOGGER)
    if ((ser_currentSpeed != tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value) || ser_currentConfiguration != tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value) {
      ser_HardwareA.begin(ser_speeds[tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value], ser_configurations[tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value].serConfig, SER_RXPIN1, SER_TXPIN1);  // RX, TX
#ifdef USE_SERIALLOGGER
      ser_HardwareB.begin(ser_speeds[tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value], ser_configurations[tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value].serConfig, SER_RXPIN2, SER_TXPIN2);  // RX, TX
#endif
    }
#endif

    // Default function
    if (gen_currentDefaultFunction != tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value)
      savePreferencesDefaultFunction(tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value);
  }
  disp_bDisplayMenu = !disp_bDisplayMenu;
  tft_showPage();
}  // void btn_btn1LongClick()

// Button 1 short click
// Switch screen or move to next menu item or accept incoming Bluetooth conenction
void btn_btn1ShortClick() {
  btn_click = false;
#ifdef DEBUGSERIAL
  Serial.println("void btn_btn1ShortClick()");
#endif

#ifdef USE_BTSERIAL
  // Handle Bluetooth connection request with priority
  if (bt_isConnectRequest) {
    bt_isConnectRequest = false;
    SerialBT.confirmReply(true);
    bt_isConnected = true;
    tft_displayData1[TFT_HEADERENTRY_BT].color = TFT_DARKGREEN;
    tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_DARKGREEN;
    tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = true;  // Enaable logging to SD selection
#ifdef DEBUGSERIAL
    Serial.println("btn_btn1ShortClick(): BT TFT_DARKGREEN");
    Serial.println("btn_btn1ShortClick(): SERA TFT_DARKGREEN");
    Serial.println("btn_btn1ShortClick(): tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = true");
#endif
    tft_showPage();
#ifdef DEBUGSERIAL
    Serial.println("Bluetooth conection accepted.");
#endif
  } else {
#endif
    if (disp_bDisplayMenu) {
      tft_userMenuPos++;
      if (tft_userMenuPos >= sizeof(tft_userMenu) / sizeof(tft_userMenu[0])) {
        tft_userMenuPos = 0;
      }
      tft_showPage();
    } else
      tft_switchScreen();
#ifdef USE_BTSERIAL
  }
#endif
}  // void btn_btn1ShortClick()

/*
// Button 2 long click
void btn_btn2LongClick() {
  // not used
#ifdef DEBUGSERIAL
  Serial.println("Button 2 long click");
#endif
  btn_click = false;
}  // void btn_btn2LongClick()
*/

// Button 2 short click
// Enable / disable / toggle menu entry or select WiFi SSID or tracking
void btn_btn2ShortClick() {
  btn_click = false;
#ifdef DEBUGSERIAL
  Serial.println("void btn_btn2ShortClick()");
#endif

#ifdef USE_BTSERIAL
  // Handle Bluetooth connection request with priority
  if (bt_isConnectRequest) {
    bt_isConnectRequest = false;
    SerialBT.confirmReply(false);
    bt_isConnected = false;
    tft_displayData1[TFT_HEADERENTRY_BT].color = TFT_BLUE;
    tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_BLACK;
    tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = true;  // Enaable logging to SD selection
#ifdef DEBUGSERIAL
    Serial.println("btn_btn2ShortClick(): BT TFT_BLUE");
    Serial.println("btn_btn2ShortClick(): BT TFT_BLACK");
    Serial.println("btn_btn2ShortClick(): tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = true");
#endif
    tft_showPage();
#ifdef DEBUGSERIAL
    Serial.println("Bluetooth conection declined.");
#endif
  } else {
#endif
    if (disp_bDisplayMenu) {
      switch (tft_userMenuPos) {
        case TFT_MENUENTRY_ETHERNET:  // Select Ethernet
          {
            if (tft_userMenu[TFT_MENUENTRY_ETHERNET].isActive) {
              gen_switchFunction(fEthernet);
            }
            break;
          }

        case TFT_MENUENTRY_WIFI:  // Select WiFi
          {
            if (tft_userMenu[TFT_MENUENTRY_WIFI].isActive) {
              gen_switchFunction(fWiFi);
            }
            break;
          }

        case TFT_MENUENTRY_BTSERIAL:  // Select Bluetooth serial (BT serial <=> Com A)
          {
            if (tft_userMenu[TFT_MENUENTRY_BTSERIAL].isActive) {
              gen_switchFunction(fBluetoothSerial);
            }
            break;
          }

        case TFT_MENUENTRY_BTSERIALLOGSD:  // Select Bluetooth serial logging to SD card
          {
            if (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive) {
              if (tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value == bt_SerLogNo) {
                tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value = bt_SerLogYes;
                tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive = false;  // Disable export to SD
                if (xSemaphoreTake(xMutex_sd_card, pdMS_TO_TICKS(SD_SEMA_WAIT)) == pdTRUE) {
                  file = SD.open(SD_BTSERLOGFILENAME, FILE_APPEND);
                  if (!file) {
#ifdef DEBUGSERIAL
                    Serial.println("Failed to open file for appending");
#endif
                    xSemaphoreGive(xMutex_sd_card);
                    tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value = bt_SerLogNo;
                    tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive = true;  // Enable export to SD
                  }
                  tft_displayData1[TFT_HEADERENTRY_SD].color = TFT_DARKGREEN;
                } else {
                  tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value = bt_SerLogNo;
                  tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive = true;  // Enable export to SD
                }
              } else {
                file.close();
                xSemaphoreGive(xMutex_sd_card);
                tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].value = bt_SerLogNo;
                tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive = true;  // Enable export to SD
                tft_displayData1[TFT_HEADERENTRY_SD].color = TFT_BLACK;
              }
            }
            break;
          }

        case TFT_MENUENTRY_SERIALLOGGING:  // Select serial port logger (Com A <=> Com B ASCII)
          {
            if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGING].isActive) {
              gen_switchFunction(fSerialLogger);
            }
            break;
          }

        case TFT_MENUENTRY_SERIALLOGGINGMODE:  // Select serial logging mode (toggle between ASCII and HEX)
          {
            if (tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].isActive) {
              tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value = tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].value == ser_LogASCII ? ser_LogHEX : ser_LogASCII;
            }
            break;
          }

        case TFT_MENUENTRY_SERIALSPEED:  // Select serial speed / baud rate
          {
            if (tft_userMenu[TFT_MENUENTRY_SERIALSPEED].isActive) {
              tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value++;
              if (tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value >= ser_speedsCount)
                tft_userMenu[TFT_MENUENTRY_SERIALSPEED].value = 0;
            }
            break;
          }

        case TFT_MENUENTRY_SERIALCONFIGURATION:  // Select serial configuration
          {
            if (tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].isActive) {
              tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value++;
              if (tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value >= ser_configurationsCount)
                tft_userMenu[TFT_MENUENTRY_SERIALCONFIGURATION].value = 0;
            }
            break;
          }

        case TFT_MENUENTRY_DEFAULTFUNCTION:  // Select default mode
          {
            if (tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].isActive) {
              eFunction deffunc = (eFunction)tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value;
              deffunc++;
              tft_userMenu[TFT_MENUENTRY_DEFAULTFUNCTION].value = (byte)deffunc;
            }
            break;
          }

        case TFT_MENUENTRY_WRITETOLOG:  // write received configuration to SD log file
          {
            if (tft_userMenu[TFT_MENUENTRY_WRITETOLOG].isActive) {
              sd_exportData();
            }
            break;
          }


        case TFT_MENUENTRY_ROTATESCREEN:  // Select rotate screen
          {
            if (tft_userMenu[TFT_MENUENTRY_ROTATESCREEN].isActive) {
              tft_rotateScreen();
            }
            break;
          }

        case TFT_MENUENTRY_SCREENSWITCHDELAY:  // Select screen switch delay
          {
            if (tft_userMenu[TFT_MENUENTRY_SCREENSWITCHDELAY].isActive) {
              tft_changeDisplayAutoSwitchDelay();
              break;
            }
          }

        default:
          break;
      }
    } else if (disp_currentScreen == TFT_SCREEN_WIFIS) {
      // WiFis might be displayed
      if (wifi_scanForSSID == NULL) {
        // Currently no scan for a single WiFi
        if (wifi_isEnabled) {
          // WiFi is enabled so it is possible to scan for the current displayed
          if (wifiList.size() > 0) {
            wifiClass *tmpWifi;
            tmpWifi = wifiList.get(wifi_Current);
            wifi_scanForSSID = tmpWifi->SSID;
          }  // if (wifiList.size() > 0)
        }    // if (wifi_isEnabled)
      }      //if (wifi_scanForSSID == NULL)
      else {
        wifi_scanForSSID = NULL;
      }
    } else
      gen_previousMillis = millis();
#ifdef USE_BTSERIAL
  }
#endif

  tft_showPage();
}  // void btn_btn2ShortClick()


//*********************************************
// SD card functions
#ifdef USE_SDCARD
// Initialize SD card
bool sd_initialize() {
  bool init = false;
  wifi_MACloaded = false;

  // Configure SPI for SD using the existent SPI configuration from the TFT_eSPI
  static SPIClass &tmpSPI = TFT_eSPI::getSPIinstance();

  // Depending on the SD card a higher frequency might help to get data fast enough
  if (SD.begin(HSPI_SD_CS, tmpSPI)) {
#ifdef DEBUGSERIAL
    Serial.println("SD card initialized successfull");
    Serial.print("SD card type=");
    switch (SD.cardType()) {
      case CARD_NONE:
        Serial.println("None");
        break;
      case CARD_MMC:
        Serial.println("MMC");
        break;
      case CARD_SD:
        Serial.println("SD");
        break;
      case CARD_SDHC:
        Serial.println("SDHC");
        break;
      case CARD_UNKNOWN:
        Serial.println("Unknown");
        break;
      default:
        Serial.println("Unknown");
        break;
    }
#endif

    // Get card size in MB
    sd_cardSize = SD.cardSize() / 1000000;
    sd_cardSizeFree = (SD.totalBytes() - SD.usedBytes())/ 1000000;
#ifdef DEBUGSERIAL
    Serial.print("Card size:   ");
    Serial.print(SD.cardSize() / 1000000);
    Serial.println("MB");
    Serial.print("Total bytes: ");
    Serial.print(SD.totalBytes() / 1000000);
    Serial.println("MB");
    Serial.print("Used bytes:  ");
    Serial.print(SD.usedBytes() / 1000000);
    Serial.println("MB");
#endif

    init = true;
    tft_displayData1[TFT_HEADERENTRY_SD].color = TFT_BLACK;
  }  // if (SD.begin(HSPI_SD_CS, tmpSPI))
  else {
    tft_displayData1[TFT_HEADERENTRY_SD].color = TFT_SILVER;
#ifdef DEBUGSERIAL
    Serial.println("sd_initialize(): Error talking to SD card!");
#endif
  }

  return init;
}  // bool sd_initialize()

/*
// Unmount SD card
void sd_cardUnmount() {
  if (sd_available) {
#ifdef DEBUGSERIAL
    Serial.print("Unmounting SD card...");
#endif

    // Unmount SD card
    SD.end();

#ifdef DEBUGSERIAL
    Serial.println("done.");
#endif
    sd_available = false;

    tft_updateHeader(false);
  }
}  // void sd_cardUnmount()
*/
#endif

// Check the first bytes of a file if there are unicode byte ordering marks
bool sd_checkUnicode(unsigned char *cFileHeader) {
  // List of Byte Order Marks which have to be checked when opening the "/wifimac.csv"
  // https://en.wikipedia.org/wiki/Byte_order_mark
  bool isUnicode = false;
#ifdef DEBUGSERIAL
  Serial.printf("sd_checkUnicode(): %2x %2x %2x %2x\n", cFileHeader[0], cFileHeader[1], cFileHeader[2], cFileHeader[3]);
#endif

  if (cFileHeader[0] == 0xef && cFileHeader[1] == 0xbb && cFileHeader[2] == 0xbf)
    isUnicode = true;  // UTF-8
  else if (cFileHeader[0] == 0x00 && cFileHeader[1] == 0x00 && cFileHeader[2] == 0xfe && cFileHeader[3] == 0xff)
    isUnicode = true;  // UTF-32 (big endian)
  else if (cFileHeader[0] == 0x00 && cFileHeader[1] == 0x00 && cFileHeader[2] == 0xff && cFileHeader[3] == 0xfe)
    isUnicode = true;  // UTF-32 (little endian)
  else if (cFileHeader[0] == 0xfe && cFileHeader[1] == 0xff)
    isUnicode = true;  // UTF-16 (big endian)
  else if (cFileHeader[0] == 0xff && cFileHeader[1] == 0xfe)
    isUnicode = true;  // UTF-16 (little endian)
  else if (cFileHeader[0] == 0x2b && cFileHeader[1] == 0x2f && cFileHeader[2] == 0x76) {
    if (cFileHeader[3] == 0x2b || cFileHeader[3] == 0x2f || cFileHeader[3] == 0x38 || cFileHeader[3] == 0x39)
      isUnicode = true;  // UTF-7
  } else if (cFileHeader[0] == 0xf7 && cFileHeader[1] == 0x64 && cFileHeader[2] == 0x4c)
    isUnicode = true;  // UTF-1
  else if (cFileHeader[0] == 0xdd && cFileHeader[1] == 0x73 && cFileHeader[2] == 0x66 && cFileHeader[3] == 0x73)
    isUnicode = true;  // UTF-EBCDIC
  else if (cFileHeader[0] == 0x0e && cFileHeader[1] == 0xfe && cFileHeader[2] == 0xff)
    isUnicode = true;  // SCSU
  else if (cFileHeader[0] == 0xfb && cFileHeader[1] == 0xee && cFileHeader[2] == 0x28)
    isUnicode = true;  // BOCU-1
  else if (cFileHeader[0] == 0x84 && cFileHeader[1] == 0x31 && cFileHeader[2] == 0x95 && cFileHeader[3] == 0x33)
    isUnicode = true;  // GB 18030
  return isUnicode;
} // bool sd_checkUnicode(unsigned char *cFileHeader)

#ifdef USE_SDCARD
// Export data to SD card
// - time and date   ok
// - MAC address     ok
// - Device name     ok
// - Software version and ownership   ok
// - DHCP            ok
// - Voip VLAN DHCP  ok
// - NTP             ok
// - CDP             ok
// - LLDP            ok
// - WiFis           ok
void sd_exportData() {
#ifdef DEBUGSERIAL
  Serial.println("void sd_exportData()");
  unsigned long t1, t2;
  t1 = micros();
#endif

  wifiClass *tmpWifi;

  if (!sd_available) {
#ifdef DEBUGSERIAL
    Serial.println("No SD card available.");
#endif
    return;
  }

#ifdef DEBUGSERIAL
  Serial.printf("Update file: %s\n", SD_LOGFILENAME);
#endif

  // Take the mutex for writing to SD card
  if (xSemaphoreTake(xMutex_sd_card, pdMS_TO_TICKS(SD_SEMA_WAIT)) == pdTRUE) {
    file = SD.open(SD_LOGFILENAME, FILE_APPEND);
    if (!file) {
#ifdef DEBUGSERIAL
      Serial.println("Failed to open file for appending");
#endif
      xSemaphoreGive(xMutex_sd_card);
      return;
    }

    // Create a long string first, than write it in one step to the SD card
    String exportStr = "----------------------------------------\n";

#ifdef USE_RTCTIME
    if (ertc_present) {
      // Read current time from RTC
      char curDat[30];  // "HH:MM yyyy-mm-dd0"
      DateTime now = rtc.now();
      sprintf(curDat, "%02u:%02u %04u-%02u-%02u\n", now.hour(), now.minute(), now.year(), now.month(), now.day());
      exportStr += "RTC DS3231 time: " + String(curDat);
    } else {
      exportStr += "External battery buffered RTC DS3231 configured but not found.\n";
    }
#endif

    // Get ESP32 internal time
    // formating options  http://www.cplusplus.com/reference/ctime/strftime/
    exportStr += esprtc.getTime("%A, %B %d %Y %H:%M:%S") + " (UTC)\n";

    // DAMPF device software version, ownership and other specific data
    exportStr += "Version: " + String(TXT_GEN_DEVNAME) + " " + String(TXT_GEN_VERSION) + "\n";
    exportStr += String(TXT_GEN_PROPERTYOF) + " " + String(TXT_GEN_OWNER) + "\nMAC adress: " + eth_myMACString + "\n";
    exportStr += "Device name: " + String(TXT_GEN_DEVNAME) + "\n";

    // Append all received DHCP informations
    if (eth_dhcpInfo[0][0].Option[1] != "-") {
      exportStr += "DHCP data:\n";
      for (byte i = 0; i < 254; i++) {
        if (eth_dhcpInfo[0][i].Option[1] != "-")
          exportStr += String(i, DEC) + ": " + eth_dhcpInfo[0][i].Option[0] + "=" + eth_dhcpInfo[0][i].Option[1] + "\n";
      }
    }

    // Append all received DHCP informations
    if (eth_dhcpInfo[1][0].Option[1] != "-") {
      exportStr += "\nVoIP VLAN DHCP data:\n";
      for (byte i = 0; i < 254; i++) {
        if (eth_dhcpInfo[1][i].Option[1] != "-")
          exportStr += String(i, DEC) + ": " + eth_dhcpInfo[1][i].Option[0] + "=" + eth_dhcpInfo[1][i].Option[1] + "\n";
      }
    }

    // NTP
    if (eth_timeFromNTP > 0l) {
      exportStr += "\nNTP source: ";
      for (byte i = 0; i < IP_LEN; i++) {
        exportStr += String(eth_ntpIPs[eth_currentNTPSource][i]);
        if (i < (IP_LEN - 1))
          exportStr += ".";
      }
      exportStr += "\n";
    }

    // WiFis
    if (wifi_CountFound > 0) {
      exportStr += "\nWiFis found: " + String(wifi_CountFound) + "\n";
      for (uint16_t i = 0; i < wifiList.size(); i++) {
        tmpWifi = wifiList.get(i);
        if ((tmpWifi->found) && (!tmpWifi->ignore)) {
          exportStr += "MAC: " + String(tmpWifi->mac) + " (";
          if (tmpWifi->deviceName != NULL)
            exportStr += String(tmpWifi->deviceName);
          else
            exportStr += "<unknown device>";
          exportStr += "), SSID: " + String(tmpWifi->SSID) + "\n";
        }
      }
    }

    // LLDP Discovery data received
    if (eth_lldpPacketReceived) {
      exportStr += "\nLLDP discover data:\n";
      exportStr += sd_createPInfoString(&eth_lldpPacket);
    }

    // CDP Discovery data received
    if (eth_cdpPacketReceived) {
      exportStr += "\nCDP discover data:\n";
      exportStr += sd_createPInfoString(&eth_cdpPacket);
    }

    // Export data
    if (file.println(exportStr)) {
#ifdef DEBUGSERIAL
      Serial.println("Received data exported");
      Serial.println(exportStr);
#endif
    } else {
#ifdef DEBUGSERIAL
      Serial.println("Export to log failed.");
#endif
    }

    file.close();
    xSemaphoreGive(xMutex_sd_card);
  }

#ifdef DEBUGSERIAL
  Serial.print("end void sd_exportData() Duration: ");
  t2 = micros();
  Serial.println(t2 - t1);
#endif
} // void sd_exportData()

// Create string with gathered data
String sd_createPInfoString(PINFO *info) {
  String tempStr = "";

  if (info->SWName[1] != "-")
    tempStr += info->SWName[0] + "=" + info->SWName[1] + "\n";

  if (info->SWDomain[1] != "-")
    tempStr += info->SWDomain[0] + "=" + info->SWDomain[1] + "\n";

  if (info->MAC[1] != "-")
    tempStr += info->MAC[0] + "=" + info->MAC[1] + "\n";

  if (info->Port[1] != "-")
    tempStr += info->Port[0] + "=" + info->Port[1] + "\n";

  if (info->PortDesc[1] != "-")
    tempStr += info->PortDesc[0] + "=" + info->PortDesc[1] + "\n";

  if (info->Model[1] != "-")
    tempStr += info->Model[0] + "=" + info->Model[1] + "\n";

  if (info->ChassisID[1] != "-")
    tempStr += info->ChassisID[0] + "=" + info->ChassisID[1] + "\n";

  if (info->Proto[1] != "-") {
    tempStr += info->Proto[0] + "=" + info->Proto[1] + " ";
    if (info->ProtoVer[1] != "-")
      tempStr += info->ProtoVer[1] + "\n";
    tempStr += "\n";
  }

  if (info->IP[1] != "-")
    tempStr += info->IP[0] + "=" + info->IP[1] + "\n";

  if (info->Cap[1] != "-")
    tempStr += info->Cap[0] + "=" + info->Cap[1] + "\n";

  if (info->SWver[1] != "-")
    tempStr += info->SWver[0] + "=" + info->SWver[1] + "\n";

  if (info->VLAN[1] != "-")
    tempStr += info->VLAN[0] + "=" + info->VLAN[1] + "\n";

  if (info->VoiceVLAN[1] != "-")
    tempStr += info->VoiceVLAN[0] + "=" + info->VoiceVLAN[1] + "\n";

  if (info->VTP[1] != "-")
    tempStr += info->VTP[0] + "=" + info->VTP[1] + "\n";

  if (info->MgmtIP[1] != "-")
    tempStr += info->MgmtIP[0] + "=" + info->MgmtIP[1] + "\n";

  if (info->MgmtVLAN[1] != "-")
    tempStr += info->MgmtVLAN[0] + "=" + info->MgmtVLAN[1] + "\n";

  if (info->TTL[1] != "-")
    tempStr += info->TTL[0] + "=" + info->TTL[1] + "\n";

  if (info->Dup[1] != "-")
    tempStr += info->Dup[0] + "=" + info->Dup[1] + "\n";

  if (info->PoEAvail[1] != "-")
    tempStr += info->PoEAvail[0] + "=" + info->PoEAvail[1] + "\n";

  if (info->PoECons[1] != "-")
    tempStr += info->PoECons[0] + "=" + info->PoECons[1] + "\n";

  return tempStr;
}
#endif


//*********************************************
// WiFi functions
// Enable WiFi
void wifi_enable() {
#ifdef USE_SDCARD
  if (!wifi_MACloaded) {
    wifi_getDevNames();
  }
#endif

  wifi_isScanComplete = true;
#ifdef DEBUGSERIAL
  esp_err_t result;
  bool bresult;

  if (WiFi.isConnected()) {
    result = WiFi.disconnect();
    Serial.print("WiFi.disconnect()=");
    Serial.println(result);
  }

  bresult = WiFi.mode(WIFI_STA);
  Serial.print("WiFi.mode( WIFI_STA )=");
  if (bresult)
    Serial.println("OK");
  else
    Serial.println("not OK");
#else
  if (WiFi.isConnected())
    WiFi.disconnect();

  WiFi.mode(WIFI_STA);
#endif

  tft_displayData1[TFT_HEADERENTRY_WIFI].color = TFT_DARKGREEN;

  wifi_isEnabled = true;
  wifi_lastScanMillis = millis();
  if (wifi_lastScanMillis > WIFI_SCANPERIOD)
    wifi_lastScanMillis -= WIFI_SCANPERIOD;
  wifi_CountFound = 0;

  gen_currentFunction = fWiFi;
}  // void wifi_enable()

// Disable WiFi
void wifi_disable() {
#ifdef DEBUGSERIAL
  esp_err_t result;
  bool bresult;

  if (WiFi.isConnected()) {
    result = WiFi.disconnect();
    Serial.print("WiFi.disconnect()=");
    Serial.println(result);
  }

  bresult = WiFi.mode(WIFI_OFF);
  Serial.print("WiFi.mode( WIFI_OFF )=");
  if (bresult)
    Serial.println("OK");
  else
    Serial.println("not OK");
#else
  if (WiFi.isConnected())
    WiFi.disconnect();

  WiFi.mode(WIFI_OFF);
#endif

  tft_displayData1[TFT_HEADERENTRY_WIFI].color = TFT_BLACK;
  wifi_isEnabled = false;
  tft_showPage();
}  // void wifi_disable()

// Check if the WiFi scan has been finished or is still running
// and evaluate the  wifis found after the last scan.
int16_t wifi_checkScanState() {
  char *tmp;
  wifiClass *tmpWifi;
  wifiClass *newWifi;

  // Print out Wi-Fi network scan result upon completion
  int16_t n = WiFi.scanComplete();
  if ((n != WIFI_SCAN_RUNNING) && (n != WIFI_SCAN_FAILED)) {
    wifi_isScanComplete = true;

    // Set all known Wifis to not found
    wifi_setToNotFound();

    if (n > 0) {
      for (uint16_t i = 0; i < n; i++) {
        bool found = false;
        String bssidStr = String(WiFi.BSSIDstr(i));
        bssidStr.replace(":", "");
        bssidStr.toUpperCase();

        // Search for the current WiFi in the list of known WiFis
        for (uint16_t j = 0; j < wifiList.size(); j++) {
          tmpWifi = wifiList.get(j);
          if (strcmp(tmpWifi->mac, bssidStr.c_str()) == 0) {
            // WiFi found, so update current entry
            // - MAC address does not be to updated
            // - SSID should not be needed to update
            found = true;
            tmpWifi->strength = WiFi.RSSI(i);
            tmpWifi->encryption = WiFi.encryptionType(i);
            tmpWifi->found = true;
            tmpWifi->lastTimeFound = gen_currentMillis;
            if (tmpWifi->SSID == NULL) {
              // Create entry for SSID
              // $$$ to do: search in list of all known Wifis first
              tmp = (char *)malloc(sizeof(char) * WiFi.SSID(i).length() + 1);
              if (tmp == NULL) {
#ifdef DEBUGSERIAL
                Serial.printf("Failed to allocate %d of memory.\n", sizeof(char) * WiFi.SSID(i).length() + 1);
#endif
                break;
              }
              strcpy(tmp, WiFi.SSID(i).c_str());
              tmpWifi->SSID = (const char *)tmp;
            }
            break;
          }  // if (strcmp(tmpWifi->mac, bssidStr.c_str()) == 0)
        }    // for( uint16_t j = 0; j < wifiList.size(); j++ )

        // All known Wifis have been checked. If the current WiFi was not found
        // create and add a new entry.
        if (!found) {
          // Create new entry
          newWifi = new wifiClass();
          if (newWifi != NULL) {
            tmp = (char *)malloc(sizeof(char) * bssidStr.length() + 1);
            if (tmp == NULL) {
#ifdef DEBUGSERIAL
              Serial.printf("Failed to allocate %d of memory.\n", sizeof(char) * bssidStr.length() + 1);
#endif
              break;
            }
            strcpy(tmp, bssidStr.c_str());
            newWifi->mac = (const char *)tmp;
            newWifi->deviceName = NULL;  // Since the MAC address has not been found there is no device name

            // Create entry for SSID
            // $$$ search in list of all known Wifis first
            tmp = (char *)malloc(sizeof(char) * WiFi.SSID(i).length() + 1);
            if (tmp == NULL) {
#ifdef DEBUGSERIAL
              Serial.printf("Failed to allocate %d of memory.\n", sizeof(char) * WiFi.SSID(i).length() + 1);
#endif
              break;
            }
            strcpy(tmp, WiFi.SSID(i).c_str());
            newWifi->SSID = (const char *)tmp;

            newWifi->strength = WiFi.RSSI(i);
            newWifi->encryption = WiFi.encryptionType(i);
            newWifi->found = true;  // WiFi has just yet been found
            newWifi->lastTimeFound = gen_currentMillis;
            newWifi->written = false;  // WiFi newly discovered
            newWifi->ignore = false;   // WiFi newly discovered so do not ignore

            wifiList.add(newWifi);
          }  // if (newWifi != NULL)
        }    // if (!found)
      }      // for (uint16_t i = 0; i < n; i++)
    }        // if( n > 0 )

    WiFi.scanDelete();

    // Count WiFis found
    for (uint16_t i = 0; i < wifiList.size(); i++) {
      tmpWifi = wifiList.get(i);
      if (tmpWifi->found)
        wifi_CountFound++;
    }

#ifdef DEBUGSERIALx
    for (uint16_t i = 0; i < wifiList.size(); i++) {
      tmpWifi = wifiList.get(i);
      if (tmpWifi->found) {
        Serial.printf("%d. MAC: %s\n", i, tmpWifi->mac);
        if (tmpWifi->SSID != NULL)
          Serial.printf("%d. SSID: %s\n", i, tmpWifi->SSID);
        else
          Serial.printf("%d. SSID: unknown\n", i);
        if (tmpWifi->deviceName != NULL)
          Serial.printf("%d. Devicename: %s\n", i, tmpWifi->deviceName);
        Serial.printf("%d. Encryption: %d\n", i, tmpWifi->encryption);
        Serial.printf("%d. Encryption: %s\n", i, WIFI_AUTHENTICATIONMODE[tmpWifi->encryption]);
        Serial.printf("%d. Found: %d\n", i, tmpWifi->found);
        Serial.printf("%d. Found last time: %d\n", i, tmpWiFi->lastTimeFound);
        Serial.printf("%d. Ignore: %d\n", i, tmpWifi->ignore);
      }
    }
#endif

#ifdef USE_SDCARD
    wifi_updateCSV();
#endif
  }

  return n;
}  // int16_t wifi_checkScanState()

// Set all known Wifis to not found using the lowest value for signal strength.
// Set the found variable to false after the predefined duration.
void wifi_setToNotFound() {
  unsigned long cm = gen_currentMillis;
  wifiClass *tmpWiFi;
  for (uint16_t j = 0; j < wifiList.size(); j++) {
    tmpWiFi = wifiList.get(j);
    tmpWiFi->strength = INT16_MIN;
    // Check when the WiFi was found the last time. If it is out of range
    // for a longer time, set it to not found.
    if ((tmpWiFi->found) && ((cm - wifiClass::WIFINOTFOUNDAFTER) > tmpWiFi->lastTimeFound)) {
#ifdef DEBUGSERIAL
      Serial.printf("wifi_setToNotFound(): %s\n", tmpWiFi->mac);
#endif
      tmpWiFi->found = false;
      tmpWiFi->lastTimeFound = 0;
    }
  }
  wifi_CountFound = 0;
} // void wifi_setToNotFound()

// Get the next WiFi to be displayed.
// Return true if a WiFi has been found for displaing
// Return false if no WiFi has been found or the end of the WiFi list has been reached
bool wifi_getNextWifiToDisplay() {
  wifiClass *tmpWiFi;
  int16_t wiFiListSize = wifiList.size();

  // Are there entries in the WiFi list?
  if (wiFiListSize == 0) {
    // No, so nothing to display
    wifi_Current = -1;
    return false;
  }

  // Check if we start without any information about a WiFi to display
  if (wifi_Current >= wiFiListSize)
    wifi_Current = -1;

  bool found = false;
  do {
    wifi_Current++;
    if (wifi_Current >= wiFiListSize) {
#ifdef DEBUGSERIALx
      Serial.printf("wifi_getNextWifiToDisplay(): do-loop: wifi_Current >= wiFiListSize\n");
#endif
      break;
    }

    tmpWiFi = wifiList.get(wifi_Current);
    if (!tmpWiFi->ignore) {
      // WiFi should not be ignored
      if (tmpWiFi->found) {
        // WiFi was found during scan
        if (wifi_scanForSSID != NULL) {
#ifdef DEBUGSERIAL
          Serial.printf("wifi_getNextWifiToDisplay(): do-loop: wifi_scanForSSID != NULL\n");
#endif
          if (strcasecmp(wifi_scanForSSID, tmpWiFi->SSID) == 0) {
#ifdef DEBUGSERIALx
            Serial.printf("wifi_getNextWifiToDisplay(): do-loop: strcasecmp==0\n");
#endif
            found = true;
          }
        } else {
          found = true;
        }
      } // if (tmpWiFi->found)
    } // if (!tmpWiFi->ignore)
  } while (!found);

  if (wifi_Current >= wiFiListSize) {
    // No more WiFi found for displaying in this loop
    wifi_Current = -1;
    return false;
  } // if (wifi_Current >= wiFiListSize)

  // WiFi found so return true
  return true;
} // bool wifi_getNextWifiToDisplay()

// Set the text color for displaying the WiFi strength value
// depending on the RSSI
// > -30:      green, very good
// -31 to -50: yellow green, good
// -51 to -70: yellow, usable
// -71 to -90: orange, not good
// < -90:      red, unusable
void wifi_setTextColorForStrength(int16_t _strength) {
  if (_strength > -30)
    tft.setTextColor(TFT_GREEN);
  else if (_strength > -50)
    tft.setTextColor(TFT_GREENYELLOW);
  else if (_strength > -70)
    tft.setTextColor(TFT_YELLOW);
  else if (_strength > -90)
    tft.setTextColor(TFT_ORANGE);
  else
    tft.setTextColor(TFT_RED);
} // void wifi_setTextColorForStrength(int16_t _strength)

#ifdef USE_SDCARD
// Get device name for known WiFi mac addresses from file
// File structure:
// MAC(xxxxxxxxxxxx);DeviceName[;SSID;Encryption type]
// The SSID and encryption type will be added automatically if new WiFi Access Pointa are found
// but can be removed or edited later on a computer. For evaluating a WiFi device only the MAC
// address (which is usually offered by the Basic Service Set Identification (BSSID)) is checked.
// Some systems allow to offere different WiFi SSIDs and they use different virtual MAC addresses.
// I found this on Cisco APs where the base MAC address are the first five octects and the upper
// four bits of the sixth octet. To simplyfy the CSV file a '?' might be used as a wildcard on
// any location in the MAC address. Example: an AP offeres SSID "WiFi1" and SSID "WiFi2" and use
// the MAC address "CA:FE:C0:FF:EE:00" for "Wifi1" and  "CA:FE:C0:FF:EE:01" for "WiFi2" the CSV
// will contain this two lines after DAMPF has found them:
// CAFEC0FFEE01;;Wifi1;WPA2
// CAFEC0FFEE02;;Wifi2;WPA2
// On a computer the network device name for this AP can be added into the second column:
// CAFEC0FFEE01;APName;Wifi1;WPA2
// CAFEC0FFEE02;APName;Wifi2;WPA2
// These two lines might be combined to
// CAFEC0FFEE0?;APName;Wifi1;WPA2
// since the MAC address using this '?' wildcard matches both for "WiFi1" and "WiFi2". Which makes
// sense because it is the same physical device.
// Only if a device group can be configured like
// CAFEC0FFEE01;AP1Name;Wifi1;WPA2
// CAFEC0FFEE02;AP1Name;Wifi2;WPA2
// CAFEC0FFEE03;AP2Name;Wifi1;WPA2
// CAFEC0FFEE04;AP2Name;Wifi2;WPA2
// then the wildcard will not work because "CAFEC0FFEE0?" would also matches for all four WiFis and
// both network devices.
// Maybe some kind of regex expresssion will be addded later.
void wifi_getDevNames() {
  String csvLine;
  String tmpBSSID;
  String tmpDevName;
  String tmpSSID;
  String tmpIgnore;
  bool ignore;
  char *tmp;
  uint16_t lineCount = 0;
  unsigned char cFileHeader[5];  // Buffer for file header for Unicode BOM check

  // Check if the list has already been loaded
  if (wifi_MACloaded) {
#ifdef DEBUGSERIAL
    Serial.println("WiFi MAC list has already been loaded.");
#endif
    return;
  }

#ifdef DEBUGSERIALx
  Serial.println("void wifi_getDevNames()");
  unsigned long ulFreeHeap = ESP.getFreeHeap();
  Serial.println("Free heap: " + String(ulFreeHeap));
#endif

  // Position of semicolon separator
  int16_t startPos = 0;
  int16_t endPos = 0;

  if (!sd_available) {
#ifdef DEBUGSERIAL
    Serial.println("wifi_getDevNames(): No SD card available.");
#endif
    return;
  }

  // Take the mutex for writing to SD card
  if (xSemaphoreTake(xMutex_sd_card, pdMS_TO_TICKS(SD_SEMA_WAIT)) == pdTRUE) {
    file = SD.open(SD_WIFIRESFILENAME, FILE_READ);
    if (!file) {
#ifdef DEBUGSERIAL
      Serial.println("wifi_getDevNames(): Failed to open file for reading.");
#endif
      xSemaphoreGive(xMutex_sd_card);
      return;
    }

    if (!SD.exists(SD_WIFIRESFILENAME)) {
#ifdef DEBUGSERIAL
      Serial.println("wifi_getDevNames(): File does not exist.");
#endif
      xSemaphoreGive(xMutex_sd_card);
      return;
    }

    if (file.size() > 0) {
      size_t bytesRead = file.read(cFileHeader, 4);
      if (bytesRead > 0) {
        bool isUnicode = sd_checkUnicode(cFileHeader);
        if (isUnicode) {
#ifdef DEBUGSERIAL
          Serial.println("Warning: wifimac.csv is a unicode file!");
#endif
          xSemaphoreGive(xMutex_sd_card);
          return;
        }  // if(isUnicode)
        else {
#ifdef DEBUGSERIAL
          Serial.println("wifimac.csv is ASCII.");
#endif
          file.seek(0);  // Rewind
        }
      }  // if (bytesRead > 0)
    }    // if (file.size() > 0)

    // Read all lines
    while (file.available()) {
      lineCount++;
      csvLine = file.readStringUntil('\n');
      csvLine.trim();
      if (csvLine != "") {
        // Check for header line
        // "MAC;DeviceName;SSID;Ignore"
        if (lineCount == 1) {
#ifdef DEBUGSERIAL
          Serial.println("First line loaded.");
#endif
          if (strcmp(TXT_SD_HEADERLINE, csvLine.c_str()) == 0) {
#ifdef DEBUGSERIAL
            Serial.println("Header line found.");
#endif
            continue;
          } else {
#ifdef DEBUGSERIAL
            Serial.println("Header line not found.");
#endif
          }
        } // if (lineCount == 1)

        // Read MAC address, first field in CSV
        startPos = 0;
        endPos = csvLine.indexOf(';', 0);
        if (endPos == -1)
          continue;
        tmpBSSID = csvLine.substring(startPos, endPos);  // MAC address
        tmpBSSID.replace(":", "");
        tmpBSSID.toUpperCase();

        // Read device name, second field in CSV
        if ((endPos > 0) && (endPos < 32767)) {
          startPos = endPos + 1;
          endPos = csvLine.indexOf(';', startPos);
          if (endPos == -1)
            continue;
          tmpDevName = csvLine.substring(startPos, endPos);  // Device name
        } else
          continue;

        // Read SSID, third field in CSV
        if ((endPos > 0) && (endPos < 32767)) {
          startPos = endPos + 1;
          endPos = csvLine.indexOf(';', startPos);
          if (endPos == -1)
            continue;
          tmpSSID = csvLine.substring(startPos, endPos);  // SSID
        } else
          continue;

        // Read ignore entry, fourth field in CSV
        if ((endPos > 0) && (endPos < 32767)) {
          startPos = endPos + 1;
          endPos = csvLine.indexOf(';', startPos);
          if (endPos == -1)
            tmpIgnore = csvLine.substring(startPos);
          else
            tmpIgnore = csvLine.substring(startPos, endPos);

          if (tmpIgnore == "1")
            ignore = true;
          else
            ignore = false;
        } else
          continue;

        // Create new entry
        wifiClass *newWifi = new wifiClass();
        if (newWifi != NULL) {
          // Store MAC address
          tmp = (char *)malloc(sizeof(char) * tmpBSSID.length() + 1);
          if (tmp == NULL) {
#ifdef DEBUGSERIAL
            Serial.printf("Failed to allocate %d of memory.\n", sizeof(char) * tmpBSSID.length() + 1);
#endif
            break;
          }  // if( tmp == NULL )
          strcpy(tmp, tmpBSSID.c_str());
          newWifi->mac = (const char *)tmp;

          // Check device name and if the WiFi should not be ignored
          if (tmpDevName != "" && !ignore) {
            // A device name is available so check if this name has been use
            // with an other WiFi yet. Check all loaded WiFi entries.
            bool found = false;
            for (uint16_t i = 0; i < wifiList.size(); i++) {
              wifiClass *tmpWifi = wifiList.get(i);
              if (tmpWifi->deviceName != NULL) {
                // Device name was set. Compare with current device name
                if (strcmp(tmpWifi->deviceName, tmpDevName.c_str()) == 0) {
                  // Device name was used before so copy pointer
                  found = true;
                  newWifi->deviceName = tmpWifi->deviceName;
                  break;
                }
              }
            }  // for( uint16_t i = 0; i < wifiList.size(); i++ )

            if (!found) {
              // Device name wasn't used yet so store it
              tmp = (char *)malloc(sizeof(char) * tmpDevName.length() + 1);
              if (tmp == NULL) {
#ifdef DEBUGSERIAL
                Serial.printf("Failed to allocate %d of memory.\n", sizeof(char) * tmpBSSID.length() + 1);
#endif
                break;
              }
              strcpy(tmp, tmpDevName.c_str());
              newWifi->deviceName = (const char *)tmp;
            }
          }  // if( tmpDevName != "" )
          else
            newWifi->deviceName = NULL;

          // Check SSID
          if (tmpSSID == "")
            continue;
          else {
            // A SSID is available so check if this name has been use
            // with an other WiFi yet. Check all loaded WiFi entries.
            bool found = false;
            for (uint16_t i = 0; i < wifiList.size(); i++) {
              wifiClass *tmpWifi = wifiList.get(i);
              if (tmpWifi->SSID != NULL) {
                // SSID was set. Compare with current SSID.
                if (strcmp(tmpWifi->SSID, tmpSSID.c_str()) == 0) {
                  // SSID was used before so copy pointer
                  found = true;
                  newWifi->SSID = tmpWifi->SSID;
                  break;
                }
              }
            }

            if (!found) {
              // SSID wasn't used yet so store it
              tmp = (char *)malloc(sizeof(char) * tmpSSID.length() + 1);
              if (tmp == NULL) {
#ifdef DEBUGSERIAL
                Serial.printf("Failed to allocate %d of memory.\n", sizeof(char) * tmpBSSID.length() + 1);
#endif
                break;
              }
              strcpy(tmp, tmpSSID.c_str());
              newWifi->SSID = (const char *)tmp;
            }
          }

          newWifi->strength = INT16_MIN;  // Strength is always negative, usually in a range between -120 to 0
          newWifi->encryption = WIFI_AUTH_OPEN;
          newWifi->found = false;
          newWifi->written = true;  // WiFi not newly discovered, it has been read from CSV
          newWifi->ignore = ignore;

          wifiList.add(newWifi);
        }  // if( newWifi != NULL )
      }    // if( csvLine != "" )
    }      // while( file.available() )
    file.close();
    xSemaphoreGive(xMutex_sd_card);
  }

  wifi_MACloaded = true;
}  // void wifi_getDevNames()
#endif

#ifdef USE_SDCARD
// Update CSV file for all found WiFi sources
// File structure:
// MAC;DeviceName;SSID;Encryption type
void wifi_updateCSV() {
  bool writeCSVHeader = false;
  wifiClass *tmpWifi;

  if (!sd_available) {
#ifdef DEBUGSERIAL
    Serial.println("No SD card available.");
#endif
    return;
  } // if (!sd_available)

#ifdef DEBUGSERIAL
  Serial.printf("Update file: %s\n", SD_WIFIRESFILENAME);
#endif

  // Take the mutex for writing to SD card
  if (xSemaphoreTake(xMutex_sd_card, pdMS_TO_TICKS(SD_SEMA_WAIT)) == pdTRUE) {
    // Check if files exists. If not the header line should be written
    if (!SD.exists(SD_WIFIRESFILENAME))
      writeCSVHeader = true;

    file = SD.open(SD_WIFIRESFILENAME, FILE_APPEND);
    if (!file) {
#ifdef DEBUGSERIAL
      Serial.println("Failed to open file for appending");
#endif
      xSemaphoreGive(xMutex_sd_card);
      return;
    } // if (!file)

    // If the header should be written
    if (writeCSVHeader)
      file.printf("%s\n", TXT_SD_HEADERLINE);  // "MAC;DeviceName;SSID;Ignore"

#ifdef DEBUGSERIAL
    Serial.println("Updating CSV with the following entries:");
#endif

    for (uint16_t i = 0; i < wifiList.size(); i++) {
      tmpWifi = wifiList.get(i);
      if (!tmpWifi->written) {
        file.printf("%s;", tmpWifi->mac);
        if (tmpWifi->deviceName != NULL)
          file.printf("%s", tmpWifi->deviceName);
        file.printf(";%s;%d\n", tmpWifi->SSID, tmpWifi->ignore);
        tmpWifi->written = true;

#ifdef DEBUGSERIAL
        Serial.printf("%d. SSID: %s\n", i, tmpWifi->SSID);
        Serial.printf("%d. MAC: %s\n", i, tmpWifi->mac);
        if (tmpWifi->deviceName != NULL)
          Serial.printf("%d. Devicename: %s\n", i, tmpWifi->deviceName);
        Serial.printf("%d. Encryption: %d\n", i, tmpWifi->encryption);
        Serial.printf("%d. Ignore: %d\n", i, tmpWifi->ignore);
#endif
      } // if (!tmpWifi->written)
    } // for (uint16_t i = 0; i < wifiList.size(); i++)

    file.close();
    xSemaphoreGive(xMutex_sd_card);
  } // if (xSemaphoreTake(xMutex_sd_card, pdMS_TO_TICKS(SD_SEMA_WAIT)) == pdTRUE)
} // void wifi_updateCSV()
#endif

//*********************************************
// Bluetooth functions
#ifdef USE_BTSERIAL
// Setup Bluetooth options
void bt_initialize() {
  esp_err_t returncode;

  bt_isConnected = false;
  bt_isConnectRequest = false;
  bt_isConnectRequestPageShow = false;
  bt_connectRequestStarttime = 0ul;

  // Configure Bluetooth power
  //returncode = esp_ble_tx_power_set( ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9 );
  returncode = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N0);
  if (returncode != ESP_OK) {
    tft_userMenu[TFT_MENUENTRY_BTSERIAL].isActive = false;
    tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = false;
    return;
  }

  // Enable "Simple Secure Pairing"
  SerialBT.enableSSP();

  // Register Bluetooth authentication functions
  SerialBT.onConfirmRequest(bt_confirmRequestFunction);
  SerialBT.onAuthComplete(bt_authCompleteFunction);

  // Register Bluetooth callback function
  SerialBT.register_callback(bt_callBackFunction);

  tft_displayData1[TFT_HEADERENTRY_BT].color = TFT_BLACK;
  tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = true;
}  // void initializeBluetooth()

// Enable Bluetooth
void bt_enable() {
  bt_isConnected = false;
  bt_isConnectRequest = false;
  bt_isConnectRequestPageShow = false;
  bt_connectRequestStarttime = 0ul;

  // Initialize Bluetooth connection
  bt_isInitialized = SerialBT.begin(gen_DeviceName, false);
  if (bt_isInitialized) {
    tft_displayData1[TFT_HEADERENTRY_BT].color = TFT_BLUE;
    tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_BLACK;
    tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = true;  // Enable logging to SD selection
#ifdef DEBUGSERIAL
    Serial.println("bt_enable(): BT TFT_BLUE");
    Serial.println("bt_enable(): SERA TFT_BLACK");
    Serial.println("bt_enable(): tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = true");
#endif
#ifdef DEBUGSERIAL
    Serial.println("bt_enable(): Bluetooth initialized");
#endif
  } else {
    tft_displayData1[TFT_HEADERENTRY_BT].color = TFT_BLACK;
    tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_BLACK;
    tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = false;  // Disable logging to SD selection
#ifdef DEBUGSERIAL
    Serial.println("bt_enable(): BT TFT_BLACK");
    Serial.println("bt_enable(): SERA TFT_BLACK");
    Serial.println("bt_enable(): tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = false");
#endif
    btStop();
#ifdef DEBUGSERIAL
    Serial.println("bt_enable(): Bluetooth initialization failed");
#endif
  }
} // void bt_enable()

// Disable Bluetooth
void bt_disable() {
  if (bt_isConnected) {
    if (SerialBT.hasClient()) {
#ifdef DEBUGSERIAL
      Serial.println("bt_disable(): Disconnecting Bluetooth");
#endif
      SerialBT.disconnect();
    }
  }

  tft_displayData1[TFT_HEADERENTRY_BT].color = TFT_BLACK;
  tft_displayData1[TFT_HEADERENTRY_SERA].color = TFT_BLACK;
  tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = false;  // Disable logging to SD selection
#ifdef DEBUGSERIAL
  Serial.println("bt_disable(): BT TFT_BLACK");
  Serial.println("bt_disable(): SERA TFT_BLACK");
  Serial.println("bt_disable(): tft_userMenu[TFT_MENUENTRY_BTSERIALLOGSD].isActive = false");
#endif
  btStop();

  bt_isInitialized = false;
  bt_isConnected = false;
  bt_isConnectRequest = false;
  bt_isConnectRequestPageShow = false;
  bt_connectRequestStarttime = 0ul;
} // void bt_disable()

// Bluetooth connection request screen
// Print request and received Pin for accepting or denying
void bt_connectScreen() {
#ifdef DEBUGSERIAL
  Serial.println("void bt_connectScreen()");
#endif

  uint8_t row = 0;
  tft.setTextColor(TFT_ORANGE);

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_BT_CONNECTION);
  row++;

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row);

  uint8_t currentTextSize = tft.textsize;
  tft.setTextSize(currentTextSize * 2);
  tft.print(String(bt_RequestedPin));
  tft.setTextSize(currentTextSize);
  row += 3;

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_BT_CONNACCEPT);

  tft.setCursor(0, tft_userY + (tft_fontHeight * TFT_SIZESCALER) * row++);
  tft.print(TXT_BT_CONNDECLINE);
}  // void bt_connectScreen()

// Bluetooth callback function
void bt_callBackFunction(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
#ifdef DEBUGSERIALx
  Serial.println("void bt_callBackFunction( esp_spp_cb_event_t event, esp_spp_cb_param_t *param )");
  switch (event) {
    case ESP_SPP_INIT_EVT:
      Serial.println("ESP_SPP_INIT_EVT");
      break;

    case ESP_SPP_SRV_OPEN_EVT:
      // Server connection open
      Serial.println("ESP_SPP_SRV_OPEN_EVT");
      break;

    case ESP_SPP_CLOSE_EVT:
      // Client connection closed
      Serial.println("ESP_SPP_CLOSE_EVT");
      break;

    case ESP_SPP_CONG_EVT:
      // Connection congestion status changed
      Serial.println("ESP_SPP_CONG_EVT");
      break;

    case ESP_SPP_WRITE_EVT:
      // Write operation completed
      Serial.println("ESP_SPP_WRITE_EVT");
      break;

    case ESP_SPP_DATA_IND_EVT:
      // Connection received data
      Serial.println("ESP_SPP_DATA_IND_EVT");
      break;

    case ESP_SPP_DISCOVERY_COMP_EVT:
      // Discovery complete
      Serial.println("ESP_SPP_DISCOVERY_COMP_EVT");
      break;

    case ESP_SPP_OPEN_EVT:
      // Client connection open
      Serial.println("ESP_SPP_OPEN_EVT");
      break;

    case ESP_SPP_START_EVT:
      // Server started
      Serial.println("ESP_SPP_START_EVT");
      break;

    case ESP_SPP_CL_INIT_EVT:
      // Client initiated a connection
      Serial.println("ESP_SPP_CL_INIT_EVT");
      break;

    default:
      Serial.println("unknown event!");
      Serial.println(String(event));
      break;
  }  // switch( event )
#endif

  // function implementation
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    bt_status = 1;
    bt_isConnected = true;
#ifdef DEBUGSERIAL
    Serial.println("Bluetooth connected.");
    for (byte i = 0; i < 6; i++) {
      Serial.printf("%02X", param->srv_open.rem_bda[i]);
      if (i < 5) {
        Serial.print(":");
      }
    }
    Serial.println();
#endif
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    bt_status = 0;
    bt_isConnected = false;
#ifdef DEBUGSERIAL
    Serial.println("Bluetooth disconnected.");
#endif
  }
}  // void bt_callBackFunction(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)

// Bluetooth connection confirm request
void bt_confirmRequestFunction(uint32_t btpin) {
#ifdef DEBUGSERIAL
  Serial.println("void bt_confirmRequestFunction( uint32_t btpin )");
#endif
  bt_isConnectRequest = true;
  bt_connectRequestStarttime = millis();  // Bluetooth connection started at
  bt_isConnectRequestPageShow = true;
  bt_RequestedPin = btpin;
}  // void bt_confirmRequestFunction(

// Bluetooth authentication completed
void bt_authCompleteFunction(boolean success) {
  bt_isConnectRequest = false;

  if (success) {
    bt_isConnected = true;
#ifdef DEBUGSERIAL
    Serial.println("Pairing success!");
#endif
  } else {
    bt_isConnected = false;
#ifdef DEBUGSERIAL
    Serial.println("Pairing failed, rejected by user!");
#endif
  }

  bt_RequestedPin = 0;
  disp_autoSwitchLastTime = 0l;
}  // void bt_authCompleteFunction(boolean success)
#endif


// *************************************************************************
// External real time clock functions
#ifdef USE_RTCTIME
// Initialize real time clock
void rtc_initialize() {
  if (!rtc.begin()) {
    tft_displayData1[TFT_HEADERENTRY_RTC].color = TFT_SILVER;
#ifdef DEBUGSERIAL
    Serial.println("Couldn't find RTC DS3231.");
#endif
  }  // if (!rtc.begin())
  else {
    ertc_present = true;
    DateTime now = rtc.now();
    if (now.year() > 2000) {
      esprtc.setTime(now.unixtime());
      tft_displayData1[TFT_HEADERENTRY_RTC].color = TFT_DARKGREEN;
#ifdef DEBUGSERIAL
      Serial.println("RTC DS3231 found:");
      char curDat[30];  // "HH:MM yyyy-mm-dd0"
      sprintf(curDat, "%02u:%02u %04u-%02u-%02u\n", now.hour(), now.minute(), now.year(), now.month(), now.day());
      Serial.println(curDat);
#endif
    }  // if (now.year() > 2000)
    else {
      tft_displayData1[TFT_HEADERENTRY_RTC].color = TFT_BLACK;
    }  // else
  }    // else
}  // void rtc_initialize()
#endif


// *************************************************************************
// General functions
void gen_switchFunction(eFunction newFunction) {
  if ((gen_currentFunction == fNone) && (newFunction == fNone))
    return;  // Nothing to do

  // Disable the current function
  switch (gen_currentFunction) {
    case fEthernet:
      eth_stop();
      break;
    case fWiFi:
      wifi_disable();
      break;
    case fBluetoothSerial:
      bt_disable();
      break;
    case fSerialLogger:
      ser_disableLogger();
      break;
    case fNone:
    default:
      break;
  }

  if (newFunction != gen_currentFunction) {
    switch (newFunction) {
      case fEthernet:
        eth_restart();
        break;
      case fWiFi:
        wifi_enable();
        break;
      case fBluetoothSerial:
        bt_enable();
        break;
      case fSerialLogger:
        ser_enableLogger();  // Enable logging mode selection
        tft_userMenu[TFT_MENUENTRY_SERIALLOGGINGMODE].isActive = true;
        break;
      case fNone:
      default:
        break;
    }

    // Set the current function to the new function
    gen_currentFunction = newFunction;
  } else {
    gen_currentFunction = fNone;
  }
}

// Send ESP32 to hibernation
void gen_hibernate() {
#ifdef DEBUGSERIAL
  Serial.println("Battery power too low, powering down.");
#endif

  // Stop current function
  gen_switchFunction(fNone);

  // Clear screen. This should have no effect for power saving
  tft.fillRect(0, 0, tft_width - 1, tft_height - 1, TFT_BLACK);

#if defined (TFT_BL) && defined (TFT_BACKLIGHT_ON)
  // Turn off the back-light LED
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, !TFT_BACKLIGHT_ON);
  // Set the pin to hold the low value in deep sleep
  gpio_deep_sleep_hold_en();
  gpio_hold_en((gpio_num_t)TFT_BL);
#endif

#ifdef TFT_DISPOFF
  // Turn display off
  tft.writecommand(TFT_DISPOFF);
  delay(120);
#endif

#ifdef TFT_SLPIN
  // Put display controller to sleep
  tft.writecommand(TFT_SLPIN);
#endif

  // Sleep loop, never wake up
  while (1) {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
    delay(1000);
  }
} // void gen_hibernate()
