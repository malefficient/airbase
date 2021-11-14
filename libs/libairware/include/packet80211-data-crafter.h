#ifndef _PACKET80211_data_CRAFTER_H
#define _PACKET80211_data_CRAFTER_H

#include "datatypes.h"
#include "packet80211-data.h"
#include <pcap.h>
#include <string>
using namespace std;
using std::string;


class Packet_80211_data_Crafter
{
	public:
	u_int8_t dst[6];
	u_int8_t src[6];
	u_int8_t bss[6];

	Packet_80211_data_Crafter();
	Packet_80211_data_Crafter( u_int8_t *da, u_int8_t *sa, u_int8_t * Bss);

	bool craft(Packet_80211_data &P);
};





#endif

