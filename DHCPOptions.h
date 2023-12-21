/*
DHCPOptions.h

Description used from several internet pages, i.e.
https://www.iana.org/assignments/bootp-dhcp-parameters/bootp-dhcp-parameters.xhtml
https://help.sonicwall.com/help/sw/eng/6800/26/2/3/content/Network_DHCP_Server.042.12.htm
 
2023-12-18: Initial version
*/

#include <EtherCard.h>
#include <Arduino.h>
#include "Packet_data.h"

#ifndef DHCPOPTIONS_H
#define DHCPOPTIONS_H

struct DHCP_DATA {
  // Option[ 0 ] will be used for regular DHCP information
  // Option[ 1 ] will be used for DHCP information recevied from a tagged VLAN
  String Option[2] = { "-", "-" };
};

void DHCPOption(uint8_t option, const byte* data, uint8_t len);
void IPv4(uint8_t option, String optlabel, const byte* data, uint8_t len);
void IPv4NTP(const byte* data, uint8_t len);
void DHCP_Text(uint8_t option, String optlabel, const byte* data, uint8_t len);
void DHCP_Time(uint8_t option, String optlabel, const byte* data, uint8_t len);
void DHCP_Search(uint8_t option, String optlabel, const byte* data, uint8_t len);
void DHCP_NumField(uint8_t option, String optlabel, const byte* data, uint8_t len);
#endif
