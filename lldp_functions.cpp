/* 
lldp_functions.cpp

Evaluate received LLDP packages
Some details added as description / comments from WireShark
IEEE 802.1AB:
https://www.ieee802.org/3/frame_study/0409/blatherwick_1_0409.pdf

2023-12-18: Initial version
*/

#include "Definitions.h"
#include <Arduino.h>
#include "lldp_functions.h"
#include "Packet_data.h"

PINFO lldpinfo;

int lldpIPaddr = 0;

// LLDP broadcast address
const byte lldp_mac[] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e };

byte lldp_encbuff[1500];

#ifdef DEBUGSENDLLDP
// Use the following variables from the main sketch
extern byte eth_buffcheck[];
extern PINFO eth_lldpPacket;
#endif

unsigned int lldp_check_Packet(byte lldp_encbuff[], unsigned int length) {
  if (length > 0) {
    if (length > sizeof(lldp_encbuff))
      length = sizeof(lldp_encbuff);

    unsigned int lldpDataIndex = 0;
    if (lldp_byte_array_contains(lldp_encbuff, lldpDataIndex, lldp_mac, sizeof(lldp_mac))) {
      // LDDP Packet found and is now getting processed
#ifdef DEBUGSERIAL
      Serial.println("\n\nLLDP Packet Recieved");
#endif

      lldpDataIndex += sizeof(lldp_mac);  // Increment index the length of the source MAC address

      // Get source MAC Address
      byte* macFrom = lldp_encbuff + lldpDataIndex;
      lldpinfo.MAC[1] = lldp_print_mac(macFrom, 0, 6);
      lldpDataIndex += sizeof(lldp_mac);  // Increment index the length of the MAC address
      lldpDataIndex += 2;
      return lldpDataIndex;
    } else {
      return (0);
    }
  }
  return (0);
}


bool lldp_byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length) {
  for (unsigned int i = offset, j = 0; j < length; ++i, ++j) {
    if (a[i] != b[j])
      return false;
  }
  return true;
}


PINFO lldp_packet_handler(byte lldpData[], uint16_t plen) {
  lldpinfo.Proto[1] = "LLDP";
  byte* macFrom = lldpData + sizeof(lldp_mac);
  lldp_print_mac(macFrom, 0, 6);
  unsigned int lldpDataIndex = 14;
  uint32_t OUI;
  uint8_t OUIsubType;

  while (lldpDataIndex < plen) {
    // https://en.wikipedia.org/wiki/Link_Layer_Discovery_Protocol#Frame_structure
    // TLV structure
    // Type    Length  Value
    // 7 bits  9 bits  0-511 octets

    // read all remaining TLV fields
    unsigned int lldpFieldType = lldpData[lldpDataIndex] >> 1;
    unsigned int lldpFieldLength = (lldpData[lldpDataIndex] & 0x01) << 8;
    lldpDataIndex += 1;
    lldpFieldLength += lldpData[lldpDataIndex];
    lldpDataIndex += 1;

    switch (lldpFieldType) {
      case 0x0000:
        // End of LLDPDU
        break;

      case 0x0001:
        {
          // Chassis ID / Chassis Type
          lldpinfo.ChassisID[1] = handlePortSubtype(lldpData, lldpDataIndex, lldpFieldLength);
          break;
        }

      case 0x0002:
        {
          // Port / Port ID
          // Strip unnecessary data, only having the last port number
          String tmpStr = handlePortSubtype(lldpData, lldpDataIndex, lldpFieldLength);
          if (tmpStr.indexOf('/') > 0) {
            tmpStr = tmpStr.substring(tmpStr.lastIndexOf('/') + 1);
          }
          lldpinfo.Port[1] = tmpStr;
          break;
        }

      case 0x0003:
        {
          // TTL - Time to live
          // 16 bit value in seconds
          lldpinfo.TTL[1] = lldp_handleCdpNumField(lldpData, lldpDataIndex, lldpFieldLength);
          break;
        }

      case 0x0004:
        {
          // Port Description
          // Strip unnecessary data, only having the last port number
          String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
          if (tmpStr.indexOf('/') > 0) {
            tmpStr = tmpStr.substring(tmpStr.lastIndexOf('/') + 1);
          }
          lldpinfo.PortDesc[1] = tmpStr;
          break;
        }

      case 0x0005:
        {
          // Device Name
          // Split device name into name and domain
          String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
          byte pos = tmpStr.indexOf('.');
          if (pos > 0) {
            lldpinfo.SWName[1] = tmpStr.substring(0, pos);
            lldpinfo.SWDomain[1] = tmpStr.substring(pos + 1);
          } else {
            lldpinfo.SWName[1] = tmpStr;
            lldpinfo.SWDomain[1] = "";
          }
          break;
        }

      case 0x0006:
        {
          // Model Name / System Description
          String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
          uint16_t tmp16 = tmpStr.indexOf("\n");
          tmpStr = tmpStr.substring(0, tmp16 - 1);
          if (lldpinfo.Model[1] == "-")
            lldpinfo.Model[1] = tmpStr;
          break;
        }

      case 0x0007:
        {
          // System Capabilities
          handlelldpCapabilities(lldpData, lldpDataIndex + 2, lldpFieldLength - 2);
          break;
        }

      case 0x0008:
        {
          // Management IP Address
          lldpinfo.IP[1] = handleManagementSubtype(lldpData, lldpDataIndex, lldpFieldLength);
          break;
        }

      case 0x007f:  // Custom TLVs
        {
          // Custom TLVs
          // https://www.ieee802.org/3/frame_study/0409/blatherwick_1_0409.pdf
          // 3 byte: Organizationally Unique Identifier (OUI)
          // 1 byte: Group-defined TLV subtype
          // 0 < n < 507 bytes: Group defined information string
          OUI = (lldpData[lldpDataIndex + 0] << 16) + (lldpData[lldpDataIndex + 1] << 8) + (lldpData[lldpDataIndex + 2]);
          OUIsubType = lldpData[lldpDataIndex + 3];
          lldpDataIndex += 4;
          lldpFieldLength -= 4;
#ifdef DEBUGSERIAL
          Serial.println("\n\nLLDP custom type OUI:     " + String(OUI, HEX));
          Serial.println("LLDP custom type subtype: " + String(OUIsubType, HEX));
          Serial.println("LLDP custom data length:  " + String(lldpFieldLength, DEC));
          Serial.println("lldp custom field:");
          for (uint16_t i = 0; i < lldpFieldLength; i++) {
            String data = "00" + String(lldpData[lldpDataIndex + i], HEX);
            data = "0x" + data.substring(data.length() - 2);
            Serial.print(data + " ");
          }
          Serial.println("\n\n");
#endif

          switch (OUI) {
              // OUIs from https://wiki.wireshark.org/LinkLayerDiscoveryProtocol
              // 00-12-0F - IEEE 802.3
              // 00-12-BB - TIA TR-41 Committee - Media Endpoint Discovery (LLDP-MED, ANSI/TIA-1057)
              // 00-0E-CF - PROFIBUS International (PNO) Extension for PROFINET discovery information
              // 00-80-c2 - IEEE 802.1
              // 30-B2-16 - Hytec Geraetebau GmbH Extensions

            case 0x00120f:  // IEEE 802.3
              {
#ifdef DEBUGSERIAL
                Serial.println("\n\nLLDP custom type OUI:     " + String(OUI, HEX));
                Serial.println("LLDP custom type subtype: " + String(OUIsubType, HEX));
                Serial.println("LLDP custom data length:  " + String(lldpFieldLength, DEC));
                Serial.println("lldp custom field:");
                for (uint16_t i = 0; i < lldpFieldLength; i++) {
                  String data = "00" + String(lldpData[lldpDataIndex + i], HEX);
                  data = "0x" + data.substring(data.length() - 2);
                  Serial.print(data + " ");
                }
                Serial.println("\n\n");
#endif

                switch (OUIsubType) {
                    // Annex G of the LLDP specification defines the following set of IEEE 802.3 Organizationally Specific TLVs:
                    // https://www.ieee802.org/3/frame_study/0409/blatherwick_1_0409.pdf
                    // MAC/PHY Configuration/Status TLV (OUI = 00-12-0f, Subtype = 1)
                    // Subtype 1: MAC/PHY Configuration/Status
                    // Subtype 2: MDI Power Support (MDI=Media Dependent Interface)
                    // Subtype 3: Link Aggregation
                    // Subtype 4: Maximum Frame Size
                    // Subtype 5: Energy-Efficient Ethernet
                    // Subtype 6: unknown
                    // Subtype 7: IEEE 802.3br Frame Preemption Protocol

                  case 1:  // MAC/PHY Configuration/Status
                    {
                      // 1 byte: Auto-Negotiation Support/Status:
                      //         0b0000 000x = Auto-Negotiation: 1=Supported
                      //         0b0000 00x0 = Auto-Negotiation: 1=Enabled
                      // 2 byte: PMD Auto-Negotiation Advertised Capability
                      //         0b0000 0000 0000 000x = 1000BASE-T (full duplex mode)
                      //         0b0000 0000 0000 00x0 = 1000BASE-T (half duplex mode)
                      //         0b0000 0000 0000 0x00 = 1000BASE-X (-LX, -SX, -CX full duplex mode)
                      //         0b0000 0000 0000 x000 = 1000BASE-X (-LX, -SX, -CX half duplex mode)
                      //         0b0000 0000 000x 0000 = Asymmetric and Symmetric PAUSE (for full-duplex links)
                      //         0b0000 0000 00x0 0000 = Symmetric PAUSE (for full-duplex links)
                      //         0b0000 0000 0x00 0000 = Asymmetric PAUSE (for full-duplex links)
                      //         0b0000 0000 x000 0000 = PAUSE (for full-duplex links)
                      //         0b0000 000x 0000 0000 = 100BASE-T2 (full duplex mode)
                      //         0b0000 00x0 0000 0000 = 100BASE-T2 (half duplex mode)
                      //         0b0000 0x00 0000 0000 = 100BASE-TX (full duplex mode)
                      //         0b0000 x000 0000 0000 = 100BASE-TX (half duplex mode)
                      //         0b000x 0000 0000 0000 = 100BASE-T4
                      //         0b00x0 0000 0000 0000 = 10BASE-T (full duplex mode)
                      //         0b0x00 0000 0000 0000 = 10BASE-T (half duplex mode)
                      //         0bx000 0000 0000 0000 = Other or unknown
                      // 2 byte: Operational MAU Type: (0x0000) = Other or unknown
#ifdef DEBUGSERIAL
                      byte autoNeg = lldpData[lldpDataIndex + 0];
                      String autoNegStr = "Auto negotiation ";
                      if (autoNeg & (0x01 == 0)) autoNegStr += "not ";
                      autoNegStr += "supported / ";
                      if (autoNeg & (0x02 == 0)) autoNegStr += "not ";
                      autoNegStr += "enabled";

                      uint16_t autoNegAdvCap = (lldpData[lldpDataIndex + 1] << 8) | lldpData[lldpDataIndex + 2];
                      Serial.println(autoNegStr);
                      Serial.println("PMD Auto-Negotiation Advertised Capability: " + String(autoNegAdvCap, BIN));
#endif
                      break;
                    }  // case 1: MAC/PHY Configuration/Status

                  case 2:  // MDI Power Support
                    {
                      // 1 byte: Port Class, Support capability, Enabled (pethPsePortAdminEnable), Pair control ability (pethPsePortPowerPairContolAbility)
                      //   bit 0: Port class: 1 = PSE, 0 = PD
                      //   bit 1: Power Sourcing Equipment (PSE): MDI power support: 1 = supported, 0 = not supported
                      //   bit 2: PSE MDI power state: 1 = enabled, 0 = disabled
                      //   bit 3: PSE pairs control ability: 1 = pair selection can be controlled, 0 = pair selection can not be controlled
                      //   bit 4-7: reserved for future use
                      //
                      // 1 byte: Power Pairs as defined in pethPsePortPowerPairs:
                      //   bit 1: signal pairs only are in use
                      //   bit 2: spare pairs only are in use. or binary coded?
                      //
                      // 1 byte: Power Class as defined in pethPsePortPowerClassification:
                      //   0: not available?
                      //   1: Class 0: 0.44W to 12.95W, max Power 15.4W
                      //   2: Class 1: 0.44W to 3.84W, max Power 4.00W
                      //   3: Class 2: 3.84W to 6.49W, max Power 7.00W
                      //   4: Class 3: 6.49W to 12.95W, max Power 15.4W
                      //   5: Class 4: 12.95W to 25.5W, max Power 30W
                      //   6: Class 5: 40W,, max Power 45W
                      //   7: Class 6: 51W,, max Power 60W
                      //   8: Class 7: 62W,, max Power 75W
                      //   9: Class 8: 71.3W,, max Power 99W
                      // 1 byte: Type/source priority
                      //   0b000000xx power priority: 11=low;10=high;01=critical;00=unknown
                      //   0b0000xx00 reserved
                      //   0b00xx0000 power source
                      //   0b0x000000 power type: 1=PD; 0=PSE
                      //   0bx0000000 power type: 1=Type 1; 0=Type 2
                      // 2 byte: PD requested power value, 0–25.5 W in 0.1 W steps
                      // 2 byte: PSE allocated power value, 0–25.5 W in 0.1 W steps

#ifdef DEBUGSERIAL
                      byte portClass = lldpData[lldpDataIndex + 0];
                      String portClassStr;
                      if ((portClass & 0x01) == 0x01)
                        portClassStr += "PSE";
                      else
                        portClassStr += "PD";
                      portClassStr += "\nMDI power ";
                      if ((portClass & 0x02) == 0x02)
                        portClassStr += "";
                      else
                        portClassStr += "not";
                      portClassStr += " supported\nPSE MDI power state: ";
                      if ((portClass & 0x04) == 0x04)
                        portClassStr += "enabled";
                      else
                        portClassStr += "disabled";
                      portClassStr += "\nPSE pairs control ability: pair selection can ";
                      if ((portClass & 0x08) == 0x08)
                        portClassStr += "";
                      else
                        portClassStr += "not ";
                      portClassStr += "be controlled";

                      // The PSE power pair field shall contain an integer value as defined by the pethPsePortPowerPairs object in IETF RFC 3621.
                      byte powerPairs = lldpData[lldpDataIndex + 1];

                      // The power class field shall contain an integer value as defined by the pethPsePortPowerClassifications object in IETF RFC 3621.
                      byte powerClass = lldpData[lldpDataIndex + 2];

                      switch (powerClass) {
                        case 0:
                          lldpinfo.PoEAvail[1] = "n/a";
                          break;
                        case 1:
                          lldpinfo.PoEAvail[1] = "0.44W-12.95W";
                          break;
                        case 2:
                          lldpinfo.PoEAvail[1] = "0.44W-3.84W";
                          break;
                        case 3:
                          lldpinfo.PoEAvail[1] = "3.84W-6.49W";
                          break;
                        case 4:
                          lldpinfo.PoEAvail[1] = "6.49W-12.95W";
                          break;
                        case 5:
                          lldpinfo.PoEAvail[1] = "12.95W-25.5W";
                          break;
                        case 6:
                          lldpinfo.PoEAvail[1] = "40W";
                          break;
                        case 7:
                          lldpinfo.PoEAvail[1] = "51W";
                          break;
                        case 8:
                          lldpinfo.PoEAvail[1] = "62W";
                          break;
                        case 9:
                          lldpinfo.PoEAvail[1] = "71.3W";
                          break;
                        default:
                          break;
                      }
                      byte typeSourcePrio = lldpData[lldpDataIndex + 3];

                      Serial.println(portClassStr);
                      Serial.println("\nPower pairs: " + String(powerPairs) + " / 1=signal pairs only are in use, 2=spare pairs only are in use");
                      Serial.println("\nPower class: " + String(powerClass - 1) + " (0-9): " + lldpinfo.PoEAvail[1]);
                      Serial.println("\nType Source Prio: " + String(typeSourcePrio));

                      uint16_t pdReqPower = (lldpData[lldpDataIndex + 4] << 8) | lldpData[lldpDataIndex + 5];
                      String pdReqPowerStr = String((float)(pdReqPower) / 10.0) + "Wh";
                      uint16_t pdAllocPower = (lldpData[lldpDataIndex + 6] << 8) | lldpData[lldpDataIndex + 7];
                      String pdAllocPowerStr = String((float)(pdAllocPower) / 10.0) + "Wh";
                      Serial.println("\nPD requested power: " + pdReqPowerStr);
                      Serial.println("\nPSE allocated power: " + pdAllocPowerStr);
#endif
                      break;
                    }  // case 2: MDI Power Support

                  case 3:  // Link Aggregation
                    {
                      // 1 byte: Capability and Status: Bit mask for capability and current aggregation status
                      //         bit 0 Aggregation capability 0 = not capable of being aggregated, 1 = capable of being aggregated
                      //         bit 1 Aggregation status 0 = not currently in aggregation, 1 = currently in aggregation
                      // 4 byte: Port Identifier, derived from ifNumber in ifIndex (aAggPortID)
#ifdef DEBUGSERIAL
                      byte capStat = lldpData[lldpDataIndex + 0];
                      uint32_t portID = (lldpData[lldpDataIndex + 1] << 24) | (lldpData[lldpDataIndex + 2] << 16) | (lldpData[lldpDataIndex + 3] << 8) | lldpData[lldpDataIndex + 4];
                      Serial.println("Link Aggregation:\nCapability and Status: " + String(capStat) + "\nPort ID: " + String(portID));
#endif
                      break;
                    }  // case 3: Link Aggregation

                  case 4:  // Maximum Frame Size
                    {
                      // 2 byte: - Basic MAC frame size (subclause 3.1.1) is 1518
                      //         - Tagged MAC frame size (subclause 3.5) is 1522
                      //         - Other frame sizes are implementation dependent0
#ifdef DEBUGSERIAL
                      uint16_t frameSize = (lldpData[lldpDataIndex + 0] << 8) | lldpData[lldpDataIndex + 1];
                      Serial.println("Maximum frame size:: " + String(frameSize));
#endif
                      break;
                    }  // case 4: Maximum Frame Size

                  case 5:  // Energy-Efficient Ethernet
                    {
#ifdef DEBUGSERIAL
                      Serial.println("CDP: unhandled Energy efficient Ethernet subtype");
#endif
                      break;
                    }

                  case 7:  // IEEE 802.3br Frame Preemption Protocol
                    {
#ifdef DEBUGSERIAL
                      Serial.println("CDP: unhandled IEEE 802.3br Frame Preemption Protocol");
#endif
                      break;
                    }

                  default:
                    {
                      break;
                    }
                }  // switch( OUIsubType )

                break;
              }  // case 0x00120f: IEEE 802.3

              // #######################################################

            case 0x0012bb:  // TIA TR-41 Committee - Media Endpoint Discovery (LLDP-MED, ANSI/TIA-1057)
              {
                // 0x0012bb: Telecommunications In Media Subtype
                // The LLDP-MED specification defines the following set of TIA Organizationally Specific TLVs:
                // LLDP-MED Capabilities TLV (OUI = 00-12-BB, Subtype = 1)
                // Network Policy TLV (OUI = 00-12-BB, Subtype = 2)
                // Location Identification TLV (OUI = 00-12-BB, Subtype = 3)
                // Extended Power-via-MDI TLV (OUI = 00-12-BB, Subtype = 4)
                // Inventory - Hardware Revision TLV (OUI = 00-12-BB, Subtype = 5)
                // Inventory - Firmware Revision TLV (OUI = 00-12-BB, Subtype = 6)
                // Inventory - Software Revision TLV (OUI = 00-12-BB, Subtype = 7)
                // Inventory - Serial Number TLV (OUI = 00-12-BB, Subtype = 8)
                // Inventory - Manufacturer Name TLV (OUI = 00-12-BB, Subtype = 9)
                // Inventory - Model Name TLV (OUI = 00-12-BB, Subtype = 10)
                // Inventory - Asset ID TLV (OUI = 00-12-BB, Subtype = 11)
                switch (OUIsubType) {
                  case 1:  // LLDP-MED Capabilities TLV
                    {
                      // 2 byte: Capabilities
                      //         0b0000 0000 0000 000x = 0x0001 LLPD-MED Capabilities
                      //         0b0000 0000 0000 00x0 = 0x0002 Network policy
                      //         0b0000 0000 0000 0x00 = 0x0004 Location Identification
                      //         0b0000 0000 0000 x000 = 0x0008 Extended Power via MDI-PSE
                      //         0b0000 0000 000x 0000 = 0x0010 Extended Power via MDI-PD
                      //         0b0000 0000 00x0 0000 = 0x0020 Inventory
                      // 1 byte: Class Type
                      //         0: Type Not Defined
                      //         1: Endpoint Class I
                      //         2: Endpoint Class II
                      //         3: Endpoint Class III
                      //         4: Network Connectivity
#ifdef DEBUGSERIAL
                      uint16_t tmp16 = (lldpData[lldpDataIndex + 0] << 8) | lldpData[lldpDataIndex + 1];
                      String tmpStr = "Capabilities:\n";
                      if ((tmp16 & 0x001) != 0) tmpStr += "LLPD-MED\n";
                      if ((tmp16 & 0x002) != 0) tmpStr += "Network policy\n";
                      if ((tmp16 & 0x004) != 0) tmpStr += "Location Identification\n";
                      if ((tmp16 & 0x008) != 0) tmpStr += "Extended Power via MDI-PSE\n";
                      if ((tmp16 & 0x010) != 0) tmpStr += "Extended Power via MDI-PD\n";
                      if ((tmp16 & 0x020) != 0) tmpStr += "Inventory";
                      Serial.println(tmpStr + "\n");  // Capabilities

                      uint8_t tmp8 = lldpData[lldpDataIndex + 2];
                      tmpStr = "Class type: ";
                      switch (tmp8) {
                        case 0:
                          tmpStr += " Type Not Defined";
                          break;
                        case 1:
                          tmpStr += "Endpoint Class I";
                          break;
                        case 2:
                          tmpStr += "Endpoint Class II";
                          break;
                        case 3:
                          tmpStr += "Endpoint Class III";
                          break;
                        case 4:
                          tmpStr += "Network Connectivity";
                          break;
                        default:
                          tmpStr += "unknown type " + String(tmp8);
                          break;
                      }

                      Serial.println(tmpStr);  // Class type
#endif
                      break;
                    }  // case 1: LLDP-MED Capabilities TLV

                  case 2:  // Network Policy
                    {
                      // 1 byte: Media application
                      //         0: Reserved
                      //         1: Voice
                      //         2: Voice Signaling
                      //         3: Guest Voice
                      //         4: Guest Voice Signaling
                      //         5: Softphone Voice
                      //         6: Video Conferencing
                      //         7: Streaming Video
                      //         8: Video Signaling
                      // 3 byte: Flags
                      //         0x00003F DSCP Priority
                      //         0x0001C0 Media L2 Priority
                      //         0x1FFE00 Media VLAN ID
                      //         0x400000 Media tag flag: tagged VLAN if set
                      //         0x800000 Media policy flag
                      if (lldpFieldLength < 4) {
#ifdef DEBUGSERIAL
                        Serial.println("TLV length too short.");
#endif
                        break;
                      }
                      byte mediaApplication = lldpData[lldpDataIndex];
                      String tmpStr = "Media application: ";
                      switch (mediaApplication) {
                        case 0:
                          tmpStr += "Reserved";
                          break;
                        case 1:
                          tmpStr += "Voice";
                          break;
                        case 2:
                          tmpStr += "Voice Signaling";
                          break;
                        case 3:
                          tmpStr += "Guest Voice";
                          break;
                        case 4:
                          tmpStr += "Guest Voice Signaling";
                          break;
                        case 5:
                          tmpStr += "Softphone Voice";
                          break;
                        case 6:
                          tmpStr += "Video Conferencing";
                          break;
                        case 7:
                          tmpStr += "Streaming Video";
                          break;
                        case 8:
                          tmpStr += "Video Signaling";
                          break;
                        default:
                          tmpStr += "unknown";
                          break;
                      }

                      uint32_t tmp32 = (lldpData[lldpDataIndex + 1] << 16) | (lldpData[lldpDataIndex + 2] << 8) | lldpData[lldpDataIndex + 3];
#ifdef DEBUGSERIAL
                      byte mediaPolicyFlag = (tmp32 & 0x800000) >> 23;
#endif
                      byte mediaTagFlag = (tmp32 & 0x400000) >> 22;
                      uint16_t mediaVlanID = (tmp32 & 0x1FFE00) >> 9;
#ifdef DEBUGSERIAL
                      byte mediaL2Prio = (tmp32 & 0x0001C0) >> 6;
                      byte dscp = (tmp32 & 0x00003F);
#endif

#ifdef DEBUGSERIAL
                      Serial.println(tmpStr);
                      Serial.println("Media Policy Flag: " + String(mediaPolicyFlag));
                      Serial.println("Media Tag Flag:    " + String(mediaTagFlag));
                      Serial.println("Media VLan ID:     " + String(mediaVlanID));
                      Serial.println("Media L2 Prio:     " + String(mediaL2Prio));
                      Serial.println("Media l2 DSCP:     " + String(dscp));
#endif
                      if ((mediaTagFlag == 1) && (mediaVlanID != 0)) {
                        lldpinfo.VoiceVLAN[1] = String(mediaVlanID);
                      }
                      break;
                    }  // case 2: Network Policy

                  case 3:  // Location Identification
                    {
                      // complex format, usefull here?
                      // 1 byte: Location data format:
                      //   0: invalid
                      //   1: Coordinate-based LCI => complex calculation, will be skipped
                      //   2: Civic Address LCI
                      //   3: ECS ELIN
                      byte locFormat = lldpData[lldpDataIndex++];
                      switch (locFormat) {
                        case 2:  // Civic Address LCI
                          {
                            byte len = lldpData[lldpDataIndex++];

                            // Address owner
                            byte addressOwnerVal = lldpData[lldpDataIndex++];
                            String addressOwner;
                            if (addressOwnerVal == 0)
                              addressOwner = "Location of the DHCP server";
                            else if (addressOwnerVal == 1)
                              addressOwner = "Location of the network element believed to be closest to the client";
                            else if (addressOwnerVal == 2)
                              addressOwner = "Location of the client";
                            else
                              addressOwner = "Invalid address owner value: " + String(addressOwnerVal);
                            len--;

                            String country = handlelldpAsciiField(lldpData, lldpDataIndex, 2);
                            len -= 2;

                            String addressStr;
                            while (len > 0) {
                              byte addressType = lldpData[lldpDataIndex++];
                              len--;
                              addressStr += handleAddressType(addressType);

                              byte addressLen = lldpData[lldpDataIndex++];
                              len--;
                              addressStr += handlelldpAsciiField(lldpData, lldpDataIndex, addressLen);
                              len -= addressLen;

                              if (len > 0)
                                addressStr += "\n";
                            }  // while( len > 0 )

#ifdef DEBUGSERIAL
                            Serial.println("Address owner: " + addressOwner);
                            Serial.println("Country: " + country);
                            Serial.println("Address:\n" + addressStr);
#endif
                            break;
                          }  // case 2: Civic Address LCI

                        case 3:  // ECS ELIN
                          {
                            String addressStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength - 1);
#ifdef DEBUGSERIAL
                            Serial.println("ECS ELIN Address: " + addressStr);
#endif

                            break;
                          }  // case 3: ECS ELIN

                        case 0:   // invalid
                        case 1:   // Coordinate-based LCI
                        default:  // unknown
                          break;
                      }  // switch( locFormat )
                      break;
                    }  // case 3: Location Identification

                  case 4:  // Extended Power-via-MDI
                    {
                      // 1 byte: Power type (& 0xc0)
                      //          0bxx00 0000 = 0xc0: PD Device or PSE Device
                      //         Power Source
                      //          0b00xx 0000 = 0x30: Power source depending on device
                      //         Power Priority
                      //          0b0000 xxxx = 0x0f
                      //          0b0000 0000 = 0x00: Unknown
                      //          0b0000 0001 = 0x01: Critical
                      //          0b0000 0010 = 0x02: High
                      //          0b0000 0011 = 0x03: Low
                      // 2 byte: Power in 0.1 W steps
                      byte powerType = lldpData[lldpDataIndex];
                      String tmpStr = "Power type: ";
                      if (((powerType & 0xc0) == 0) || ((powerType & 0xc0) == 0x80)) {
                        tmpStr += "Power Source Equipment\n";
                        if ((powerType & 0x10) != 0)
                          tmpStr += "Primary Power Source\n";
                        else if ((powerType & 0x20) != 0)
                          tmpStr += "Backup Power Source\n";
                      } else if (((powerType & 0xc0) == 0x40) || ((powerType & 0xc0) == 0xc0)) {
                        tmpStr += "Powered Device\n";
                        if ((powerType & 0x30) == 0x30)
                          tmpStr += "PSE and Local\n";
                        else if ((powerType & 0x10) != 0)
                          tmpStr += "PSE\n";
                        else if ((powerType & 0x20) != 0)
                          tmpStr += "Local\n";
                      }

                      tmpStr += "Power Priority ";
                      switch (powerType & 0x0f) {
                        case 1:
                          tmpStr += "Critical";
                          break;

                        case 2:
                          tmpStr += "High";
                          break;

                        case 3:
                          tmpStr += "Low";
                          break;

                        default:
                          tmpStr += "Unknown";
                          break;
                      }
#ifdef DEBUGSERIAL
                      Serial.println(tmpStr);
#endif

                      uint16_t powerValue = (lldpData[lldpDataIndex + 1] << 8) | lldpData[lldpDataIndex + 2];
                      String powerValueStr = String((float)(powerValue) / 10.0) + "Wh";
#ifdef DEBUGSERIAL
                      Serial.println("Power value: " + powerValueStr);
#endif
                      break;
                    }  // case 4: Extended Power-via-MDI

                  case 5:  // Inventory - Hardware Revision
                    {
#ifdef DEBUGSERIAL
                      String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
                      Serial.println("Inventory - Hardware Revision: " + tmpStr);
#endif
                      break;
                    }  // case 5: Inventory - Hardware Revision

                  case 6:  // Inventory - Firmware Revision
                    {
#ifdef DEBUGSERIAL
                      String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
                      Serial.println("Inventory - Firmware Revision: " + tmpStr);
#endif
                      break;
                    }  // case 6: Inventory - Firmware Revision

                  case 7:  // Inventory - Software Revision
                    {
#ifdef DEBUGSERIAL
                      String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
                      Serial.println("Inventory - Software Revision: " + tmpStr);
#endif
                      break;
                    }  // case 7: Inventory - Software Revision

                  case 8:  // Inventory - Serial Number
                    {
#ifdef DEBUGSERIAL
                      String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
                      Serial.println("Inventory - Serial Number: " + tmpStr);
#endif
                      break;
                    }  // case 8: Inventory - Serial Number

                  case 9:  // Inventory - Manufacturer Name
                    {
#ifdef DEBUGSERIAL
                      String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
                      Serial.println("Inventory - Manufacturer Name: " + tmpStr);
#endif
                      break;
                    }  // case 9: Inventory - Manufacturer Name

                  case 10:  // Inventory - Model Name
                    {
                      lldpinfo.Model[1] = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
                      ;
#ifdef DEBUGSERIAL
                      Serial.println("Inventory - Model Name: " + lldpinfo.Model[1]);
#endif
                      break;
                    }  // case 10: Inventory - Model Name

                  case 11:  // Inventory - Asset ID
                    {
#ifdef DEBUGSERIAL
                      String tmpStr = handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
                      Serial.println("Inventory - Asset ID: " + tmpStr);
#endif
                      break;
                    }  // case 11: Inventory - Asset ID

                  default:
                    {
                      break;
                    }
                }  // switch( OUIsubType )
                break;
              }  //case 0x0012bb: TIA TR-41 Committee - Media Endpoint Discovery (LLDP-MED, ANSI/TIA-1057)

              // #######################################################

            case 0x0080c2:  // IEEE 802.1
              {
                switch (OUIsubType) {
                  // LLDP specification defines the following set of IEEE 802.1 Organizationally Specific TLVs reference(02-Dec-2011):
                  // Port VLAN ID TLV (OUI = 00-80-c2, Subtype = 1): 2 byte
                  // Port And Protocol VLAN ID TLV (OUI = 00-80-c2, Subtype = 2): 1 + 2 byte
                  // VLAN Name TLV (OUI = 00-80-c2, Subtype = 3): 2 byte length + length bytes
                  // Protocol Identity (OUI = 00-80-c2, Subtype = 4):
                  // VID Usage Digest (OUI = 00-80-c2, Subtype = 5)
                  // Management VID (OUI = 00-80-c2, Subtype = 6)
                  // Link Aggregation (OUI = 00-80-c2, Subtype = 7)
                  // Congestion Notification (OUI = 00-80-c2, Subtype = 8)
                  // ETS Configuration TLV (OUI = 00-80-c2, Subtype = 9)
                  // ETS Recommendation TLV (OUI = 00-80-c2, Subtype = A)
                  // Priority-based Flow Control Configuration TLV (OUI = 00-80-c2, Subtype = B )
                  // Application Priority TLV (OUI = 00-80-c2, Subtype = C)
                  // EVB TLV (OUI = 00-80-c2, Subtype = D)
                  // CDCP TLV (OUI = 00-80-c2, Subtype = E)
                  // Port extension TLV (OUI = 00-80-c2, Subtype = F)
                  case 1:  // Port VLAN ID TLV
                    {
#ifdef DEBUGSERIAL
                      uint16_t vlanID = (lldpData[lldpDataIndex + 0] << 8) | lldpData[lldpDataIndex + 1];
#endif
                      lldpinfo.VLAN[1] = lldp_handleCdpNumField(lldpData, lldpDataIndex, 2);
#ifdef DEBUGSERIAL
                      Serial.println("Port VLAN ID: " + String(vlanID) + "/" + lldpinfo.VLAN[1]);
#endif
                      break;
                    }  // case 1: Port VLAN ID TLV

                  case 2:  // Port And Protocol VLAN ID
                    {
#ifdef DEBUGSERIAL
                      uint8_t port = lldpData[lldpDataIndex + 0];
                      uint16_t protvlanID = (lldpData[lldpDataIndex + 1] << 8) | lldpData[lldpDataIndex + 2];
                      Serial.println("Port: " + String(port));
                      Serial.println("Protocol VLAN ID: " + String(protvlanID));
#endif
                      break;
                    }  // case 2: Port And Protocol VLAN ID

                  case 3:  // VLAN Name TLV (OUI = 00-80-c2, Subtype = 3): 2 byte length + length bytes
                    {
#ifdef DEBUGSERIAL
                      uint16_t nameLength = (lldpData[lldpDataIndex + 0] << 8) | lldpData[lldpDataIndex + 1];
                      Serial.println("VLAN name: " + handlelldpAsciiField(lldpData, 2, nameLength));
#endif
                      break;
                    }  // case 3: VLAN Name TLV

                  default:
                    {
#ifdef DEBUGSERIAL
                      Serial.println("\n\nLLDP custom type OUI:     " + String(OUI, HEX));
                      Serial.println("LLDP custom type subtype: " + String(OUIsubType, HEX));
                      Serial.println("LLDP custom data length:  " + String(lldpFieldLength, DEC));
                      Serial.println("lldp custom field:");
                      for (uint16_t i = 0; i < lldpFieldLength; i++) {
                        String data = "00" + String(lldpData[lldpDataIndex + i], HEX);
                        data = "0x" + data.substring(data.length() - 2);
                        Serial.print(data + " ");
                      }
                      Serial.println("\n\n");
#endif
                      break;
                    }
                }  // switch( OUIsubType )
                break;
              }  // case 0x0080c2: IEEE 802.1

              // #######################################################

            case 0x30b216:
              {
                // The LLDP specification defines the following set of Hytec Organizationally Specific TLVs (Homepage: www.hytec.de, protocol documentation: HYTEC):
                // Transceiver TLV (OUI = 30-B2-16, Subtype = 1)
                // Trace TLV (OUI = 30-B2-16, Subtype = 2)
                //#ifdef DEBUGSERIAL
                //            Serial.println( "LLDP custom type OUI: 0x30b216" );
                //#endif
                break;
              }  // case 0x30b216:

            default:
              {
#ifdef DEBUGSERIAL
                Serial.println("\n\nLLDP custom type OUI:     " + String(OUI, HEX));
                Serial.println("LLDP custom type subtype: " + String(OUIsubType, HEX));
                Serial.println("LLDP custom data length:  " + String(lldpFieldLength, DEC));
                Serial.println("lldp custom field:");
                for (uint16_t i = 0; i < lldpFieldLength; i++) {
                  String data = "00" + String(lldpData[lldpDataIndex + i], HEX);
                  data = "0x" + data.substring(data.length() - 2);
                  Serial.print(data + " ");
                }
                Serial.println("\n\n");
#endif
                break;
              }  // default
          }      // switch( OUI )
          break;
        }  // case 0x007f: // Custom TLVs

        // #######################################################

      default:
        {
#ifdef DEBUGSERIAL
          Serial.println("LLDP unhandled type: 0x" + String(lldpFieldType, HEX));
          Serial.println("LLDP field length:   " + String(lldpFieldLength, DEC));
#endif
          break;
        }
    }  // switch( lldpFieldType )
    lldpDataIndex += lldpFieldLength;
  }  // while( lldpDataIndex < plen )

  return lldpinfo;
}

String handleLLDPIPField(const byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;
  String temp;
  for (unsigned int i = offset; i < (offset + lengtha); ++i, ++j) {
    temp += String(a[i], DEC);
    if (j < 3)
      temp += ".";
  }
  temp += '\0';
  return temp;
}

String handlelldpAsciiField(byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;
  char temp[lengtha + 1];
  for (unsigned int i = offset; i < (offset + lengtha); ++i, ++j)
    temp[j] = a[i];

  temp[lengtha] = '\0';
  String temp1 = temp;
  return temp1;
}

void handlelldpCapabilities(const byte a[], unsigned int offset, unsigned int lengtha) {
  int j = 0;
  String temp;

  for (unsigned int i = offset; i < (offset + lengtha); ++i, ++j)
    temp = temp + lldp_print_binary(a[i], 8);

  lldpinfo.Cap[1] = LldpCapabilities(temp);
}

String lldp_print_binary(int v, int num_places) {
  String output;
  int mask = 0, n;
  for (n = 1; n <= num_places; n++)
    mask = (mask << 1) | 0x0001;
  v = v & mask;  // truncate v to specified number of places

  while (num_places) {
    if (v & ((0x0001 << num_places) - 1))
      output = output + "1";
    else
      output = output + "0";

    --num_places;
  }
  return output;
}

String LldpCapabilities(String temp) {
  // Capabilities
  // OTHER     0x0001 = 0b0000 0000 0000 0001
  // REPEATER  0x0002 = 0b0000 0000 0000 0010
  // BRIDGE    0x0004 = 0b0000 0000 0000 0100
  // WLAN      0x0008 = 0b0000 0000 0000 1000
  // ROUTER    0x0010 = 0b0000 0000 0001 0000
  // TELEPHONE 0x0020 = 0b0000 0000 0010 0000
  // DOCSIS    0x0040 = 0b0000 0000 0100 0000
  // STATION   0x0080 = 0b0000 0000 1000 0000
  // CVLAN     0x0100 = 0b0000 0001 0000 0000
  // SVLAN     0x0200 = 0b0000 0010 0000 0000
  // TPMR      0x0400 = 0b0000 0100 0000 0000

  String output;
  if (temp.substring(15, 16) == "1")
    output = output + "Other ";
  if (temp.substring(14, 15) == "1")
    output = output + "Repeater ";
  if (temp.substring(13, 14) == "1")
    output = output + "Bridge ";
  if (temp.substring(12, 13) == "1")
    output = output + "WLAN ";
  if (temp.substring(11, 12) == "1")
    output = output + "Router ";
  if (temp.substring(10, 11) == "1")
    output = output + "Telephone ";
  if (temp.substring(9, 10) == "1")
    output = output + "DOCSIS ";
  if (temp.substring(8, 9) == "1")
    output = output + "Station ";
  if (temp.substring(7, 8) == "1")
    output = output + "CVLAN ";
  if (temp.substring(6, 7) == "1")
    output = output + "SVLAN ";
  if (temp.substring(5, 6) == "1")
    output = output + "TMPR ";
  return output;
}

String lldp_print_mac(const byte a[], unsigned int offset, unsigned int length) {
  String Mac;
  for (unsigned int i = offset; i < offset + length; ++i) {
    if (a[i] < 0x10)
      Mac = Mac + '0';

    Mac = Mac + String(a[i], HEX);
  }

  return Mac;
}

String lldp_handleCdpNumField(const byte a[], unsigned int offset, unsigned int length) {
  String temp;
  unsigned long num = 0;
  for (unsigned int i = 0; i < length; ++i) {
    num <<= 8;
    num += a[offset + i];
  }
  temp = "" + String(num, DEC);
  return temp;
}

String handleChassisSubtype(byte lldpData[], unsigned int lldpDataIndex, unsigned int lldpFieldLength) {
  lldpFieldLength = lldpFieldLength - 1;
  unsigned int charTemp = lldpData[lldpDataIndex];
  lldpDataIndex++;

  switch (charTemp) {
    case 0x0000:
      return "Reserved";
      break;

    case 0x0001:
      return "Chassis component";
      break;

    case 0x0002:
      return "Interface alias";
      break;

    case 0x0003:
      return "Port component";
      break;

    case 0x0004:
      return "MAC address";
      break;

    case 0x0005:
      return "Network address";
      break;

    case 0x0006:
      return "Interface name";
      break;

    case 0x0007:
      return "Locally assigned";
      break;
  }
  return " ";
}

String handlePortSubtype(byte lldpData[], unsigned int lldpDataIndex, unsigned int lldpFieldLength) {
  lldpFieldLength = lldpFieldLength - 1;
  unsigned int charTemp = lldpData[lldpDataIndex];
  lldpDataIndex++;

  /*
        https://docs.zephyrproject.org/latest/reference/kconfig/CONFIG_NET_LLDP_PORT_ID_SUBTYPE.html
        Subtype 1 = Interface alias
        Subtype 2 = Port component
        Subtype 3 = MAC address
        Subtype 4 = Network address
        Subtype 5 = Interface name
        Subtype 6 = Agent circuit ID
        Subtype 7 = Locally assigned
  */
  switch (charTemp) {
    case 0x0001:
      // ASCII
      return handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
      break;

    case 0x0002:
      // ASCII
      return handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
      break;

    case 0x0003:
      // MAC Address
      return lldp_print_mac(lldpData, lldpDataIndex, lldpFieldLength);
      break;

    case 0x0004:
      // IP Address
      return handleLLDPIPField(lldpData, lldpDataIndex, lldpFieldLength);
      break;

    case 0x0005:
      // ASCII
      return handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
      break;

    case 0x0006:
      // ??
      break;

    case 0x0007:
      // ASCII ??
      return handlelldpAsciiField(lldpData, lldpDataIndex, lldpFieldLength);
      break;
  }
  return " ";
}

String handleManagementSubtype(byte lldpData[], unsigned int lldpDataIndex, unsigned int lldpFieldLength) {
  unsigned int SubtypeLength = lldpData[lldpDataIndex] - 1;
  lldpDataIndex++;
  unsigned int charTemp = lldpData[lldpDataIndex];
  lldpDataIndex++;
  /*
     https://github.com/boundary/wireshark/blob/master/epan/dissectors/packet-lldp.c
        Subtype 1 = IPv4
        Subtype 2 = IPv6
        Subtype [Other] = MAC
  */
  switch (charTemp) {
    case 0x0001:
      // IPv4
      return handleLLDPIPField(lldpData, lldpDataIndex, SubtypeLength);
      break;

    case 0x0002:
      // IPv6
      //return handleLLDPIPv6Field( lldpData, lldpDataIndex, SubtypeLength );
      break;

    default:
      // MAC
      return lldp_print_mac(lldpData, lldpDataIndex, SubtypeLength);
      break;
  }
  return " ";
}

String handleAddressType(byte addressType) {
  String address;
  if (addressType == 0)
    address = F("Language");
  else if (addressType == 1)
    address = F("National subdivisions (province, state, etc)");
  else if (addressType == 2)
    address = F("County, parish, district");
  else if (addressType == 3)
    address = F("City, township");
  else if (addressType == 4)
    address = F("City division, borough, ward");
  else if (addressType == 5)
    address = F("Neighborhood, block");
  else if (addressType == 6)
    address = F("Street");
  else if (addressType == 16)
    address = F("Leading street direction");
  else if (addressType == 17)
    address = F("Trailing street suffix");
  else if (addressType == 18)
    address = F("Street suffix");
  else if (addressType == 19)
    address = F("House number");
  else if (addressType == 20)
    address = F("House number suffix");
  else if (addressType == 21)
    address = F("Landmark or vanity address");
  else if (addressType == 22)
    address = F("Additional location information");
  else if (addressType == 23)
    address = F("Name");
  else if (addressType == 24)
    address = F("Postal/ZIP code");
  else if (addressType == 25)
    address = F("Building");
  else if (addressType == 26)
    address = F("Unit");
  else if (addressType == 27)
    address = F("Floor");
  else if (addressType == 28)
    address = F("Room number");
  else if (addressType == 29)
    address = F("Place type");
  else if (addressType == 128)
    address = F("Script");
  else
    address = "Unknown address type (" + String(addressType) + ")";
  address += ": ";
  return address;
}

// Send a currently very static LLDP-MED packet to the LLDP broadcast address to. This should
// trigger the switch / router to send itself a LLDP-MED packet.
void send_LLDP_MED(uint16_t buffersize, uint16_t voiceVLAN, unsigned long* lastLLDPsent, byte mymac[]) {
  // LLDP-MED packet
  const byte packet_LLDP_MED[] = {
    0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e,                                                  // [  6] 6 byte LLDP broadcast address
    0xca, 0xfe, 0xc0, 0xff, 0xee, 0x00,                                                  // [ 12] 6 byte own MAC address
    0x88, 0xcc,                                                                          // [ 14] 2 byte Ethernet Frame type
    0x02, 0x06, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00,                                      // [ 22] 7 bit type=1 (Chassis ID), 9 bit len=6, subtype 5, Value 0x0100000000
    0x04, 0x07, 0x03, 0xca, 0xfe, 0xc0, 0xff, 0xee, 0x00,                                // [ 31] 7 bit type=2 (Port), 9 bit len=7, 1 byte port type=MAC, 6 byte own MAC
    0x06, 0x02, 0x00, 0xb4,                                                              // [ 35] 7 bit type=3 (TTL), 9 bit len=2, TTL=0x00b4=180s
    0x0a, 0x05, 0x44, 0x41, 0x4D, 0x50, 0x46,                                            // [ 42] 7 bit type=5 (Device name), 9 bit len=5, "DAMPF"
    0x0c, 0x05, 0x44, 0x41, 0x4D, 0x50, 0x46,                                            // [ 49] 7 bit type=6 (Model name), 9 bit len=5, "DAMPF"
    0x0e, 0x04, 0x00, 0x20, 0x00, 0x20,                                                  // [ 55] 7 bit type=7 (System Capabilities), 9 bit len=4, 0x0020=Phone 0x0020=Phone
    0x08, 0x08, 0x57, 0x41, 0x4e, 0x20, 0x50, 0x4f, 0x52, 0x54,                          // [ 65] 7 bit type=4 (Port Description),  9 bit len=8, "WAN PORT"
    0xfe, 0x09, 0x00, 0x12, 0x0f, 0x01, 0x03, 0x6c, 0x00, 0x00, 0x10,                    // [ 76] 7 bit type=127, 9 bit len=9, OUI=0x00120f, subtype=1 (MAC/PHY), AutoNeg en/sup, 0x6c00=phy, 0x0010 MAU
    0xfe, 0x0c, 0x00, 0x12, 0x0f, 0x02, 0x00, 0x02, 0x03, 0xf3, 0x00, 0x32, 0x00, 0x00,  // ...[ 90] 7 bit type=127, 9 bit len=7, OUI=0x00120f, subtype=2 (MDI Power Support), 0x00:MDI Power support:PD Device, 0x02:Spare pairs, 0x03:Power Class 2, 0x74:Type/Source/Prio, 0x0032: PD 5.0W
    0xfe, 0x07, 0x00, 0x12, 0xbb, 0x01, 0x00, 0x33, 0x03,                                // [ 99] 7 bit type=127, 9 bit len=7, OUI=0x0012bb, subtype=1 (LLDP-MED Capabilities), 0x0033 (LLPD-MED Capabilities,Network policy,Extended Power via MDI-PD,Inventory), 0x03 Endpoint Class III
    0xfe, 0x08, 0x00, 0x12, 0xbb, 0x02, 0x01, 0x80, 0x00, 0x00,                          // [109] 7 bit type=127, 9 bit len=8, OUI=0x0012bb, subtype=2 (Network Policy), App 1=Voice, 0x8000, 0x00=Media policy flag
    0xfe, 0x07, 0x00, 0x12, 0xbb, 0x04, 0x52, 0x00, 0x32,                                // [118] 7 bit type=127, 9 bit len=7, OUI=0x0012bb, subtype=4 (Extended Power-via-MDI), 0x52 (Powered Device,Primary Power Source,Power Prio High), 0x0026=3,8W request
    0xfe, 0x07, 0x00, 0x12, 0xbb, 0x05, 0x31, 0x2e, 0x30,                                // [127] 7 bit type=127, 9 bit len=7, OUI=0x0012bb, subtype=5 (Inventory - Hardware Revision), "1.0"
    0xfe, 0x07, 0x00, 0x12, 0xbb, 0x06, 0x31, 0x2e, 0x36,                                // [136] 7 bit type=127, 9 bit len=7, OUI=0x0012bb, subtype=6 (Inventory - Firmware Revision), "1.6"
    0xfe, 0x07, 0x00, 0x12, 0xbb, 0x07, 0x31, 0x2e, 0x36,                                // [145] 7 bit type=127, 9 bit len=7, OUI=0x0012bb, subtype=7 (Inventory - Software Revision), "1.6"
    0xfe, 0x10, 0x00, 0x12, 0xbb, 0x08,                                                  // [151] 7 bit type=127, 9 bit len=16, OUI=0x0012bb, subtype=8 (Inventory - Serial Number)
    0x63, 0x61, 0x66, 0x65, 0x63, 0x30, 0x66, 0x66, 0x65, 0x65, 0x30, 0x30,              // [163] "cafec0ffee00"
    0xfe, 0x07, 0x00, 0x12, 0xbb, 0x09, 0x44, 0x49, 0x59,                                // [172] 7 bit type=127, 9 bit len=7, OUI=0x0012bb, subtype=9 (Inventory - Manufacturer Name), "DIY"
    0xfe, 0x09, 0x00, 0x12, 0xbb, 0x0a, 0x44, 0x41, 0x4d, 0x50, 0x46,                    // [182] 7 bit type=127, 9 bit len=9, OUI=0x0012bb, subtype=10 (Inventory - Model Name), "DAMPF"
  };

  // Check size of data structure and copy packet to send buffer.
  uint16_t len = sizeof(packet_LLDP_MED);
  if (len < buffersize) {
    memset(Ethernet::buffer, 0, buffersize);
    memcpy_P(Ethernet::buffer, packet_LLDP_MED, len);
  }

#ifdef DEBUGSENDLLDP
  // Copy Ethernet buffer to input buffer and check if the LLDP data is valid
  uint16_t plen = buffersize;
  memcpy(eth_buffcheck, Ethernet::buffer, plen);
#ifdef DEBUGSERIAL
  Serial.println("Check DAMF-lldp():");
#endif

  unsigned int lldp_correct = lldp_check_Packet(eth_buffcheck, plen);
  if (lldp_correct > 1) {
#ifdef DEBUGSERIAL
    Serial.println("lldp_correct=" + String(lldp_correct));
#endif
    eth_lldpPacket = lldp_packet_handler(eth_buffcheck, plen);
  }
#endif

  bool wastagged = false;
  if (ENC28J60::is_VLAN_tagging_enabled()) {
    ENC28J60::disable_VLAN_tagging();
    wastagged = true;
  }
  ether.packetSend(len);
  if (wastagged) {
    ENC28J60::enable_VLAN_tagging(voiceVLAN);
  }

  *lastLLDPsent = millis();

  // Clear buffer
  memset(Ethernet::buffer, 0, buffersize);
}
