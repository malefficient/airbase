#ifndef _PACKET80211_MGMT_H
#define _PACKET80211_MGMT_H

#include "datatypes.h"
#include "packet80211.h"
#include <pcap.h>
#include <string>
using namespace std;
using std::string;


enum mgmt_subtype {Assoc_Req = 0, Assoc_Res = 1, Reassoc_Req =  2, Reassoc_Res = 3, Probe_Req = 4, Probe_Res = 5, 
				   Beacon = 8, Atim = 9, Disassoc = 10, Auth = 11, Deauth = 12, Action = 13};

class Packet_80211_mgmt: public Packet_80211
{

	public:
	string toString(bool recurse = true);

	Packet_80211_mgmt();

	Packet_80211_mgmt(const Packet_80211 &base);
	void Init(const Packet_80211 &base);


	private:
	
	public:
	void print();
	void print(ostream &s);
	
};


bool is_reserved_mgmt_subtype(u_int8_t subtype);

int mgmt_packet_min_size(u_int8_t subtype);
string mgmt_subtype_to_string(u_int8_t subtype);
#endif

