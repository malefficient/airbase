#include "packet80211-crafter.h"
#include "datatypes.h"
#include "jc-util.h"

Packet_80211_Crafter :: Packet_80211_Crafter( 
						  Packet_80211_data_Crafter &_data_crafter,
                          Packet_80211_ctrl_Crafter &_ctrl_crafter,
                          Packet_80211_mgmt_Crafter &_mgmt_crafter)
{
	data_crafter = _data_crafter;
	ctrl_crafter = _ctrl_crafter;
	mgmt_crafter = _mgmt_crafter;

}

bool Packet_80211_Crafter :: craft(u_int8_t type, u_int8_t subtype, Packet_80211 &P)
{

	struct ieee80211 hdr;
	memset((void *) &hdr, 0, sizeof(struct ieee80211));
	hdr.version = 0;
	hdr.type = type;
	hdr.subtype = subtype; 
	if (type == Data80211Type) //2
	{
		return data_crafter.craft( (Packet_80211_data &) P);	
	}
	else if (type == Ctrl80211Type) //1
	{
		return ctrl_crafter.craft(subtype, (Packet_80211_ctrl &) P);	
	}
	else if (type == Mgmt80211Type) // 0
	{
		return mgmt_crafter.craft(subtype, (Packet_80211_mgmt &) P);	

	}
	else
		return false;

	

}

