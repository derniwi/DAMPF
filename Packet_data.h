/*
Packet_data.h

Hold received information from LLDP or CDP (and possibly other protocols in future) for displaying.

2023-12-18: Initial version
*/

#include <EtherCard.h>
#include <Arduino.h>

#ifndef PACKET_DATA_H
#define PACKET_DATA_H

struct PINFO {
  String ChassisID[2] = { "ChassisID", "-" };
  String Proto[2] = { "Proto", "-" };
  String ProtoVer[2] = { "ProtoVer", "-" };
  String SWName[2] = { "Name", "-" };
  String SWDomain[2] = { "Domain", "-" };
  String MAC[2] = { "MAC", "-" };
  String Port[2] = { "Port", "-" };
  String PortDesc[2] = { "PortDesc", "-" };
  String Model[2] = { "Model", "-" };
  String VLAN[2] = { "VLAN", "-" };
  String IP[2] = { "IP", "-" };
  String VoiceVLAN[2] = { "VoiceVLAN", "-" };
  String Cap[2] = { "Cap", "-" };
  String SWver[2] = { "SWver", "-" };
  String TTL[2] = { "TTL", "-" };
  String VTP[2] = { "VTP", "-" };
  String Dup[2] = { "Dup", "-" };
  String PoEAvail[2] = { "PoE avail", "-" };
  String PoECons[2] = { "PoE cons", "-" };
  String MgmtIP[2] = { "MgmtIP", "-" };
  String MgmtVLAN[2] = { "MgmtVLAN", "-" };
  String Checksum[2] = { "Checksum", "-" };
};

#endif
