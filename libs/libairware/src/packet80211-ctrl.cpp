#include "packet80211-ctrl.h"
#include "datatypes.h"
#include "jc-util.h"
#include <netinet/in.h> //ntohs 


Packet_80211_ctrl :: Packet_80211_ctrl()
{

}

void Packet_80211_ctrl :: Init(const Packet_80211 &base)
{
	Packet_80211::Init(base);
	if (! initialized)
		{
			printf("Packet_80211_ctrl::Error. Failed to initialize base class.\n");
			return;
		}
	if (!isControl())
	{
		printf("Error. Init'd Packet_80211_ctrl with a base packet thats not control\n");
		initialized = false;
		return;
		print();
		print_hex_dump();
		exit(0);
	}

	if (length < control_packet_correct_size( wlan_header->subtype))
	{
		initialized = false;
		return;
	}

}
Packet_80211_ctrl :: Packet_80211_ctrl(const Packet_80211 &base) : Packet_80211(base)
{
	Init(base); 
}

void Packet_80211_ctrl :: print()
{
	print(cout);
}

void Packet_80211_ctrl :: print(ostream &s)
{
	s << toString();
}
string Packet_80211_ctrl :: toString(bool recurse)
{
	check_initialized("Packet_80211:ctrl::toString");

	string LevelAbove;
	if (recurse)
		LevelAbove = Packet_80211::toString(recurse);

	string curr_level;

	curr_level = LevelAbove;

	char tmp[256];
	snprintf(tmp, sizeof(tmp),  "subtype: %s (0x%X)\n",
					control_subtype_to_string(wlan_header->subtype).c_str(),
					wlan_header->subtype);

	curr_level += tmp;
	
	//the Find* functions know what packets have what fields. Rather than reproduce
	//the logic here we call them. If one fails then we simply dont have that field.
	u_int8_t *local_RA, *local_TA, *local_BSS;
	local_RA = findRecvAddr(); //[]
	local_TA = findTA(); //[]
	local_BSS = findBssId(); //<>

	//snprintf(tmp, sizeof(tmp), "##%d : $%d##\n", length, control_packet_correct_size(wlan_header->subtype));
	//curr_level += tmp;	

	bool append_hex_dump = false;
	if (  is_reserved_ctrl_subtype(wlan_header->subtype))
	{
		append_hex_dump = true;
	}
	else if ( length != control_packet_correct_size(wlan_header->subtype))
	{
		snprintf(tmp, sizeof(tmp), "Invalid size for frame %+d bytes\n", 
						length - control_packet_correct_size(wlan_header->subtype));
		append_hex_dump = true;
		curr_level += tmp;
	}
	
	//One special case lets us do consistent output to the screen:
	if (wlan_header->subtype == PS_Poll)
	{
			

		if (local_BSS==NULL || local_TA==NULL)
			append_hex_dump=true;
		else
			snprintf(tmp, sizeof(tmp), "[TA: %s]==><BSS: %s> AID:%4.4X\n", 
							hex2string(local_TA, 6).c_str(), 
							hex2string(local_BSS, 6).c_str(), wlan_header->duration);
		if (append_hex_dump)
			return curr_level + get_hex_dump() + "\n";
		else
			return curr_level + tmp;
	
	}

	if ( local_TA != NULL)
	{
		snprintf(tmp, sizeof(tmp), "[TA %s]==>", mac_to_string(local_TA).c_str());
		curr_level += tmp;
	}
	if ( local_BSS != NULL)
	{
		snprintf(tmp, sizeof(tmp), "<BSS %s>==>", mac_to_string(local_BSS).c_str());
		curr_level += tmp;
	}
	if ( local_RA != NULL)
	{
		snprintf(tmp, sizeof(tmp), "[RA %s]", mac_to_string(local_RA).c_str());
		curr_level += tmp;
	}

	if (append_hex_dump)
		curr_level = curr_level +  get_hex_dump();

	return curr_level + "\n";
}

bool Packet_80211_ctrl ::getRecvAddr(u_int8_t *addr)
{
	check_initialized("Packet_80211:ctrl::getRecvAddr");
	if (findRecvAddr() == NULL)
		return false;
	
	memcpy(addr, findRecvAddr(), 6);
	return true;
}
bool Packet_80211_ctrl ::setRecvAddr(u_int8_t *addr)
{
	check_initialized("Packet_80211:ctrl::setRecvAddr");
	if (findRecvAddr() == NULL)
		return false;
	
	memcpy(findRecvAddr(),addr, 6);
	return true;
}
bool Packet_80211_ctrl ::getTA(u_int8_t *addr)
{
	check_initialized("Packet_80211:ctrl::getTA");
	if (findTA() == NULL)
		return false;
	
	memcpy(addr, findTA(), 6);
	return true;
}

bool Packet_80211_ctrl ::setTA(u_int8_t *addr)
{
	check_initialized("Packet_80211:ctrl::setTA");
	if (findTA() == NULL)
		return false;
	
	memcpy(findTA(),addr, 6);
	return true;
}

bool Packet_80211_ctrl ::getBssId(u_int8_t *addr)
{
	check_initialized("Packet_80211:ctrl::getBssId");
	if (findBssId() == NULL)
		return false;
	
	memcpy(addr, findBssId(), 6);
	return true;
}
bool Packet_80211_ctrl ::setBssId(u_int8_t *addr)
{
	check_initialized("Packet_80211:ctrl::setBssId");
	if (findBssId() == NULL)
		return false;
	
	memcpy(findBssId(), addr, 6);
	return true;
}


u_int8_t * Packet_80211_ctrl :: findRecvAddr()
{

	check_initialized("Packet_80211:ctrl::findRecvAddr");
	if (is_reserved_ctrl_subtype(wlan_header->subtype))
		return NULL;

	if (length < control_packet_correct_size(wlan_header->subtype))
			return NULL;
	
	switch( wlan_header->subtype)
	{
			case BlockAckReq: return (u_int8_t*)&wlan_header->addr1; break; //RA, TA, BAR control
			case BlockAck:	  return (u_int8_t*)&wlan_header->addr1; break; //RA, TA, BA control
			case PS_Poll: return NULL; break; // BSS, TA
			case RTS: return (u_int8_t*) &(wlan_header->addr1); break; // RA
			case CTS: return (u_int8_t*)&wlan_header->addr1; break; // RA
			case ACK: return (u_int8_t*)&wlan_header->addr1; break; //RA 
			case CF_End: return (u_int8_t*)&wlan_header->addr1; break; //RA BSS
			case CF_End_Ack: return (u_int8_t*)&wlan_header->addr1; break; //RA BSS
	};

}
u_int8_t * Packet_80211_ctrl :: findTA()
{
	check_initialized("Packet_80211:ctrl::findTA");
	if (is_reserved_ctrl_subtype(wlan_header->subtype))
		return NULL;

	if (length < control_packet_correct_size(wlan_header->subtype))
			return NULL;

	switch( wlan_header->subtype)
	{
			case BlockAckReq: return (u_int8_t*)&wlan_header->addr2; break; //RA, TA, BAR control
			case BlockAck:	  return (u_int8_t*)&wlan_header->addr2; break; //RA, TA, BA control
			case PS_Poll: return (u_int8_t*) &(wlan_header->addr2); break; // BSS, TA
			case RTS: return (u_int8_t*) &(wlan_header->addr2); break; // RA
			case CTS: return NULL; break; // RA
			case ACK: return  NULL; break; //RA 
			case CF_End: return NULL; break; //RA BSS
			case CF_End_Ack: return NULL; break; //RA BSS
	};

}

u_int8_t * Packet_80211_ctrl :: findBssId()
{
	check_initialized("Packet_80211:ctrl::findBssId");

	if (is_reserved_ctrl_subtype(wlan_header->subtype))
		return NULL;
	if (length < control_packet_correct_size(wlan_header->subtype))
			return NULL;
	switch( wlan_header->subtype)
	{
			case BlockAckReq: return NULL; break; //RA, TA, BAR control
			case BlockAck:	  return NULL; break; //RA, TA, BA control
			case PS_Poll: return (u_int8_t*) &(wlan_header->addr1); break; // BSS, TA
			case RTS: return  NULL; break; //RA
			case CTS: return NULL; break; // RA
			case ACK: return  NULL; break; //RA 
			case CF_End: return (u_int8_t *) &(wlan_header->addr2); break; //RA BSS
			case CF_End_Ack: return (u_int8_t*) &(wlan_header->addr2) ; break; //RA BSS
	};

}

int control_packet_correct_size(u_int8_t  subtype)
{
	//returning > 0 allows us to mess with reserved frames. If
	//we return 0 then we cant use this class to build reserved packets.
	if ( is_reserved_ctrl_subtype(subtype))
			return 10;

	switch (subtype)
	{
			case BlockAckReq: return 20; break; //RA, TA, BAR Ctrl, Seq Ctrl
			case BlockAck: return 148; break; //RA, TA, BA Ctrl, Seq Ctrl, Bitmap
			case PS_Poll: return 16; break; // BSS, TA
			case RTS: return 16; break; // RA
			case CTS: return 10; break; // RA
			case ACK: return 10; break; //RA 
			case CF_End: return 16; break; //RA BSS
			case CF_End_Ack: return 16; break; //RA BSS

	};

}
string control_subtype_to_string(u_int8_t subtype)
{
	string reserved_string = "Reserved";
	if (is_reserved_ctrl_subtype(subtype))
			return reserved_string;

	switch (subtype)
	{
			case BlockAckReq: return "BlockAckReq"; break; //RA, TA, BAR Ctrl, Seq Ctrl
			case BlockAck: return "BlockAck"; break; //RA, TA, BA Ctrl, Seq Ctrl, Bitmap
			case PS_Poll: return "PS_Poll"; break; // BSS, TA
			case RTS: return "RTS"; break; // RA
			case CTS: return "CTS"; break; // RA
			case ACK: return "ACK"; break; //RA 
			case CF_End: return "CF_End"; break; //RA BSS
			case CF_End_Ack: return "CF_End_Ack"; break; //RA BSS
	};

	return "Totally-Whack";

}

bool is_reserved_ctrl_subtype(u_int8_t subtype)
{
	if (  subtype < BlockAckReq)
		return true;
	else
		return false;
}
/*
bool is_valid_ctrl_subtype(u_int8_t subtype)
{
	if ( is_reserved_ctrl_subtype(subtype))
			return false;
	if (subtype < CF_End_Ack && subtype < PS_Poll)
			return false;

	return true;
}
*/

