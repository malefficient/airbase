#include "packet80211-ctrl-crafter.h"
#include "datatypes.h"
#include "jc-util.h"


Packet_80211_ctrl_Crafter :: Packet_80211_ctrl_Crafter()
{
	memset(RA, 0, 6);
	memset(TA, 0, 6);
	memset(BSS, 0, 6);
}
Packet_80211_ctrl_Crafter :: Packet_80211_ctrl_Crafter(u_int8_t *_RA, u_int8_t *_TA, u_int8_t *_BSS)
{
	memcpy(RA, _RA, 6);
	memcpy(TA, _TA, 6);
	memcpy(BSS, _BSS, 6);
}

bool Packet_80211_ctrl_Crafter :: craft(u_int8_t subtype, Packet_80211_ctrl &P)
{

	struct ieee80211 hdr;
	memset((void *) &hdr, 0, sizeof(struct ieee80211));
	hdr.version = 0;
	hdr.type = Ctrl80211Type;; //ctrl
	hdr.subtype = subtype; 

	u_int32_t size = control_packet_correct_size(subtype);

	Pcap_Packet PcapPacket;
	PcapPacket.Init(size,(u_int8_t *)  &hdr, DLT_IEEE802_11);
	if (! PcapPacket.is_initialized())
	{
		return false;
	}

	Packet_80211 currPacket;
	currPacket.Init(PcapPacket);
	if (! currPacket.is_initialized())
		return false;

	P.Init(currPacket);
	if (! P.is_initialized())
		return false;
	Packet_80211_ctrl &CntrlPkt = P; //incase this changed to accept Packet_80211 as paramter.
	//Some of these will harmlessly fail.
	CntrlPkt.setRecvAddr(RA);
	CntrlPkt.setTA(TA);
	CntrlPkt.setBssId(BSS);
	
	return true;

}

