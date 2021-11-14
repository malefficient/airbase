#ifndef _PACKET80211_CRAFTER_H
#define _PACKET80211_CRAFTER_H

#include "datatypes.h"
#include "packet80211.h"
#include "packet80211-data-crafter.h"
#include "packet80211-mgmt-crafter.h"
#include "packet80211-ctrl-crafter.h"
#include <pcap.h>
#include <string>
using namespace std;
using std::string;


class Packet_80211_Crafter
{
	public: //no harm in letting user change these. 
	Packet_80211_data_Crafter data_crafter;
	Packet_80211_ctrl_Crafter ctrl_crafter;
	Packet_80211_mgmt_Crafter mgmt_crafter;
	public:

	Packet_80211_Crafter( Packet_80211_data_Crafter &data_crafter, 
						  Packet_80211_ctrl_Crafter &ctrl_crafter, 
						  Packet_80211_mgmt_Crafter &mgmt_crafter);

	bool craft(u_int8_t, u_int8_t subtype, Packet_80211 &P);
};





#endif

