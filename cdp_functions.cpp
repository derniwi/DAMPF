/*
cdp_functions.cpp

Evaluate received CDP packages
Some details added as description / comments from WireShark

2023-12-18: Initial version
*/

#include "Definitions.h"
#include <Arduino.h>
#include "cdp_functions.h"
#include "Packet_data.h"

// CDP broadcast address
byte cdp_mac[] = { 0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc };

/*
#define printhex(n) \
  { \
    if ((n) < 0x10) { Serial.print('0'); } \
    Serial.print((n), HEX); \
  }
*/

PINFO cdpinfo;

unsigned int cdp_check_Packet(byte EthBuffer[], unsigned int length) {
  if (length > 0) {
    unsigned int cdpDataIndex = 0;

    // This PID is for CDP only and will filter out VTP, DTP, etc..
    String PID = print_mac(EthBuffer, 20, 2);
    if (PID != "2000") {
      return (0);
    } else {
      if (byte_array_contains(EthBuffer, cdpDataIndex, cdp_mac, sizeof(cdp_mac))) {
        // CDP Packet found and is now getting processed
#ifdef DEBUGSERIAL
        Serial.println("\n\nCDP Packet Recieved");
#endif

        cdpDataIndex += sizeof(cdp_mac);  // Increment index the length of the source MAC address

        // Get source MAC Address
        byte* macFrom = EthBuffer + cdpDataIndex;
        cdpinfo.MAC[1] = print_mac(macFrom, 0, 6);

        cdpDataIndex += sizeof(cdp_mac);  // received from, MAC address = 6 bytes

        cdpDataIndex += 2;
        return cdpDataIndex;
      } else {
        return (0);
      }
    }
  }

  return (0);
}


PINFO cdp_packet_handler(byte cdpData[], size_t plen) {
  unsigned int cdpDataIndex = 22;
  int cdpVersion = cdpData[cdpDataIndex];

  // 1 byte: TTL - Time To Live in seconds
  cdpDataIndex++;
  int cdpTtl = cdpData[cdpDataIndex];
  cdpinfo.TTL[1] = cdpTtl;
  cdpinfo.Proto[1] = "CDP";
  cdpinfo.ProtoVer[1] = String(cdpVersion);
  cdpDataIndex++;

  // 2 byte: Checksum
  cdpDataIndex += 2;

  while (cdpDataIndex < plen) {
    unsigned int cdpFieldType = (cdpData[cdpDataIndex] << 8) | cdpData[cdpDataIndex + 1];
    cdpDataIndex += 2;
    unsigned int cdpFieldLength = (cdpData[cdpDataIndex] << 8) | cdpData[cdpDataIndex + 1];
    cdpDataIndex += 2;
    cdpFieldLength -= 4;

    switch (cdpFieldType) {
      case 0x0001:
        {
          // Device ID / Device name
          // Split device name into name and domain
          String tmpStr = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);
          byte pos = tmpStr.indexOf('.');
          if (pos > 0) {
            cdpinfo.SWName[1] = tmpStr.substring(0, pos);
            cdpinfo.SWDomain[1] = tmpStr.substring(pos + 1);
          } else {
            cdpinfo.SWName[1] = tmpStr;
            cdpinfo.SWDomain[1] = "";
          }
          break;
        }

      case 0x0002:
        {
          // IP addresses
          // 4 byte: Number of addresses
          // for each IP address:
          //    1 byte: Protocol type
          //            0x01: NLPID
          //            0x02: 802.2
          //    1 byte: Protocol length
          //    x byte: Protocol
          //            0x81: ISO CLNS (protocol type 3D 1)
          //            0xcc: IP
          //            0xAAAA03 000000 0800: Pv6 (protocol type 3D 2)
          //            0xAAAA03 000000 6003: DECNET Phase IV (protocol type 3D 2)
          //            0xAAAA03 000000 809B: AppleTalk (protocol type 3D 2)
          //            0xAAAA03 000000 8137: Novell IPX (protocol type 3D 2)
          //            0xAAAA03 000000 80c4: Banyan VINES (protocol type 3D 2)
          //            0xAAAA03 000000 0600: XNS (protocol type 3D 2)
          //            0xAAAA03 000000 8019: Apollo Domain (protocol type 3D 2)
          //    2 byte: Address length
          //    4 byte: (for IP protocol?): IP address
          cdpinfo.IP[1] = handleCdpAddresses(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        }

      case 0x0003:
        {
          // Port ID / Port Name
          // Strip unnecessary data, only having the last port number
          String tmpStr = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);
          if (tmpStr.indexOf('/') > 0) {
            tmpStr = tmpStr.substring(tmpStr.lastIndexOf('/') + 1);
          }
          cdpinfo.Port[1] = tmpStr;
          break;
        }

      case 0x0004:
        {
          // Capabilities
          // 4 byte: Capabilities
          //         0b000 0000 0000 0000 0000 0000 0000 000x = Router
          //         0b000 0000 0000 0000 0000 0000 0000 00x0 = Transparent Birdge
          //         0b000 0000 0000 0000 0000 0000 0000 0x00 = Source Route Bridge
          //         0b000 0000 0000 0000 0000 0000 0000 x000 = Switch
          //         0b000 0000 0000 0000 0000 0000 000x 0000 = Host
          //         0b000 0000 0000 0000 0000 0000 00x0 0000 = IGMP capable
          //         0b000 0000 0000 0000 0000 0000 0x00 0000 = Repeater
          //         0b000 0000 0000 0000 0000 0000 x000 0000 = VoIP Phone
          //         0b000 0000 0000 0000 0000 000x 0000 0000 = Remotely Managed Device
          //         0b000 0000 0000 0000 0000 00x0 0000 0000 = CVTA/STP Dispute Resolution/Cisco VT Camera
          //         0b000 0000 0000 0000 0000 0x00 0000 0000 = Two Port Mac Relay
          cdpinfo.Cap[1] = handleCdpCapabilities(cdpData, cdpDataIndex + 2, cdpFieldLength - 2);
          break;
        }

      case 0x0005:
        {
          // Software version
          cdpinfo.SWver[1] = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        }

      case 0x0006:
        {
          // Platform / CDP Model Name
          //cdpinfo.Model[ 1 ] = handleCdpAsciiField( cdpData, cdpDataIndex, cdpFieldLength );
          String tmpStr = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);
          uint16_t tmp16 = tmpStr.indexOf("\n");
          cdpinfo.Model[1] = tmpStr.substring(0, tmp16 - 1);
          break;
        }

        /*
      case 0x0007:
        // IP Prefix
        break;
*/

      case 0x0008:
        {
          // Hello Protocol
          // 3 byte: OUT
          //        0x00000c: Cisco Systems, Inc
          // 2 byte: Protocol ID
          //   0x0112: Cluster Management
          //     4 byte: Cluster Master IP
          //     4 byte: IP Network Mask
          //     1 byte: Version?
          //     1 byte: Sub Version?
          //     1 byte: Status?
          //     1 byte: UNKNOWN
          //     6 byte: Cluster Commander MAC Address
          //     6 byte: Switch's MAC Address
          //     1 byte: uNKNOWN
          //     2 byte: Management VLAN

          /*
        // Get 3 byte OUT
        tmp32 = ( cdpData[ cdpDataIndex ] << 16 ) | ( cdpData[ cdpDataIndex + 1 ] << 8 ) | cdpData[ cdpDataIndex + 2 ];
        cdpDataIndex += 3;
        cdpFieldLength -= 3;

        if( tmp32 == 0x0000000c )
        {
          // OUT: Cisco
          // Get 2 byte protocol ID
          tmp16 = ( cdpData[ cdpDataIndex ] << 8 ) | cdpData[ cdpDataIndex + 1 ];
          cdpDataIndex += 2;
          cdpFieldLength -= 2;

          if( tmp16 == 0x0112 )
          {
            // Protocol ID: Cluster Management
            // Get 4 byte Cluster Master IP

            // ...

            // Get Management VLAN
            cdpinfo.MgmtVLAN[ 1 ] = String( ( cdpData[ cdpDataIndex + 25 ] << 8 ) | cdpData[ cdpDataIndex + 26 ], DEC );

//            cdpinfo.MgmtVLAN[ 1 ] = handleCdpNumField( cdpData, cdpDataIndex + 25, cdpFieldLength - 25 );        
//#ifdef DEBUGSERIAL
//Serial.println( "CDP Hello MgmtVLAN: " + cdpinfo.MgmtVLAN[ 1 ] );
//#endif
          }
        }
*/
          break;
        }

      case 0x0009:
        {
          // VTP (VLAN Trunk Protocol) Management Domain
          cdpinfo.VTP[1] = handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        }

      case 0x000a:
        {
          // Native VLAN / CDP VLAN #
          // 2 byte: VLAN Number
          cdpinfo.VLAN[1] = handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        }

      case 0x000b:
        {
          // Half or full duplex
          // 1 byte: Duplex
          //  0b0000 000x: 1 = Full Duplex
          cdpinfo.Dup[1] = handleCdpDuplex(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        }

      case 0x000e:
        {
          // VoIP VLAN Reply / CDP VLAN voice#
          // 1 byte: Data
          // 2 byte: Voice VLAN Number
          cdpinfo.VoiceVLAN[1] = handleCdpVoiceVLAN(cdpData, cdpDataIndex + 1, cdpFieldLength - 1);
          break;
        }

      case 0x000f:
        {
          // VoIP VLAN query
          // four bytes received on Yealink W60 containing hex:
          // 0x20 0x02 0x00 0x01
          // 0b 0010 0000 0000 0010 0000 0000 0000 0001
          // Binary values:
          // 0x20: 0010 0000
          // 0x02: 0000 0010
          // 0x00: 0000 0000
          // 0x01: 0000 0001
          // Since VLAN IDs are only 12 bit values it is not clear what is requested exactly.
          // Maybe used as bit fields?
#ifdef DEBUGSERIAL
          Serial.println("CDP VoIP VLAN query: " + handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      case 0x0010:
        {
          // Power consumption, requested power by end device?
          String tmpStr = handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength);
          if (tmpStr != "0") {
            cdpinfo.PoECons[1] = tmpStr + "mWh";
          }
#ifdef DEBUGSERIAL
          Serial.println("CDP Power consumption: " + cdpinfo.PoECons[1]);
#endif
          break;
        }

      case 0x0011:
        {
          // MTU
#ifdef DEBUGSERIAL
          Serial.println("CDP MTU: " + handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      case 0x0012:
        {
          // Trust Bitmap
          // 1 byte: Trust Bitmap
#ifdef DEBUGSERIAL
          Serial.println("CDP Trust Bitmap: " + handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      case 0x0013:
        {
          // Untrusted Port CoS
          // 1 byte: Untrust Port CoS
#ifdef DEBUGSERIAL
          Serial.println("CDP Untrusted Port CoS: " + handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      case 0x0014:
        {
          // System Name
#ifdef DEBUGSERIAL
          Serial.println("CDP System Name: " + handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      case 0x0015:
        {
          // System Object Identifier
#ifdef DEBUGSERIAL
          Serial.println("CDP System Object Identifier: " + handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      case 0x0016:
        {
          // Management Address(es)
          // 4 byte: Number of addresses
          // for each IP address:
          //    1 byte: Protocol type
          //            0x01: NLPID
          //    1 byte: Protocol length
          //    1 byte: Protocol
          //            0xcc: IP
          //    2 byte: Address length
          //    4 byte: (for IP protocol?): IP address
          cdpinfo.MgmtIP[1] = handleCdpAddresses(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        }


      case 0x0017:
        // Location
#ifdef DEBUGSERIAL
        Serial.println("CDP Location: " + handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
        break;


        /*
      case 0x0018:
        // External Port-ID
        break;
*/

        /*
      case 0x0019:
        // Power Requested
        break;
*/

      case 0x001a:
        {
          // Power Available
          // 2 byte: Request ID
          // 2 byte: Management-ID
          // 4 byte: Power Available in mW
          // 4 byte: UNKNOWN? 0xffffffff
          if (cdpFieldLength < 9) {
            cdpinfo.PoEAvail[1] = "not available";
            break;
          }

          cdpinfo.PoEAvail[1] = handleCdpNumField(cdpData, cdpDataIndex + 4, cdpFieldLength - 8) + "mWh";
          String tmpStr = handleCdpNumField(cdpData, cdpDataIndex + 4, cdpFieldLength - 8);
          if (tmpStr != "0") {
            cdpinfo.PoEAvail[1] = tmpStr + "mWh";
#ifdef DEBUGSERIAL
            Serial.println("Power Available: " + cdpinfo.PoEAvail[1]);
#endif
          }
          break;
        }

        /*
      case 0x001b:
        // Port Unidirectional
        break;
*/

        /*
      case 0x001d:
        // EnergyWise over CDP
        break;
*/

      case 0x001f:
        {
          // Spare Pair PoE
          // 1 byte: Spare Pair PoE
          //        0b0000 000x = PSE Four-Wire PoE
          //        0b0000 00x0 = PD Spare Pair Architecture: 0=Independent
          //        0b0000 0x00 = PD Request Spare Pair PoE
          //        0b0000 x000 = PSE Spare Pair PoE
#ifdef DEBUGSERIAL
          Serial.println("CDP Spare Pair PoE: " + handleCdpNumField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      case 0x1003:
        {
          // Radio Channel
          // 1 byte: Platform
#ifdef DEBUGSERIAL
          Serial.println("CDP Radio Channel: " + handleCdpAsciiField(cdpData, cdpDataIndex, cdpFieldLength));
#endif
          break;
        }

      default:
        {
#ifdef DEBUGSERIAL
          Serial.println("CDP unhandled type: 0x" + String(cdpFieldType, HEX));
          Serial.println("CDP field length:   " + String(cdpFieldLength, DEC));
          for (uint16_t i = 0; i < cdpFieldLength; i++) {
            String data = "00" + String(cdpData[cdpDataIndex + i], HEX);
            data = "0x" + data.substring(data.length() - 2);
            Serial.print(data + " ");
            if (((i + 1) % 8) == 0) Serial.println();
          }
          Serial.println();
#endif
          break;
        }
    }
    cdpDataIndex += cdpFieldLength;
  }
  return cdpinfo;
}

bool byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length) {
  for (unsigned int i = offset, j = 0; j < length; ++i, ++j) {
    if (a[i] != b[j]) {
      return false;
    }
  }
  return true;
}

String print_mac(const byte a[], unsigned int offset, unsigned int length) {
  String Mac = "";
  for (unsigned int i = offset; i < offset + length; ++i) {
    if (a[i] < 0x10)
      Mac += ('0');

    Mac += String(a[i], HEX);
  }
  return Mac;
}

String CdpCapabilities(String temp) {
  String output;

  if (temp.substring(15, 16) == "1") {
    output = output + "Router ";
  }
  if (temp.substring(14, 15) == "1") {
    output = output + "Trans_Bridge ";
  }
  if (temp.substring(13, 14) == "1") {
    output = output + "Route_Bridge,";
  }
  if (temp.substring(12, 13) == "1") {
    output = output + "Switch ";
  }
  if (temp.substring(11, 12) == "1") {
    output = output + "Host ";
  }
  if (temp.substring(10, 11) == "1") {
    output = output + "IGMP ";
  }
  if (temp.substring(9, 10) == "1") {
    output = output + "Repeater ";
  }
  if (temp.substring(8, 9) == "1") {
    output = output + "VoIP Phone ";
  }
  if (temp.substring(7, 8) == "1") {
    output = output + "RemMgmDev ";
  }
  if (temp.substring(6, 7) == "1") {
    output = output + "Camera ";
  }
  if (temp.substring(5, 6) == "1") {
    output = output + "2PortMacRelay ";
  }

  return output;
}

String handleCdpCapabilities(const byte a[], unsigned int offset, unsigned int lengtha) {
  String temp;
  for (unsigned int i = offset; i < (offset + lengtha); ++i) {
    temp = temp + print_binary(a[i], 8);
  }
  return CdpCapabilities(temp);
}

String print_binary(int v, int num_places) {
  String output;
  int mask = 0, n;
  for (n = 1; n <= num_places; n++) {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask;  // truncate v to specified number of places

  while (num_places) {
    if (v & ((0x0001 << num_places) - 1)) {
      output = output + "1";
    } else {
      output = output + "0";
    }
    --num_places;
  }
  return output;
}

String handleCdpNumField(const byte a[], unsigned int offset, unsigned int length) {
  unsigned long num = 0;
  String temp;
  for (unsigned int i = 0; i < length; ++i) {
    num <<= 8;
    num += a[offset + i];
  }
  temp = String(num, DEC);
  return temp;
}

String handleCdpVoiceVLAN(const byte a[], unsigned int offset, unsigned int length) {
  unsigned long num = 0;
  for (unsigned int i = offset; i < (offset + length); ++i) {
    num <<= 8;
    num += a[i];
  }
  return String(num, DEC);
}

String handleCdpAddresses(const byte a[], unsigned int offset, unsigned int lengtha) {
  String temp;
  unsigned long numOfAddrs = (a[offset] << 24) | (a[offset + 1] << 16) | (a[offset + 2] << 8) | a[offset + 3];
  offset += 4;

  if (numOfAddrs < 5) {
    for (unsigned long i = 0; i < numOfAddrs; ++i) {
      offset++;
      unsigned int protoLength = a[offset++];
      offset += protoLength;
      unsigned int addressLength = (a[offset] << 8) | a[offset + 1];
      offset += 2;
      byte address[4];
      if (addressLength != 4) {
        return ("");
      } else {
        for (unsigned int j = 0; j < addressLength; ++j) {
          address[j] = a[offset++];
          temp += address[j];
          if (j < 3) {
            temp += ".";
          }
        }
      }
    }

    return temp;
  } else {
    return "CORRUPT_IP";
  }
}

String handleCdpDuplex(const byte a[], unsigned int offset, unsigned int length) {
  String temp;
  if (a[offset]) {
    temp = "Full";
  } else {
    temp = "Half";
  }
  return temp;
}

String handleCdpAsciiField(byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;
  char temp[lengtha + 1];
  for (unsigned int i = offset; i < (offset + lengtha); ++i, ++j) {
    temp[j] = a[i];
  }
  temp[lengtha] = '\0';

  String temp1 = temp;
  return temp1;
}
