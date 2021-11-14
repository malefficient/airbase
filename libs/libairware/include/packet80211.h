#ifndef _PACKET80211_H
#define _PACKET80211_H

#include "datatypes.h"
#include "pcap_packet.h"
#include "airware-radiotap.h"
#include <pcap.h>
#include <string>
using std::string;

//XXX:This will be fixed when the control class is implemented
#define MIN_CONTROL_PKTSIZE 10

static const int Data80211Type = 2;
static const int Ctrl80211Type = 1;
static const int Mgmt80211Type = 0;


//This is a generic mac timestamp struct. It is suitable for
//prism2/avs mac timestamps. It is upto consumers to parse out
//the dlt and the values of the fields. just because these are 16 
//byte fields in ehre doesnt mean all 16 bytes are used/useful.
//know your formats if you plan on using this. lib802finger can
//(hopefully) do something useful with these.
struct mac_time
{
	u_int8_t mac_time[16];
	u_int8_t host_time[16];
	u_int32_t dlt;
};

struct rtap_data
{
	u_int8_t rtap_data[MAX_RTAP_SIZE];
};
class timepiece; //Forward declaration so we can return it;
class Packet_80211: public Pcap_Packet
{


	protected:
	bool initialized;


	
	public:
	struct ieee80211 *wlan_header;
	struct ieee80211QoS *qos_header;
	struct rtap_data radiotap_header; //this is copied 'out-of-bounds' of the real packet buffer. This data does not belong in the stack.
									  //methods that retrieve/manipulate this will be looking into this buffer. It can not be offset
									  //from the 'payload' or data ptr.
	u_int8_t *wlan_fc; //1st byte of wlan header.
	u_int8_t *wlan_flags; //2nd byte of wlan_header
		

	u_int8_t *payload_ptr; //This points past the end of all the header info.

	bool check_flag(u_int8_t flag);
	bool check_fc(u_int8_t flag);
	
	// flags
	u_int8_t version();
	u_int8_t type();
	u_int8_t subtype();

	bool toDS(); //toward AP
	bool fromDS(); //away from AP
	bool more_frag(); 
	bool retry(); 
	bool pwrmgmt(); 
	bool more_data();
	bool wep();
	bool order();

	bool qos(); //This is not a flag, it is encoded in subtype of data frames.


	void set_toDS() {wlan_header->to_ds = 1;}
	void set_fromDS() {wlan_header->from_ds = 1;} 
	void set_more_frag() { wlan_header->more_frag = 1;}
	void set_retry()  { wlan_header->retry = 1;}
	void set_pwrmgmt() { wlan_header->pwrmgmt = 1;}
	void set_more_data(){ wlan_header->more_data = 1;}
	void set_wep() { wlan_header->wep = 1;}
	void set_order() { wlan_header->order = 1;}

	void clear_toDS() {wlan_header->to_ds = 0;}
	void clear_fromDS() {wlan_header->from_ds = 0;} 
	void clear_more_frag() { wlan_header->more_frag = 0;}
	void clear_retry()  { wlan_header->retry = 0;}
	void clear_pwrmgmt() { wlan_header->pwrmgmt = 0;}
	void clear_more_data(){ wlan_header->more_data = 0;}
	void clear_wep() { wlan_header->wep = 0;}
	void clear_order() { wlan_header->order = 0;}
	
	u_int16_t duration();
	//types
	bool isMgmt(); 
	bool isControl(); 
	bool isData(); 
	
	void getWlanHeader( struct DLT_IEEE802_11_frame *p);
	void getWlanHeader( struct ieee80211 *p);

	private:
	bool my_mac_time_available; //set by ctor if it copies out a mac time from a prism/avs hdr.
	struct mac_time my_mac_time;
	u_int8_t *findSrcAddr();
	u_int8_t *findDstAddr();
	u_int8_t *findBssid();
	//no find RecvAddr because its fixed.
	public:
	//These handle the re-arranging madness for you.
 	void getSrcAddr(u_int8_t* dest);
    void getDstAddr(u_int8_t* dest);
    void getBssId(u_int8_t * dest);
    void getRecvAddr(u_int8_t * dest); //Control frames only.
    void getSrcDstBss(u_int8_t *s, u_int8_t *d, u_int8_t *b);

	void getSeqNum(u_int16_t *seqnum); //Not acutally used yet.
	void getFragNum(u_int16_t *seqnum);

	//This looks a little weird because i havent debugged/verifed the Seq/Fragnum stuff yet. 
	//they will prolly end up with ismillar dual interface as Duration.
	u_int16_t getDuration(); // does byte flipping
	void getDuration(u_int8_t *); //just memcpy it in. do no byte translation

	
 	void setSrcAddr(u_int8_t* dest);
    void setDstAddr(u_int8_t* dest);
    void setBssId(u_int8_t * dest);
    void setRecvAddr(u_int8_t * dest); //Control frames only.
    void setSrcDstBss(u_int8_t *s, u_int8_t *d, u_int8_t *b);

	void setSeqNum(u_int16_t seqnum); //does byte flipping
	void setSeqNum(u_int16_t *seqnum);//just memcpy 16 bits in. no byte translation

	void setFragNum(u_int16_t fragnum); //does byte flipping
	void setFragNum(u_int16_t *fragnum);

	void setDuration(u_int16_t dur); //does byte flipping.
	void setDuration(u_int8_t *dur); //just memcpy 16 bits in. Do no byte translation.
	
	public:
	void raw_print();
	void print();
	void print(ostream& s);

	string detail_toString();
	void detail_print();
	void detail_print(ostream &s);
	string toString(bool recurse = true);
	//int initialized() { return (int) initialized;}
	Packet_80211(); 
	
	//Will result in a copy of the data pointed to by p. (see Pcap_Packet ctor)
	Packet_80211(const Pcap_Packet &p);
	bool Init(const Pcap_Packet &p);
	//
	//will make its own copy of data
	Packet_80211(struct pcap_pkthdr p, u_char_t *data, u_int32_t dlt);
	Packet_80211 (const Packet_80211 &right); //copy CTOR. same as = oper
	Packet_80211& operator = (const Packet_80211 &right);

	bool Init(const Packet_80211 &right);
	bool Init(struct pcap_pkthdr p, u_char_t *data, u_int32_t dlt);

	bool is_initialized() { return initialized;}
	void initialized_by_hand() { initialized = true;} //for packet crafters.
	void uninitialized_by_hand() { initialized = false;}
	void check_initialized(char *s);

	//this is a hack to let lib802finger get at accurate prism2 timestamps.
	//whatever it does with them is outside the scope of this library.
	bool mac_time_available() { return my_mac_time_available; }

	//Mark this private soon. dont let people know about struct mac_times?
	bool get_mac_time(struct mac_time *time); //true if mac_time_available true.
	timepiece get_pcap_ts();
	timepiece get_mac_ts(); //Exit on error!
	//--Private helper functions for parsing proprietary prism/avs/etc headers timestamps:
	private:
	bool MacTimeConverter(struct mac_time time, timepiece &ret_timepiece);
	timepiece PrismMacTimeConverter(struct mac_time time);
	
};

typedef list<Packet_80211> Packet_80211List;
typedef list<Packet_80211List>  Packet_80211Chain;
#endif

