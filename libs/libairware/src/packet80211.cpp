#include "packet80211.h"
#include "datatypes.h"
#include "jc-util.h"
//#include "print.h"
#include "misc.h"
#include <netinet/in.h> //ntohs 


Packet_80211 :: Packet_80211()
{
	//printf("Packet_80211\n");
	initialized = false;
}

Packet_80211 :: Packet_80211(const Pcap_Packet &p) : Pcap_Packet(p)
{
	//printf("Packet_80211::Packet_80211(Pcap_Packet &)\n");
	initialized = false;
	Init(p);
}

bool Packet_80211 :: Init(const Pcap_Packet &p)
{
	//printf("Packet_80211::Init called (This is the one with the brains\n");
	initialized = false;
	//printf("Packet_80211::Init(Pcap_Packet &)\n");
	
	if (p.length < 0x90 + MIN_CONTROL_PKTSIZE && p.dlt == DLT_PRISM_HEADER)
		return false;

	Pcap_Packet::Init(p); //initalize our base class.

	
	if (dlt == DLT_IEEE802_11_RADIO)  // 802.11 plus radiotap radio header 
	{
		//printf("RADIOTAP parsing enaged...\n");
		if (length < 4)
			return false;
		//TODO: we need to find the union of PRISM and RADIOTAP headers, parse out the meta info
		//(timestamps, signal strength, etc) and make it available similliar to the current 
		//support for prism header mac times..
		u_int16_t offset_to_80211_header;
		memcpy(&offset_to_80211_header, data + 0x02, 2);
#if BYTE_ORDER == BIG_ENDIAN 
		//all radiotap fields are in little endian
		offset_to_80211_header = SWAP2(offset_to_80211_header);
#endif
		if (offset_to_80211_header <= MAX_RTAP_SIZE)
		{
			//printf("radiotap header: copied %d bytes for out of band processing\n", offset_to_80211_header);
			memcpy((void *) &radiotap_header.rtap_data, data, offset_to_80211_header);
		}
		else
		{
			printf("RADIOTAP header to big. it_len = %d, MAX_RTAP_SIZE = %d\n", offset_to_80211_header, MAX_RTAP_SIZE);
			return false;
		}
		//printf("RADIOTAP length: %d\n", offset_to_80211_header);
        memmove(data, data + offset_to_80211_header, length - offset_to_80211_header); //skip past prism header.
        length -= offset_to_80211_header;
        tot_length -=offset_to_80211_header;
        dlt = DLT_IEEE802_11;
	}

	if (dlt == DLT_PRISM_HEADER)
	{
		//printf("Prism header hack engaged.\n");
		if (length < 0x90)
			return false;

		my_mac_time_available = true;
		//This copies the entire prism mac_time field. The first 4 bytes are the timestamp
		//according to ethereal, but other useful stuff follows. Rather than throw them away
		//we include them and allow the consumer to figure out wth to do with any of it.
		memcpy(my_mac_time.host_time, data + 0x18, 12);
		memcpy(my_mac_time.mac_time, data + 0x24, 12);
		my_mac_time.dlt = DLT_PRISM_HEADER;

			
		memmove(data, data + 0x90, length - 0x90); //skip past prism header.
		length -= 0x90;
		tot_length -=0x90;
		dlt = DLT_IEEE802_11;
	}
	if (dlt == DLT_IEEE802_11_RADIO_AVS)
	{
		//printf("AVS hack engaged");
		unsigned long avs_header_size;
		memcpy(&avs_header_size, data + 4, 4);
		avs_header_size = ntohl(avs_header_size);
		//printf("AVS header size %d\n", avs_header_size);
		if (length < avs_header_size) 
			return false;

		my_mac_time_available = true;
		//This copies the mac timestamp field outta the AVS header. It looks like a
		//simple 8 byte counter, as oppsed to the more complicated prism one.
		memcpy(my_mac_time.mac_time, data + 0x08, 8);
		memcpy(my_mac_time.host_time, data + 0x16, 8);
		my_mac_time.dlt = DLT_IEEE802_11_RADIO_AVS;

		memmove(data, data + avs_header_size, length - avs_header_size); //skip AVs header.
		length -= avs_header_size;
		tot_length -= avs_header_size;
		dlt = DLT_IEEE802_11;	
	}

	//We may want to call this based on DLT's if we find one 
	//that alays has (or always doesnt) have a extra FCS.
	//
	//This seems to work OK, So far. (till people start
	//explicitly crafting packets with data packets
	//that coincidentally contain  a FCS in the end of a legit payload
	//just so my lib mangles them.. *sigh*.
	if (BufferHasFCSAlready(data, length))
	{
		//printf("Spurious checksum found!\n");
		length -= 4; //Scrape that off! We dont want one.
		tot_length -=4;
	}

	
	if (dlt != DLT_IEEE802_11)
	{
		printf("Error. Cannot Init Packet_80211 with DLT != DLT_IEEE802_11\n");
		return false;
	}
	

	//move our way up the sanity checks. First check if its the smallest thing we like
	if (length < MIN_CONTROL_PKTSIZE)
	{
			//printf("Base pcap packet to small. Packet_80211::Init returning false\n");
			return false;
	}


	wlan_header = (struct ieee80211 *) data;
	wlan_fc = (u_int8_t*) wlan_header;
	wlan_flags = ( (u_int8_t *) wlan_header) + 1;
	payload_ptr = data + sizeof(struct ieee80211);

	//XXX:802.11e 
	if ( qos())
	{
		qos_header = (struct ieee80211QoS *) payload_ptr;
		payload_ptr += sizeof(struct ieee80211QoS);
		//printf("QoS Detected in Packet_80211::Init. Adding %d bytes to payload_ptr\n", sizeof(struct ieee80211QoS));
	//	exit(0);
	}
	
	//then check if its smaller than most things we like
	if (  ( !isControl() ) && length < sizeof(struct ieee80211))
		{
				
			//printf("length = %d, sizeof struct 80211 %d\n", length, sizeof(struct ieee80211));
			//printf("Base pcap packet to small. Packet_80211::Init returning false\n");
			return false;
		}
			
	if (wep())
	{
		if (length < sizeof(struct ieee80211) + 8) // 4 byte header, 4 byte checksum
			return false;

		payload_ptr += 4;
	}	
			

	initialized = true;
	return initialized;
}


Packet_80211 :: Packet_80211(struct pcap_pkthdr p, u_char_t *data, u_int32_t dlt) : Pcap_Packet(p, data, dlt)
{
	initialized = false;
	Init(p, data,dlt);
}

bool Packet_80211 :: Init(struct pcap_pkthdr p, u_char_t *data, u_int32_t dlt) 
{
	initialized = false;
	Pcap_Packet temp;
	temp.Init(p, data, dlt); //initalize a temp pcap packet. 
	Init(temp); //call our own Packet_80211::Init(Pcap_Packet) routine. 

}

bool  Packet_80211:: Init(const Packet_80211 &temp)
{

	//printf("Packet_80211::Init(Packet_80211)  called (this is one call up from the 'smart' one\n");
	Init( (Pcap_Packet&) temp); //call my lower level init routine;
	//Now do Packet_80211 centric stuff.
	if (temp.my_mac_time_available)
		{
			my_mac_time_available = true;
			memcpy((void *) & my_mac_time, (void *) &temp.my_mac_time, sizeof(my_mac_time));
		}
	else
		my_mac_time_available = false;

	
}
Packet_80211 :: Packet_80211(const Packet_80211 &right)
{
	*this = right;
}
Packet_80211& Packet_80211:: operator = (const Packet_80211 &temp)
{
	Init(temp);
}

void Packet_80211 :: detail_print()
{
		detail_print(cout);
}

void Packet_80211 :: detail_print(ostream &s)
{
	s << detail_toString();
}

string Packet_80211 :: detail_toString()
{
	string curr_level;

	//now go one up the tree for convenience's sake.
	if (isData())
	{
		string DataString;
		Packet_80211_data TmpDataPacket;
		TmpDataPacket.Init(*this);
		if (!TmpDataPacket.is_initialized())
			return toString() + "un-initializable data packet.\n";

		DataString =  TmpDataPacket.toString(true); 
		curr_level =  DataString;

	}
	else if ( isMgmt())
	{
		string MgmtString;
		Packet_80211_mgmt TmpMgmtPacket;
		TmpMgmtPacket.Init(*this);
		if (!TmpMgmtPacket.is_initialized())
			return toString() + "un-initializable mgmt packet.\n";

		MgmtString = TmpMgmtPacket.toString(true);
		curr_level = MgmtString;

	}

	else if (isControl())
	{
		Packet_80211_ctrl  TmpCtrlPacket;
		TmpCtrlPacket.Init(*this);
		if (!TmpCtrlPacket.is_initialized())
			return toString() + "un-initializable control packet.\n";

		curr_level = TmpCtrlPacket.toString();
	}
	else
		curr_level +="Uknown packet type??\n";
    
	return curr_level;

}


void Packet_80211 :: print()
{
	print(cout);
}
void Packet_80211 :: print(ostream &s)
{
	s << toString();
}


void Packet_80211 :: raw_print()
{
	//wlan_fc = data;
	//wlan_flags = data + 1;
	printf("data %p, wlan_fc: %p, wlan_flags %p\n", data, wlan_fc, wlan_flags);
	printf ("fc: %2.2X", *wlan_fc);
	printf ("\tflags: %2.2X\n", *wlan_flags);
	
}

string Packet_80211 :: toString(bool recurse)
{
	check_initialized("Packet_80211:toString");
	string LevelAbove;
	if (recurse)
		LevelAbove = Pcap_Packet::toString(recurse);

	string curr_level;

	if (isData())
		curr_level += "Data:";
	else if (isMgmt())
		curr_level += "Mgmt:";
	else if (isControl())
		curr_level+="Ctrl:";
	else
		curr_level += "Unkw:";


    char tmp[256];

	snprintf(tmp, sizeof(tmp), "ToDS:%d  FromDS:%d  retry:%d  power_mgmt:%d  ", toDS(), fromDS(), retry(), pwrmgmt());
    curr_level =  curr_level + tmp;
	snprintf(tmp, sizeof(tmp), "more_data:%d  Wep:%d  order:%d", more_data(), wep(), order());
	curr_level = curr_level + tmp;


	if (recurse)
		return LevelAbove + curr_level + "\n";
	else
		return curr_level + "\n";
}
void Packet_80211 :: getWlanHeader(struct DLT_IEEE802_11_frame *p)
{
	memcpy(p, (void *) wlan_header, sizeof(struct DLT_IEEE802_11_frame));
	return;
}

void Packet_80211 :: getWlanHeader(struct ieee80211 *p)
{
	memcpy(p, (void *) wlan_header, sizeof(struct ieee80211));
	return;
}

bool Packet_80211 :: check_flag(u_int8_t flag)
{
	if (*wlan_flags & flag)
			return true;
	else
			return false;
}

bool Packet_80211 :: check_fc(u_int8_t flag)
{
	if (*wlan_fc & flag)
		return true;
	else
		return false;
}

u_int8_t Packet_80211 :: version()
{
	return (u_int8_t) wlan_header->version;
}
u_int8_t Packet_80211 :: type()
{
	return (u_int8_t) wlan_header->type;
}

u_int8_t Packet_80211 :: subtype()
{
	return (u_int8_t) wlan_header->subtype;
}

u_int16_t Packet_80211 :: duration()
{
	return (u_int8_t) wlan_header->duration;
}


bool Packet_80211 :: toDS()
{
	//return check_flag(0x01);
	return (bool) wlan_header->to_ds;
	
}
bool Packet_80211 ::fromDS()
{
	return (bool) wlan_header->from_ds;
	//return check_flag(0x02);
}
bool Packet_80211::more_frag()
{
	return wlan_header->more_frag;
}
bool Packet_80211::retry()
{
	return wlan_header->retry;
}
bool Packet_80211::pwrmgmt()
{
	return wlan_header->pwrmgmt;
}
bool Packet_80211::more_data()
{
	return wlan_header->more_data;
}
bool Packet_80211 :: wep()
{
	return wlan_header->wep;
	//return check_flag(0x40);
}

bool Packet_80211 :: order()
{
	return wlan_header->order;
}

bool Packet_80211 :: qos()
{
	if (isData() && (wlan_header->subtype &&  0x08) ) //b7 of subtype means QoS
	{
	
		//printf("Packet_80211::qos() wlan_header->type == 0x%02x, subtype=0x%02x\n", wlan_header->type, wlan_header->subtype);
		return true;
	}
	else
		return false;
}

bool Packet_80211 :: isData()
{
	//return check_fc(0x08);
	if (wlan_header->type == 2)
		return true;
	else
		return false;
}

bool Packet_80211 :: isControl()
{
	//return check_fc(0x04);
	if (wlan_header->type == 1)
		return true;
	else
		return false;
}
bool Packet_80211 :: isMgmt()
{
	if (wlan_header->type == 0)
		return true;
	else
		return false;
}

//See IEEE 80211 1999 rev page 59
u_int8_t *Packet_80211 :: findSrcAddr()
{
    check_initialized("Packet_80211_data::findSrcAddr");

	if (isControl())
	{
		printf("Error. Does not make sense to call findSrcAddr on control frame.\n");
		exit(0);
	}
    
	if (isData())
	{
		if ( toDS() && fromDS())
		{
				printf("Packet80211::findSrcAddr Error. cannot parse goofy WDS frames.\n");
				exit(0);
		}
	    if (toDS())
	      return wlan_header->addr2;
   		else 
   	 	if (fromDS())
    	    return wlan_header->addr3;
		else //no flags set..
		{
			return wlan_header->addr2;
		}
	}
	
	if (isMgmt())
	{
    	return wlan_header->addr2;
	}

	printf("FindSrcAddr Failed. Bailing.\n");
	exit(0);
}

u_int8_t *Packet_80211 :: findDstAddr()
{
    check_initialized("Packet_80211_data::findDstAddr");

	if (isControl())
	{
		printf("Error. Does not make sense to call findDstAddr on control frame.\n");
		exit(0);
	}
    
	if (isData())
	{

		if ( toDS() && fromDS())
		{
				printf("Packet80211::findDstAddr Error. cannot parse goofy WDS frames.\n");
				exit(0);
		}
	    if (toDS())
	      return wlan_header->addr3;
   		else 
   	 	if (fromDS())
    	    return wlan_header->addr1;
		else //no flags set.. adhoc
		{
			return wlan_header->addr1;
		}
	}
	
	if (isMgmt())
	{
    	return wlan_header->addr1;
	}

}

u_int8_t *Packet_80211 :: findBssid()
{
    check_initialized("Packet_80211_data::findBssid");

	if (isControl())
	{
		printf("Error. Does not make sense to call findBssid on control frame.\n");
		exit(0);
	}
    
	if (isData())
	{

		if ( toDS() && fromDS())
		{
				printf("Packet80211::findBssid Error. cannot parse goofy WDS frames.\n");
				exit(0);
		}
	    if (toDS())
	      return wlan_header->addr1;
   		else 
   	 	if (fromDS())
    	    return wlan_header->addr2;
		else //no flags set.. adhoc
		{
			return wlan_header->addr3;
		}
	}
	
	if (isMgmt())
	{
    	return wlan_header->addr3;
	}

}

void Packet_80211 :: getSrcAddr(u_int8_t *p)
{
    check_initialized("Packet_80211::getSrcAddr");
	memcpy(p, findSrcAddr(), sizeof(wlan_header->addr1)); //nohin speical bout addr1
}

void Packet_80211 :: setSrcAddr(u_int8_t *p)
{
    check_initialized("Packet_80211::setSrcAddr");
	memcpy(findSrcAddr(), p, sizeof(wlan_header->addr1)); //nohin speical bout addr1
}

void Packet_80211 :: getDstAddr(u_int8_t *p)
{
    check_initialized("Packet_80211::getDstAddr");
	memcpy(p, findDstAddr(), sizeof(wlan_header->addr1));
}

void Packet_80211 :: setDstAddr(u_int8_t *p)
{
    check_initialized("Packet_80211::setDstAddr");
	memcpy(findDstAddr(),p, sizeof(wlan_header->addr1));
}

void Packet_80211 :: getBssId(u_int8_t *p)
{
    check_initialized("Packet_80211::getBssid");
	memcpy(p, findBssid(), sizeof(wlan_header->addr1));
}                                                                  

void Packet_80211 :: setBssId(u_int8_t *p)
{
    check_initialized("Packet_80211::setBssid");
	memcpy(findBssid(),p, sizeof(wlan_header->addr1));
}                                                                  
void Packet_80211 :: setSrcDstBss(u_int8_t *s, u_int8_t *d, u_int8_t *b)
{
    check_initialized("Packet_80211::setSrcDstBssid");
	setSrcAddr(s);
	setDstAddr(d);
	setBssId(b);
}

void Packet_80211 :: getSrcDstBss(u_int8_t *s, u_int8_t *d, u_int8_t *b)
{
    check_initialized("Packet_80211::setSrcDstBssid");
	getSrcAddr(s);
	getDstAddr(d);
	getBssId(b);
}

void Packet_80211 :: getFragNum(u_int16_t *p)
{
    check_initialized("Packet_80211::getFragNum");
	*p=wlan_header->fragment;
}

void Packet_80211 :: setFragNum(u_int16_t *p)
{
    check_initialized("Packet_80211::setFragNum");
	wlan_header->fragment=*p;
}
void Packet_80211 :: setFragNum(u_int16_t p) //does byte flipping
{
#if BYTE_ORDER == BIG_ENDIAN
	p = SWAP2(p);
#endif
	wlan_header->fragment= p;
}

void Packet_80211 :: getSeqNum(u_int16_t *p)
{
    check_initialized("Packet_80211::getSeqNum");
	*p = wlan_header->sequence;
}

void Packet_80211 :: setSeqNum(u_int16_t *p)
{
    check_initialized("Packet_80211::setSeqNum");
	wlan_header->sequence=*p;		
}

void Packet_80211 :: setSeqNum(u_int16_t p) //does byte flipping
{
#if BYTE_ORDER == BIG_ENDIAN
	p = SWAP2(p);
#endif
	wlan_header->sequence= p;
}

u_int16_t Packet_80211 :: getDuration()
{
	u_int16_t ret =wlan_header->duration;
//duration is a 16 bit little endian counter (in micro secs)
#if BYTE_ORDER == BIG_ENDIAN
	ret = SWAP2(ret);
#endif 
	return ret;
}

void Packet_80211 :: getDuration(u_int8_t *dur)
{
	memcpy(dur, &wlan_header->duration, 2);
}

void Packet_80211 :: setDuration(u_int16_t p) //does byte flipping
{
	
#if BYTE_ORDER == BIG_ENDIAN
	p = SWAP2(p);
#endif
	wlan_header->duration = p;
}

void Packet_80211 :: setDuration(u_int8_t *dur)//just copies a buffer.
{
	memcpy( &wlan_header->duration, dur, 2);
}

void Packet_80211 :: getRecvAddr(u_int8_t *p)
{
	if (!isControl())
	{
		printf("Error. Does not make sense to call getReceiverId on ! control frames.\n");
		exit(0);
	}
        	memcpy(p, wlan_header->addr1, sizeof(wlan_header->addr1));
}

void Packet_80211 :: setRecvAddr(u_int8_t *p)
{
	if (!isControl())
	{
		printf("Error. Does not make sense to call getReceiverId on ! control frames.\n");
		exit(0);
	}
        	memcpy(wlan_header->addr1,p, sizeof(wlan_header->addr1));
}
void Packet_80211  :: check_initialized(char *s)
{
	//printf("Packet_80211::Checkinitialized\n");
    if (initialized)
        return;
    //
    //If not, error so we can track down the bug.
    printf("Error! Packet_80211::check_initialized failed. called from: %s\n", s);
    //BreakHere();
    exit(0);

}               

bool Packet_80211 :: get_mac_time(struct mac_time *time)
{
	if (!my_mac_time_available)
		return false;

	memcpy(time, &my_mac_time, sizeof(my_mac_time));
	return true;
}

timepiece Packet_80211 :: get_pcap_ts()
{
	timepiece T = get_pcap_hdr().ts;
	return T;
}

timepiece Packet_80211 :: get_mac_ts()
{
	timepiece T;
	struct mac_time M;
	if (!mac_time_available())
	{
		printf("Packet_80211::get_mac_time Error. get_mac_ts called but mac_time_avilable false!\n");
		exit(0);
	}
	if (! get_mac_time(&M))
	{
		printf("Packet_80211::get_mac_time Error. get_mac_time failed!\n");
		exit(0);
	}

	if (! MacTimeConverter(M, T))
	{
		printf("Packet_80211::get_mac_ts:: Error. Failed to convert time (This should NOT happen.\n");
		printf("there is a mismatch between mac_time_Available and MacTimeConverter.\n");
		exit(0);
	}

	return T;
}


bool Packet_80211 :: MacTimeConverter(struct mac_time time, timepiece &ret_timepiece)
{
    if (time.dlt == DLT_PRISM_HEADER)
    {
        ret_timepiece = PrismMacTimeConverter(time);
        return true;
    }
    else
    {
        printf("MacTimeConverter::Error. Passed mac_time with dlt not prism.\n");
        printf("Prism = %d, time = %d\n", DLT_PRISM_HEADER, time.dlt);
        exit(0);
        return false;

    }                                                                                                                     
}
timepiece Packet_80211 :: PrismMacTimeConverter(struct mac_time time)
{
    int SEC_SCALE_FACTOR = 0x00100000;
    int USEC_SCALE_FACTOR  = SEC_SCALE_FACTOR / 1000000;
    
    int ts = ( (unsigned long) *((unsigned long *) (time.mac_time + 8)) );
#if BYTE_ORDER == BIG_ENDIAN
    ts = SWAP4(ts);
#endif 
    struct timeval curr_tv;
    curr_tv.tv_sec = ts / SEC_SCALE_FACTOR;
    ts = ts % SEC_SCALE_FACTOR;
    curr_tv.tv_usec = (ts *  USEC_SCALE_FACTOR);

    while (curr_tv.tv_usec > 1000000)
    {
        curr_tv.tv_sec++;
        curr_tv.tv_usec -=   1000000;
    }
    timepiece ret(curr_tv);
    return ret;

}

