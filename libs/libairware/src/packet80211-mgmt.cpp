#include "packet80211-mgmt.h"
#include "datatypes.h"
#include "jc-util.h"
#include <netinet/in.h> //ntohs 


Packet_80211_mgmt :: Packet_80211_mgmt()
{

}

void Packet_80211_mgmt :: Init(const Packet_80211 &base)
{
	Packet_80211::Init(base);
	if (! initialized)
		{
			printf("Packet_80211_mgmt::Error. Failed to initialize base class.\n");
			return;
		}
	if (!isMgmt())
	{
		printf("Error. Init'd Packet_80211_mgmt with a base packet thats not control\n");
		initialized = false;
		return;
		print();
		print_hex_dump();
		exit(0);
	}

	if (length < mgmt_packet_min_size( wlan_header->subtype))
	{
		initialized = false;
		return;
	}

}
Packet_80211_mgmt :: Packet_80211_mgmt(const Packet_80211 &base) : Packet_80211(base)
{
	Init(base); 
}

void Packet_80211_mgmt :: print()
{
	print(cout);
}

void Packet_80211_mgmt :: print(ostream &s)
{
	s << toString();
}
string Packet_80211_mgmt :: toString(bool recurse)
{
	check_initialized("Packet_80211:mgmt::toString");

	string LevelAbove;
	if (recurse)
		LevelAbove = Packet_80211::toString(recurse);

	string curr_level;

	curr_level = LevelAbove;

	char tmp[256];

 	u_int8_t src[6], dst[6], bss[6];
    getSrcAddr(src);
    getDstAddr(dst);
    getBssId(bss);

    snprintf(tmp, sizeof(tmp), "[%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x]", src[0], src[1], src[2], src[3], src[4], src[5]);
    curr_level = curr_level + tmp + "==>";
    snprintf(tmp, sizeof(tmp), "[%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x]", dst[0], dst[1], dst[2], dst[3], dst[4], dst[5]);
    curr_level = curr_level + tmp + "  ";
    snprintf(tmp, sizeof(tmp), "(%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x)", bss[0], bss[1], bss[2], bss[3], bss[4], bss[5]);
    curr_level = curr_level + tmp + "\n";

	snprintf(tmp, sizeof(tmp),  "subtype: %s (0x%X)\n",
					mgmt_subtype_to_string(wlan_header->subtype).c_str(),
					wlan_header->subtype);

	curr_level += tmp;
	
	//the Find* functions know what packets have what fields. Rather than reproduce
	//the logic here we call them. If one fails then we simply dont have that field.

	//snprintf(tmp, sizeof(tmp), "##%d : $%d##\n", length, control_packet_correct_size(wlan_header->subtype));
	//curr_level += tmp;	

	bool append_hex_dump = false;
	if (  is_reserved_mgmt_subtype(wlan_header->subtype))
	{
		//snprintf(tmp, sizeof(tmp), "Reserved mgmt subtype:\n");
		append_hex_dump = true;
	//	curr_level += tmp;
	}
	else if ( length < mgmt_packet_min_size(wlan_header->subtype))
	{
		snprintf(tmp, sizeof(tmp), "Invalid size for frame %+d bytes\n", 
						length - mgmt_packet_min_size(wlan_header->subtype));
		append_hex_dump = true;
		curr_level += tmp;
	}
	
	if (append_hex_dump)
		curr_level = curr_level +  get_hex_dump() + "\n";

	return curr_level;
}




int mgmt_packet_min_size(u_int8_t  subtype)
{
	
	//returning > 0 allows us to mess with reserved frames. If
	//we return 0 then we cant use this class to build reserved packets.
	if ( is_reserved_mgmt_subtype(subtype))
			return sizeof(struct ieee80211);;

	//May want to make this smarter later. this is a good sane though.
	return sizeof(struct ieee80211) ; 

	switch (subtype)
	{
			/*
			case PS_Poll: return 16; break; // BSS, TA
			case RTS: return 16; break; // RA
			case CTS: return 10; break; // RA
			case ACK: return 10; break; //RA 
			case CF_End: return 16; break; //RA BSS
			case CF_End_Ack: return 16; break; //RA BSS
		*/
			default:
					printf("Packet80211_mgmt::mgmt_packet_min_size::Error. Hit default cause. Should not happen\n");
					exit(0);
	};

}
string mgmt_subtype_to_string(u_int8_t subtype)
{
	string reserved_string = "Reserved";
	if (is_reserved_mgmt_subtype(subtype))
			return reserved_string;

	enum mgmt_subtype {Assoc_Req = 0, Assoc_Res = 1, Reassoc_Req =  2, Reassoc_Res = 3, Probe_Req = 4, Probe_Res = 5,   
			                   Beacon = 8, Atim = 9, Disassoc = 10, Auth = 11, Deauth = 12, Action=13};
	switch (subtype)
	{

			case Assoc_Req:	return "Assocation Request"; break;
			case Assoc_Res:	return "Assocation Response"; break;
			case Reassoc_Req:	return "Reassocation Request"; break;
			case Reassoc_Res:	return "Reassocation Response"; break;
			case Probe_Req:  	return "Probe Request"; break;
			case Probe_Res:		return "Probe Response"; break;
			case Beacon:		return "Beacon"; break;
			case Atim:			return "ATIM"; break;
			case Disassoc:		return "Disassocation"; break;
			case Auth:			return "Authentication"; break;
			case Deauth:		return "Deauthentication"; break;
			case Action:		return "Action"; break;
			default:
					printf("Packet80211_mgmt::mgmt_subtype_to_string::Error. Hit default case. Should not happen\n");
					exit(0);
	};

	return "Totally-Whack";

}

bool is_reserved_mgmt_subtype(u_int8_t subtype)
{
	if (  subtype <= 7 && subtype >= 6)
		return true;
	else if (  subtype <= 15 && subtype >= 14)
		return true;
	else
		return false;
}

