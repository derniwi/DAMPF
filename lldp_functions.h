/* 
lldp_functions.h

Evaluate received LLDP packages
Some details added as description / comments from WireShark
IEEE 802.1AB:
https://www.ieee802.org/3/frame_study/0409/blatherwick_1_0409.pdf

2023-12-18: Initial version
*/

#include <EtherCard.h>
#include <Arduino.h>
#include "Packet_data.h"

#ifndef LLDP_FUNCTIONS_H
#define LLDP_FUNCTIONS_H

unsigned int lldp_check_Packet(byte EthBuffer[], unsigned int length);
PINFO lldp_packet_handler(byte cdpData[], uint16_t plen);
String handleLLDPIPField(const byte a[], unsigned int offset, unsigned int lengtha);
String handlelldpAsciiField(byte a[], unsigned int offset, unsigned int lengtha);
void handlelldpCapabilities(const byte a[], unsigned int offset, unsigned int lengtha);
String lldp_print_binary(int v, int num_places);
String LldpCapabilities(String temp);
String lldp_print_mac(const byte a[], unsigned int offset, unsigned int length);
bool lldp_byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length);
String lldp_handleCdpNumField(const byte a[], unsigned int offset, unsigned int length);

String handleChassisSubtype(byte cdpData[], unsigned int cdpDataIndex, unsigned int lldpFieldLength);
String handlePortSubtype(byte cdpData[], unsigned int cdpDataIndex, unsigned int lldpFieldLength);
String handleManagementSubtype(byte cdpData[], unsigned int cdpDataIndex, unsigned int lldpFieldLength);
String handleAddressType(byte addressType);

void send_LLDP_MED(uint16_t buffersize, uint16_t voiceVLAN, unsigned long* lastLLDPsent, byte mymac[]);

#endif
