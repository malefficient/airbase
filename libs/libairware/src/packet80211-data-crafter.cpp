#include "packet80211-data-crafter.h"
#include "datatypes.h"
#include "jc-util.h"

Packet_80211_data_Crafter :: Packet_80211_data_Crafter()
{
	memset(dst, 0, 6);
	memset(src, 0, 6);
	memset(bss, 0, 6);
}

Packet_80211_data_Crafter :: Packet_80211_data_Crafter(u_int8_t *_dst, u_int8_t *_src, u_int8_t *_bss)
{
	memcpy(dst, _dst, 6);
	memcpy(src, _src, 6);
	memcpy(bss, _bss, 6);
}

bool Packet_80211_data_Crafter :: craft(Packet_80211_data &P)
{

	struct ieee80211 hdr;
	memset((void *) &hdr, 0, sizeof(struct ieee80211));
	hdr.version = 0;
	hdr.type = Data80211Type; // 2
	hdr.subtype = 0; //unused in normal data packets.

	u_int32_t size = sizeof(struct ieee80211); //This is OK for now. may need to make a 
											   //packet_80211_data_min_size (like mgmt/ctrl frames) later

	Pcap_Packet PcapPacket;
	PcapPacket.Init(size,(u_int8_t *)  &hdr, DLT_IEEE802_11);
	if (! PcapPacket.is_initialized())
	{
		printf("Data crafter returning false, couldnt init pcap packet\n");
		return false;
	}

	Packet_80211 currPacket;
	currPacket.Init(PcapPacket);
	if (! currPacket.is_initialized())
	{
		printf("Data crafter returning false, couldnt Init Packet_80211.\n");
		return false;
	}
	

	P.Init(currPacket);
	if (! P.is_initialized())
		return false;
	Packet_80211_data &DataPkt = P; //incase this changed to accept Packet_80211 as parameter.
	DataPkt.setDstAddr(dst);
	DataPkt.setSrcAddr(src);
	DataPkt.setBssId(bss);
	
	return true;

}

