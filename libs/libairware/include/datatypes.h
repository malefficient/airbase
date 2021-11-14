#ifndef DATATYPES_H
#define DATATYPES_H
#include "typedefs.h"
#include <pcap.h>
#include <list>
#include <vector>
using std::list;
using std::vector;
//This is joshwrights better struct.
struct ieee80211 {
#if BYTE_ORDER == LITTLE_ENDIAN
    u8    version:2;
    u8    type:2;
    u8    subtype:4;
#elif BYTE_ORDER == BIG_ENDIAN
    u8    subtype:4;
    u8    type:2;
    u8    version:2;
#else
#error "Please fix <bits/endian.h"
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
    u8    to_ds:1;
    u8    from_ds:1;
    u8    more_frag:1;
    u8    retry:1;
    u8    pwrmgmt:1;
    u8    more_data:1;
    u8    wep:1;
    u8    order:1;
#elif BYTE_ORDER == BIG_ENDIAN
    u8    order:1;
    u8    wep:1;
    u8    more_data:1;
    u8    pwrmgmt:1;
    u8    retry:1;
    u8    more_frag:1;
    u8    from_ds:1;
    u8    to_ds:1;
#else
#error "Please fix <bits/endian.h"
#endif
   
    u16   duration;
    u8    addr1[6];
    u8    addr2[6];
    u8    addr3[6];
    u16   fragment:4;
    u16   sequence:12;
} __attribute__ ((packed));
/*
 * Currently we don't try to look inside the Qos header. Later
 * when are actually parsing this (instead of just skipping over it)
 * we will need to define out the individual bits
 */

struct ieee80211QoS
{
	u8 b1;
	u8 b2;
} __attribute__ ((packed));

/*
struct ieee80211 
{
    u8    version:2;
    u8    type:2;
    u8    subtype:4;
    u8    to_ds:1;
    u8    from_ds:1;
    u8    more_frag:1;
    u8    retry:1;
    u8    pwrmgmt:1;
    u8    more_data:1;
    u8    wep:1;
    u8    order:1;
    u16   duration;
    u8    addr1[6];
    u8    addr2[6];
    u8    addr3[6];
    u16   fragment:4;
    u16   sequence:12;
} __attribute__ ((packed));
*/

//
//ivs files really have no endian issues.
//the only endianized thing in there is the
//magic number. this is just a hacky way of
//making everything happy.
#if BYTE_ORDER == LITTLE_ENDIAN
#define IVFILE_MAGIC 0xd484cabf
#elif BYTE_ORDER == BIG_ENDIAN
#define IVFILE_MAGIC 0xbfca84d4
#else
#error "Please fix bit bits/endian.h"
#endif
//A note on the IV Class: 
//It includes the first 2 bytes of output because many statistical attacks
//can use these. This same class however is used throughout the code for normal
//IV duties. It kind of pulls double duty and therefore there are some unfortunate
//semantic results. Calling Packet.SetIV() Will set the IV but it will not set the first
//two output bytes. Similliarly the < and == operators concern themselves with the 
//IV only. Sorry for any confusion. It was either this or have 2 diff IV classes floating
//around. 
class IV
{
	public:
	u_int8_t IV[3];
	u_int8_t keynum;
	u_int8_t b1; //first byte of output. This has NOT been xored with 0xAA to get the keystream.
	u_int8_t b2; //similliar
	//friend bool operator == (const IV & left, const IV & right);
	//bool operator <(const IV &right);
};
	bool operator == (const IV & left, const IV & right);
	bool operator < (const IV & left, const IV & right);


typedef vector<IV> IV_List;

class bssid_data
{
		public:
	u_int8_t bssid[6];
	u_int32_t num_ivs;
	u_int8_t *iv_present_map;
	IV_List IvList;
};
typedef list<bssid_data> BssidData_List;
//
//This is just really forward-looking. Maybe i want to add some 'extra'
//Pcap state someday for internal use. This is where i would put it.
class PcapFileHandle
{
	public:
	char errbuf[PCAP_ERRBUF_SIZE];
	char dlt_name[256];
	u_int32_t dlt; //data link type.	
	pcap_t *p; //Everyone always calls these P for some reason..
};

/////-----------------This is my old, less addressable format. Being phased out to use joshwr1ghts.

struct DLT_IEEE802_11_frame
{
	u_int16_t frame_ctrl;
	u_int16_t duration;
	// it is very tempting to label these as below but it is logically incorrect
	// the addresses swtich position based on the flags.
	u_int8_t addr1[6];
	u_int8_t addr2[6];
	u_int8_t addr3[6];
	
	/*
	u_int8_t dest_addr[6]; //THESE MOVE! see above.
	u_int8_t bssid[6];
	u_int8_t source_addr[6];
	*/
	u_int16_t seqfragno; // Actuall this is the seq & frag #.
};

/*
 *   all data packets have a snap (sub-network access protocol) header that
 *  * isn't entirely definied, but added for ethernet compatibility.
 *  
 */
struct wi_snap_frame 
{
    u_int16_t   wi_dat[3];
    u_int16_t   wi_type;
};

#endif

