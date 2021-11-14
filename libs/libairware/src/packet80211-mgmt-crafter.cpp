#include "packet80211-mgmt-crafter.h"
#include "datatypes.h"
#include "jc-util.h"

Packet_80211_mgmt_Crafter :: Packet_80211_mgmt_Crafter()
{
	memset(dst, 0, 6);
	memset(src, 0, 6);
	memset(bss, 0, 6);
}
Packet_80211_mgmt_Crafter :: Packet_80211_mgmt_Crafter(u_int8_t *_dst, u_int8_t *_src, u_int8_t *_bss)
{
	memcpy(dst, _dst, 6);
	memcpy(src, _src, 6);
	memcpy(bss, _bss, 6);
}

bool Packet_80211_mgmt_Crafter :: craft(u_int8_t subtype, Packet_80211_mgmt &P)
{

	struct ieee80211 hdr;
	memset((void *) &hdr, 0, sizeof(struct ieee80211));
	hdr.version = 0;
	hdr.type = Mgmt80211Type; //0
	hdr.subtype = subtype; 

	u_int32_t size = mgmt_packet_min_size(subtype);

	Pcap_Packet PcapPacket;
	PcapPacket.Init(size,(u_int8_t *)  &hdr, DLT_IEEE802_11);
	if (! PcapPacket.is_initialized())
	{
		printf("Mgmt crafter returning false, couldnt init pcap packet\n");
		return false;
	}

	Packet_80211 currPacket;
	currPacket.Init(PcapPacket);
	if (! currPacket.is_initialized())
	{
		printf("Mgmt crafter returning false, couldnt Init Packet_80211.\n");
		return false;
	}
	

	P.Init(currPacket);
	if (! P.is_initialized())
		return false;
	Packet_80211_mgmt &MgmtPkt = P; //incase this changed to accept Packet_80211 as parameter.
	MgmtPkt.setDstAddr(dst);
	MgmtPkt.setSrcAddr(src);
	MgmtPkt.setBssId(bss);
	
	return true;

}

