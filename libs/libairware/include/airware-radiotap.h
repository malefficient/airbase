#ifndef _AIRWARE_RADIOTAP_H
#define _AIRWARE_RADIOTAP_H
#include "datatypes.h"
struct ieee80211_radiotap_header {
        u_int8_t        it_version;     /* set to 0 */
        u_int8_t        it_pad;
        u_int16_t       it_len;         /* entire length */
        u_int32_t       it_present;     /* fields present */
} __attribute__((__packed__));

// for insight into the values of these fields see madwifi/net80211/ieee80211_monitor.h
//the following three tables are aligned arrays for describing order/names/sizes of radiotap fields
enum ieee80211_radiotap_type {
    IEEE80211_RADIOTAP_TSFT = 0,
    IEEE80211_RADIOTAP_FLAGS = 1,
    IEEE80211_RADIOTAP_RATE = 2,
    IEEE80211_RADIOTAP_CHANNEL = 3,
    IEEE80211_RADIOTAP_FHSS = 4,
    IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5,
    IEEE80211_RADIOTAP_DBM_ANTNOISE = 6,
    IEEE80211_RADIOTAP_LOCK_QUALITY = 7,
    IEEE80211_RADIOTAP_TX_ATTENUATION = 8,
    IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9,
    IEEE80211_RADIOTAP_DBM_TX_POWER = 10,
    IEEE80211_RADIOTAP_ANTENNA = 11,
    IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12,
    IEEE80211_RADIOTAP_DB_ANTNOISE = 13,
    IEEE80211_RADIOTAP_RX_FLAGS = 14,
    IEEE80211_RADIOTAP_TX_FLAGS = 15,
    IEEE80211_RADIOTAP_RTS_RETRIES = 16,
    IEEE80211_RADIOTAP_DATA_RETRIES = 17,
    IEEE80211_RADIOTAP_EXT = 31,
};

static u_int32_t radiotap_bit_to_size[] = {
   	8, //"IEEE80211_RADIOTAP_TSFT",				//0
    1, //"IEEE80211_RADIOTAP_FLAGS", 			//1
    1, //"IEEE80211_RADIOTAP_RATE",				//2
    4, //"IEEE80211_RADIOTAP_CHANNEL",			//2
    2, //"IEEE80211_RADIOTAP_FHSS",				//3	
    1, //"IEEE80211_RADIOTAP_DBM_ANTSIGNAL",		//4
    1, //"IEEE80211_RADIOTAP_DBM_ANTNOISE",		//5
    2, //"IEEE80211_RADIOTAP_LOCK_QUALITY",		//6 //barker code lock
    2, //"IEEE80211_RADIOTAP_TX_ATTENUATION",	//7
    2, //"IEEE80211_RADIOTAP_DB_TX_ATTENUATION", //8
    1, //"IEEE80211_RADIOTAP_DBM_TX_POWER",		//9
    1, //"IEEE80211_RADIOTAP_ANTENNA",			//10
    1, //"IEEE80211_RADIOTAP_DB_ANTSIGNAL",		//11
    1, //"IEEE80211_RADIOTAP_DB_ANTNOISE",		//12
    2, //"IEEE80211_RADIOTAP_RX_FLAGS",			//13
    2, //"IEEE80211_RADIOTAP_TX_FLAGS",			//14	
    1, //"IEEE80211_RADIOTAP_RTS_RETRIES",		//15
    1, //"IEEE80211_RADIOTAP_DATA_RETRIES",	    //16
    0, //"IEEE80211_RADIOTAP_UNDEF_17",			//17
    0, //"IEEE80211_RADIOTAP_UNDEF_18",			//18
    0, //"IEEE80211_RADIOTAP_UNDEF_19",			//19
    0, //"IEEE80211_RADIOTAP_UNDEF_20",			//20
    0, //"IEEE80211_RADIOTAP_UNDEF_21",			//21
    0, //"IEEE80211_RADIOTAP_UNDEF_22",			//22
    0, //"IEEE80211_RADIOTAP_UNDEF_23",			//23
    0, //"IEEE80211_RADIOTAP_UNDEF_24",			//24
    0, //"IEEE80211_RADIOTAP_UNDEF_25",			//25
    0, //"IEEE80211_RADIOTAP_UNDEF_26",			//26
    0, //"IEEE80211_RADIOTAP_UNDEF_27",			//27
    0, //"IEEE80211_RADIOTAP_UNDEF_28",			//28
    0, //"IEEE80211_RADIOTAP_UNDEF_29",			//29
    0, //"IEEE80211_RADIOTAP_UNDEF_30",			//30
    0, //"IEEE80211_RADIOTAP_EXT"				//31
};

static const char *radiotap_bit_to_name[] = {
   	"IEEE80211_RADIOTAP_TSFT",				//0
    "IEEE80211_RADIOTAP_FLAGS", 			//1
    "IEEE80211_RADIOTAP_RATE",				//2
    "IEEE80211_RADIOTAP_CHANNEL",			//2
    "IEEE80211_RADIOTAP_FHSS",				//3	
    "IEEE80211_RADIOTAP_DBM_ANTSIGNAL",		//4
    "IEEE80211_RADIOTAP_DBM_ANTNOISE",		//5
    "IEEE80211_RADIOTAP_LOCK_QUALITY",		//6
    "IEEE80211_RADIOTAP_TX_ATTENUATION",	//7
    "IEEE80211_RADIOTAP_DB_TX_ATTENUATION", //8
    "IEEE80211_RADIOTAP_DBM_TX_POWER",		//9
    "IEEE80211_RADIOTAP_ANTENNA",			//10
    "IEEE80211_RADIOTAP_DB_ANTSIGNAL",		//11
    "IEEE80211_RADIOTAP_DB_ANTNOISE",		//12
    "IEEE80211_RADIOTAP_RX_FLAGS",			//13
    "IEEE80211_RADIOTAP_TX_FLAGS",			//14	
    "IEEE80211_RADIOTAP_RTS_RETRIES",		//15
    "IEEE80211_RADIOTAP_DATA_RETRIES",	    //16
    "IEEE80211_RADIOTAP_UNDEF_17",			//17
    "IEEE80211_RADIOTAP_UNDEF_18",			//18
    "IEEE80211_RADIOTAP_UNDEF_19",			//19
    "IEEE80211_RADIOTAP_UNDEF_20",			//20
    "IEEE80211_RADIOTAP_UNDEF_21",			//21
    "IEEE80211_RADIOTAP_UNDEF_22",			//22
    "IEEE80211_RADIOTAP_UNDEF_23",			//23
    "IEEE80211_RADIOTAP_UNDEF_24",			//24
    "IEEE80211_RADIOTAP_UNDEF_25",			//25
    "IEEE80211_RADIOTAP_UNDEF_26",			//26
    "IEEE80211_RADIOTAP_UNDEF_27",			//27
    "IEEE80211_RADIOTAP_UNDEF_28",			//28
    "IEEE80211_RADIOTAP_UNDEF_29",			//29
    "IEEE80211_RADIOTAP_UNDEF_30",			//30
    "IEEE80211_RADIOTAP_EXT"				//31
};

#define MAX_RTAP_SIZE 64 //asuming a fully filled out header, will all values
#define MAX_RTAP_DEFINED_FIELDS 32
#define MAX_RTAP_INDIVIDUAL_FIELD_SIZE 8 //max size of any individual field. currently the timestamp
class radiotap_header
{
	private:
	bool initialized;
	u_int8_t radiotap_data[MAX_RTAP_SIZE];
	struct ieee80211_radiotap_header *rtap_ptr;
	u_int32_t offset_to_field[MAX_RTAP_DEFINED_FIELDS]; //dynamically filled in on init();
	
	
	public:
	bool init(u_int8_t *rtap_hdr);
	//0 on error
	u_int32_t sizeof_field(u_int32_t field_number) { return (field_number > MAX_RTAP_DEFINED_FIELDS ?  0 : radiotap_bit_to_size[field_number]) ;}
	//NULL on error
	const char *nameof_field(u_int32_t field_number) { return (field_number > MAX_RTAP_DEFINED_FIELDS ?  NULL : radiotap_bit_to_name[field_number]) ;}
	bool is_present(u_int32_t field_number);
	//returns number of bytes copied. 0 == error
	u_int32_t retrieve_value(u_int32_t field_num, u_int8_t *dst);
};





#endif


