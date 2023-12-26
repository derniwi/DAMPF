/*
DHCPOptions.cpp

Description used from several internet pages, i.e.
https://www.iana.org/assignments/bootp-dhcp-parameters/bootp-dhcp-parameters.xhtml
https://help.sonicwall.com/help/sw/eng/6800/26/2/3/content/Network_DHCP_Server.042.12.htm
 
2023-12-18: Initial version
*/

#include "Definitions.h"
#include <Arduino.h>
#include "DHCPOptions.h"

// Information about the DHCP options. The option 0 is reserved for padding and not used by DHCP
// itself. So this field is used in the project for the provided IP address.
// The eth_dhcpInfo is a two dimensional array:
// eth_dhcpInfo[0]: normal DHCP information
// eth_dhcpInfo[1]: DHCP information if a Voice VLAN has been found
extern DHCP_DATA eth_dhcpInfo[2][255];

// eth_vlanOption = 0: normal DHCP
// eth_vlanOption = 1: Voice VLAN DHCP
extern byte eth_vlanOption;

// How many IP addresses might be used for possible NTP servers?
// - option 42 (NTP) returns a list of IP addresses, I haven't found a maximum number
// - option 3 (Router Option) returns a list of IP addresses
// - option 119 (Domain Search) returns a list of domain names, but no IP addresses
//   https://www.rfc-editor.org/rfc/rfc3397
extern byte eth_ntpIPs[ETH_NTPMAXSOURCES][IP_LEN];
extern byte eth_ntpSources;

#define DHCP_INFINITE_LEASE 0xffffffff

void DHCPOption(uint8_t option, const byte* data, uint8_t len) {
  // likely to be an IPv4 address/Subnet
  switch (option) {
    case 1:
      // Subnet mask
      IPv4(option, "MASK", data, len);  // len must be 4
      break;

    case 2:
      // Time offset for DHCP option 4 - not used here
      // DHCP_NumField( option, "TMOF", data, len );
      //Serial.println("DHCP option 2: time offset");
      break;

    case 3:
      {
        // Router / default gateway address
        IPv4(option, "GW", data, len);  // len must be a multiple of 4
        IPv4NTP(data, len);             // Add router(s) to possible IP sources
        break;
      }

    case 4:
      // Time server - see RFC0868
      // currently not used here since this is a different protocol
      //#ifdef DEBUGSERIAL
      //      Serial.println("DHCP option 4: TIME");
      //#endif
      break;

    case 6:
      // Domain server
      IPv4(option, "DNS", data, len);  // len must be a multiple of 4
      break;

    case 15:
      // DNS domain name
      DHCP_Text(option, "DOM", data, len);
      break;

    case 28:
      // Broadcast (IPv4) address
      IPv4(option, "BC", data, len);
      break;

    case 42:
      {
        // NTP servers
        IPv4(option, "NTP", data, len);
        IPv4NTP(data, len);  // Add NTP server(s) to NTP sources
        break;
      }

    case 44:
      // NETBIOS name server
      IPv4(option, "NETB", data, len);
      break;

    case 51:
      // IP address lease time
      DHCP_Time(option, "LEASE", data, len);
      break;

    case 53:
      // DHCP message type
      // Value   Message type
      //     1   Discover
      //     2   Offer
      //     3   Request
      //     4   Decline
      //     5   P ACK (packet acknowledged)?
      //     6   P NAK (packet not acknowledged?)
      //     7   Release
      //     8   Inform
      //skip DHCP_Text( option, "DHCP Msg Type ", data, len );
      break;

    case 54:
      // DHCP server identification
      IPv4(option, "DHCP", data, len);
      break;

    case 58:
      // DHCP renewal (T1) time
      // DHCP_Time( option, "RENEW", data, len );
      break;

    case 59:
      // DHCP rebinding (T1) time
      // DHCP_Time( option, "REBIND", data, len );
      break;

    case 61:
      // Client identifier => 0x01 + Client MAC address?
      // Skip DHCP_Text( option, "CLIENTID", data, len );
      break;

    case 66:
      // TFTP server name
      // DHCP_Text( option, "TFTP", data, len );
      break;

    case 67:
      // TFTP boot file name
      // DHCP_Text( option, "PXEF", data, len );
      break;

    case 77:
      // User class information
      // Skip option
      break;

    case 119:
      // DNS domain search list
      DHCP_Search(option, "SRCDOM", data, len);
      break;

    case 158:
      // Unspecified OPTION_V4_PCP_SERVER
      // DHCP_Text( option, "OPTION_V4_PCP_SERVER", data, len );
      break;

    case 242:
      // Avaya configuration?
      DHCP_Text(option, "AVAYA", data, len);
      break;

    case 243:
      // Avaya configuration?
      DHCP_Text(option, "AVAYA", data, len);
      break;

    case 255:
      // End
      // Skip option
      break;

    default:
      {
#ifdef DEBUGSERIAL
        byte i;
        char temp[len + 1];
        int a = 0;
        for (i = 0; i < (len); ++i, ++a) {
          temp[a] = data[i];
        }
        temp[len] = '\0';
        String temp1 = temp;
        String hex;
        Serial.println("DHCP default OPT: " + String(option) + " LEN:" + String(len));
        for (i = 0; i < len; i++) {
          hex = "00" + String(temp[i], HEX);
          hex = "0x" + hex.substring(hex.length() - 2);
          Serial.print("0x" + String(temp[i], HEX) + " ");
        }
        Serial.println();
#endif
        break;
      }
  }
}

// Process IPv4 address(es)
// Some DHCP entries returns more than one address like DNS servers,
// others like the network mask should only return one address.
void IPv4(uint8_t option, String optlabel, const byte* data, uint8_t len) {
  byte address[IP_LEN];
  String temp;
  for (unsigned int i = 0; i < (len / IP_LEN); ++i) {
    for (unsigned int j = 0; j < IP_LEN; ++j) {
      address[j] = data[j + i * IP_LEN];
      temp += address[j];
      if (j < (IP_LEN - 1)) {
        temp += ".";
      }
    }
    if (i < ((len / IP_LEN) - 1))
      temp += "\n";
  }
  eth_dhcpInfo[eth_vlanOption][option].Option[0] = optlabel;
  eth_dhcpInfo[eth_vlanOption][option].Option[1] = temp;
}

// Process IPv4 address(es) which might be used as NTP time sources
// Add new addresses to eth_ntpIPs, check if address is already in list
void IPv4NTP(const byte* data, uint8_t len) {
  byte tmpIP[IP_LEN];
  bool found;

  for (byte i = 0; i < len; i += IP_LEN) {
    // Create temporary IP address field
    for (byte j = 0; j < IP_LEN; j++) {
      tmpIP[j] = data[i * IP_LEN + j];
#ifdef DEBUGSERIAL
      Serial.print(tmpIP[j]);
      if (j < (IP_LEN - 1)) Serial.print(".");
#endif
    }
#ifdef DEBUGSERIAL
    Serial.println();
#endif

    found = false;
    // Check if this IP address is already in the list
    for (byte j = 0; j < eth_ntpSources; j++) {
      int32_t res = memcmp(tmpIP, eth_ntpIPs[j], IP_LEN);
      if (res == 0) {
#ifdef DEBUGSERIAL
        Serial.println("IP address already in list");
#endif
        found = true;
        break;
      }
    }

    if (!found) {
      if (eth_ntpSources == ETH_NTPMAXSOURCES) {
#ifdef DEBUGSERIAL
        Serial.print("WARNING: current eth_ntpSources value=");
        Serial.println(String(eth_ntpSources));
        Serial.print("WARNING: IPs to be processed: ");
        byte toBeProc = (len - i * 4) / 4;
        Serial.println(String(toBeProc));
#endif
        return;
      }  // if( eth_ntpSources == ETH_NTPMAXSOURCES )
      memcpy(eth_ntpIPs[eth_ntpSources], tmpIP, IP_LEN);
      eth_ntpSources++;
#ifdef DEBUGSERIAL
      Serial.println("Address added, eth_ntpSources = " + String(eth_ntpSources));
#endif
    }  // if( !found)
  }    // for( byte i = 0; i < len; i += IP_LEN )
}

// Process text entries like the DNS domain
void DHCP_Text(uint8_t option, String optlabel, const byte* data, uint8_t len) {
  char temp[len + 1];
  int a = 0;
  String temp1;
  for (byte i = 0; i < (len); ++i, ++a) {
    temp[a] = data[i];
  }
  temp[len] = '\0';
  temp1 = temp;
  eth_dhcpInfo[eth_vlanOption][option].Option[0] = optlabel;
  eth_dhcpInfo[eth_vlanOption][option].Option[1] = temp1;
}

void DHCP_Search(uint8_t option, String optlabel, const byte* data, uint8_t len) {
  char temp[len];
  int a = 0;
  String temp1;

  unsigned int DataIndex = 0;
  while (DataIndex < len) {
    unsigned int Searchlen = data[DataIndex];
    if (Searchlen == 0) {
      temp1 += "\n";
      DataIndex++;
    } else {
      DataIndex++;
      for (int i = 0; i < Searchlen; ++i, ++a) {
        temp[a] = data[i + DataIndex];
      }
      temp[Searchlen] = '\0';
      temp1 += temp;
      if ((DataIndex + Searchlen) != len && data[DataIndex + Searchlen] != 0) {
        temp1 += ".";
      }
      DataIndex += Searchlen;
      a = 0;
    }
  }
  eth_dhcpInfo[eth_vlanOption][option].Option[0] = optlabel;
  eth_dhcpInfo[eth_vlanOption][option].Option[1] = temp1;
}

// Process time entries
void DHCP_Time(uint8_t option, String optlabel, const byte* data, uint8_t len) {
  unsigned long num = 0;
  int temp1;
  for (unsigned int i = 0; i < len; ++i) {
    num <<= 8;
    num += data[i];
  }
  temp1 = (int)num;
  temp1 = ((temp1 / 60) / 60);
  eth_dhcpInfo[eth_vlanOption][option].Option[0] = optlabel;
  eth_dhcpInfo[eth_vlanOption][option].Option[1] = String(temp1) + " hours";
}

void DHCP_NumField(uint8_t option, String optlabel, const byte* data, uint8_t len) {
  unsigned long num = 0;
  String temp;
  for (unsigned int i = 0; i < len; ++i) {
    num <<= 8;
    num += data[i];
  }
  temp = String(num, DEC);
  eth_dhcpInfo[eth_vlanOption][option].Option[0] = optlabel;
  eth_dhcpInfo[eth_vlanOption][option].Option[1] = String(temp);
}
