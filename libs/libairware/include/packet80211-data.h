#ifndef _PACKET80211_DATA_H
#define _PACKET80211_DATA_H

#include "datatypes.h"
#include "packet80211.h"
#include <pcap.h>
#include <string>
using namespace std;
using std::string;




/*
enum PacketTypes { ASSOC_REQ, ASSOC_RESP, REASSOC_REQ, REASSOC_RESP, PROBE_REQ, PROBE_RESP,
		   BEACON, ATIM, DISASSOC, AUTH, DEAUTH, 
		   PS, RTS, CTS, ACK, CF_END, CF_END_PLUS_CF_ACK};
*/
class Packet_80211_data: public Packet_80211
{

	public:
	//void print();
	//void print(ostream& s);
	string toString(bool recurse = true);

	Packet_80211_data();

	Packet_80211_data(const Packet_80211 &base);
	void Init(const Packet_80211 &base);

	//PDUDataLength for a wep-free frame is straight-forward.
	//PDUDataLength for a wep'd frame subtracts out the 4 bytes
	//of 'data' eaten up by the WEP parameters.
	//the WEP params can be accessed with functions below. it is NOT returned in the data portion.
	//With that said. the ICV at the end of the packet is returned with the data. 
	int getPDUDataLength();
	//
	//Data is everything After the wlan_header and possible wep header. It starts with the
	//snap header
	void getPDUData(u_int8_t *d);


	
	bool getWepIV(u_int8_t * dest); //copies the 3byte IV.
	bool getWepIV(IV &iv); 			//usees the provided IV class.
	bool getWepKeyNum(u_int8_t *dest);
	bool getWepICV(u_int8_t * dest);
	bool friend UNSAFE_WepDecrypt(Packet_80211_data &EncryptedPacket, Packet_80211_data &DecryptedPacket, 
				u_int8_t *key, u_int8_t keysize);

	bool friend WepDecrypt(Packet_80211_data &EncryptedPacket, Packet_80211_data &Decrypted_Packet,
			        u_int8_t *key, u_int8_t keysize);

};
typedef list<Packet_80211_data> Packet_80211_dataList;
#endif

