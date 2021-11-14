#include "packet80211-data.h"
#include "datatypes.h"
#include "jc-util.h"
//#include "print.h"
#include <netinet/in.h> //ntohs 

Packet_80211_data :: Packet_80211_data()
{

}

void Packet_80211_data :: Init(const Packet_80211 &base)
{
	Packet_80211::Init(base);
	if (!isData())
	{
		printf("Error. Init'd Packet_80211_data with a base packet thats not data\n");
		print();
		print_hex_dump();
		exit(0);
	}
		
}
Packet_80211_data :: Packet_80211_data(const Packet_80211 &base) : Packet_80211(base)
{
	Init(base); 
}


string Packet_80211_data :: toString(bool recurse)
{
	check_initialized("Packet_80211_data::toString(u_int8_t *)");
	string LevelAbove;
	if (recurse)
		LevelAbove = Packet_80211::toString(recurse);

	string curr_level;

	curr_level = LevelAbove;

	u_int8_t keynum;
	u_int8_t iv[3]; //init vector
	u_int8_t icv[4]; //checksum
	
	char tmp[256];

	u_int8_t src[6], dst[6], bss[6];
   	getSrcAddr(src);
    getDstAddr(dst);
    getBssId(bss);



	snprintf(tmp, sizeof(tmp), "[%s]",  mac_to_string(src).c_str());
	curr_level = curr_level + tmp + "==>"; 

	snprintf(tmp, sizeof(tmp),"[%s]", mac_to_string(dst).c_str());
	curr_level = curr_level + tmp + "  ";

	snprintf(tmp, sizeof(tmp), "(%s)", mac_to_string(bss).c_str());
	curr_level = curr_level + tmp + "\n";

	if (qos())
	{
		snprintf(tmp, sizeof(tmp), "QoS: 0x%02X%02X\t", qos_header->b1, qos_header->b2);
		curr_level = curr_level + tmp;
	}
		
	snprintf(tmp, sizeof(tmp), "PduDataLength: %d\t", getPDUDataLength());
	curr_level += tmp;
		
	if (wep())
	{
		getWepIV(iv);
		getWepKeyNum(&keynum);
		getWepICV(icv);
		snprintf(tmp, sizeof(tmp), "IV: %2.2x%2.2x%2.2x\t", iv[0], iv[1], iv[2]);
		curr_level = curr_level + tmp;

		snprintf(tmp, sizeof(tmp), "Key#: %2.2x", keynum);
		curr_level = curr_level + tmp;

		//snprintf(tmp, sizeof(tmp), "ICV (checksum) %2.2x%2.2x%2.2x%2.2x", icv[0], icv[1], icv[2], icv[3]);
		//curr_level = curr_level + tmp + "\n";
	}
	
		return curr_level + "\n";

}

bool Packet_80211_data :: getWepIV(u_int8_t * p)
{
	check_initialized("Packet_80211_data::getWepIv(u_int8_t *)");
	if (!wep())
	{
		p = NULL;
		return false;
	}

	//memcpy(p, data +sizeof(struct ieee80211), 3); // 3 byte IV
	memcpy(p, payload_ptr - 4, 3); // 3 byte IV, followed by 1 byte key selector
	return true;
}
bool Packet_80211_data :: getWepIV(IV &iv)
{
	check_initialized("Packet_80211_data::getWepIv(IV &)");
	if (!wep())
		return false;

	//NOTE: we offset from payload_ptr now, instead of counting the size of ieee80211hdr.
	//QOS makes that size vary (Grrr!)
	//memcpy(iv.IV, data +sizeof(struct ieee80211), 3); // 3 byte IV
	memcpy(iv.IV, payload_ptr - 4,  3); // 3 byte IV
	//iv.keynum =  (u_int8_t) *(data + sizeof(struct ieee80211) + 3);
	iv.keynum =  (u_int8_t) * (payload_ptr - 1); //1 byte in front of payload
	//iv.b1 = (u_int8_t) *(data + sizeof(struct ieee80211) + 3 + 1);//first byte after key #
	//iv.b2 = (u_int8_t) *(data + sizeof(struct ieee80211) + 4 + 1);
	iv.b1 = (u_int8_t) *payload_ptr;
	iv.b2 = (u_int8_t) * (payload_ptr + 1);
	return true;

}

bool Packet_80211_data :: getWepKeyNum(u_int8_t *p)
{
	check_initialized("Packet_80211_data::getWepKeyNum");
	if (!wep())
	{
			p = NULL;
			return false;
		}

		//data here is the Packet types superclass pointer into pcap land.
		//memcpy(p, data +sizeof(struct ieee80211) + 3, 1); //directly after IV
		memcpy(p, payload_ptr - 1, 1); //directly before PDU IV
		return true;
}

bool Packet_80211_data :: getWepICV(u_int8_t *p)
{
	check_initialized("Packet_80211_data::getWepICV");

		if (!wep())
		{
			p = NULL;
			return false;
		}

		memcpy(p, data + length - 4, 4); //Last 4 bytes
		return true;

}

/*
 * Pay Attention here, we cant be off by a byte when it comes to this >:>.
 * if the packet is encrypted getDataLength returns the size of the PDU 
 * (thats IEEE talk for the data, PLUS 4 BYTES FOR THE ICV.
 * That is because the ICV is encrypted with the data and sent along.
 *
 * If its not encrypted then this is all straight forward
 */
// return size of the payload inside the 80211 header
int Packet_80211_data :: getPDUDataLength()
{
	check_initialized("Packet_80211_data::getDataLength");
	int pdu_length = length - sizeof(struct ieee80211);
	if (wep())
		pdu_length -= 4; //dont count IV;
	if (qos())
		pdu_length -= sizeof(struct ieee80211QoS);
	if (pdu_length < 0)
		return 0;
	return pdu_length;

	 /*
	//XXX: need to account for QOS, should be using payload_ptr.
	if (wep())
			return (length - sizeof(struct ieee80211) -4); // dont count IV.
	else
		return (length - sizeof(struct ieee80211));
	*/
}
void Packet_80211_data :: getPDUData(u_char_t *dst)
{
		check_initialized("Packet_80211_data::getData");
		memcpy(dst, payload_ptr, getPDUDataLength());
		/*
		if (wep())					// +4 /-4 to account for not sending IV
								//note the ICV is still returned.	
			memcpy(dst, data + sizeof(struct ieee80211) + 4, length - sizeof(struct ieee80211) -4);
		else
			memcpy(dst,data + sizeof(struct ieee80211), length - sizeof(struct ieee80211));
		*/
}



