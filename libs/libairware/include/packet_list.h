/* this file just maintains some useful routines for working
 * with Packet_80211Lists's and Packet_80211Lists 
 */
#ifndef _PACKET_LIST_H
#define _PACKET_LIST_H
#include "airware.h"
int GetPacketWindow(Packet_80211List &PktList, Packet_80211List::iterator start, Packet_80211List::iterator& stop, 
				int secs, int u_secs, bool PcapTsOk=false);
int GetPacketWindow(Packet_80211List &PktList, Packet_80211List::iterator start, Packet_80211List::iterator& stop, timepiece duration, bool PcapTsOk = false);

//Returns number packets successfully converted and put into new list.
int PcapPacketListToPacket_80211List(PcapPacketList & PcapList, Packet_80211List &New80211List);

int PcapFileToPcapList(char *fname, PcapPacketList &L);
int PcapFileToPacket_80211List(char *fname, Packet_80211List &List);

int PcapFileHandleToPcapList(PcapFileHandle &, PcapPacketList &L);
int PcapFileHandleToPacket_80211List(PcapFileHandle &, Packet_80211List &List);

Packet_80211List Fragmentify(Packet_80211 P, u_int32_t num_pieces);

#endif
