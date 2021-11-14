#ifndef _PACKET80211_CTRL_CRAFTER_H
#define _PACKET80211_CTRL_CRAFTER_H

#include "datatypes.h"
#include "packet80211-ctrl.h"
#include <pcap.h>
#include <string>
using namespace std;
using std::string;


class Packet_80211_ctrl_Crafter
{
	public:
	u_int8_t RA[6];
	u_int8_t TA[6];
	u_int8_t BSS[6];

	Packet_80211_ctrl_Crafter();
	Packet_80211_ctrl_Crafter( u_int8_t *RA, u_int8_t *TA, u_int8_t * Bss);

	bool craft(u_int8_t subtype, Packet_80211_ctrl &P);
};





#endif

