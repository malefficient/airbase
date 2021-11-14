#ifndef _PACKET80211_CTRL_H
#define _PACKET80211_CTRL_H

#include "datatypes.h"
#include "packet80211.h"
#include <pcap.h>
#include <string>
using namespace std;
using std::string;


enum control_subtype { BlockAckReq = 0x08, BlockAck = 0x09, PS_Poll = 0xa, RTS = 0xb, CTS = 0xc, ACK=0xd, CF_End = 0xe, CF_End_Ack = 0xf};
class Packet_80211_ctrl: public Packet_80211
{

	public:
	string toString(bool recurse = true);

	Packet_80211_ctrl();

	Packet_80211_ctrl(const Packet_80211 &base);
	void Init(const Packet_80211 &base);


	private:
	u_int8_t* findRecvAddr();
	u_int8_t* findTA();
	u_int8_t* findBssId(); //null on error
	
	public:
	void print();
	void print(ostream &s);
	
	//These are not all present in any given ctrlframe. therefore these calls can fail.
	bool getRecvAddr(u_int8_t *ra); //Receiver Address
	bool getTA(u_int8_t *ta); //Transmitter Address
	bool getBssId(u_int8_t *bss); //Bssid.

	bool setRecvAddr(u_int8_t *ra); //Receiver Address
	bool setTA(u_int8_t *da); //Transmitter Address
	bool setBssId(u_int8_t *bss); //Bssid.
	
};


//some control packets have 1 address, others have two. 
//this function returns the 'proper' size for a given subtype (in bytes)
//returns 0 on reserved.

//bool is_valid_ctrl_subtype(u_int8_t subtype);
bool is_reserved_ctrl_subtype(u_int8_t subtype);

int control_packet_correct_size(u_int8_t subtype);
string control_subtype_to_string(u_int8_t subtype);
#endif

