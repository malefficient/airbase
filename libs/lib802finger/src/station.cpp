#include "station.h"
/*
 * TODO: Ad-hoc is pretty much broke. It just isnt taken into consideration when 
 * inside the < and == operators. this means a single ad hoc packet will render an entire
 * map of tese buggers ad hoc.
 */
Station::Station()
{
	memset(info.addr, 0, 6);
	memset(info.assoc_with, 0, 6);
	info.ad_hoc = false;
	info.WDS = false;
	initialized = false;
}

//Analyze an individual 80211 packet and try to discern
//Who is associated with what. May not be able to, in that
//case returns false.
bool PacketToStation(Packet_80211 &P, Station &Sta)
{
	station_struct_type s;
	s.ad_hoc = false;
	s.WDS = false;

	//In order to glean information from CTRL frames we would
	//have to guess and parse each individual type since the From/TOds bits are unused.
	//e.g. it makes sense to assume that a DeAuths came From APS toward clients. but is
	//it really worth coding in each speical case? not really. 
	//
	if (P.isControl())
	{
	//	printf("Control\n");
		return false;
	}


	if (P.isData() || P.isMgmt())
	{
		//printf("Data || Mgmt\n");	
	}	
	if (P.fromDS() && P.toDS())
	{
		s.WDS = true;
	}
			
	if (P.fromDS())
	{
		P.getSrcAddr(s.assoc_with);
		P.getDstAddr(s.addr);
	}
	else if (P.toDS())
	{
		P.getSrcAddr(s.addr);
		P.getDstAddr(s.assoc_with);
	}
	else
	{
			s.ad_hoc = true;
			P.getSrcAddr(s.addr);

	}
	Sta.Init(s);
	return true;

}

Station::Station(u_char *addr, u_char *assoc_with)
{
	memcpy(info.addr, addr, 6);
	memcpy(info.assoc_with, assoc_with, 6);
	info.ad_hoc = false;
	initialized = true;
}

bool Station::Init(station_struct_type s)
{
	info = s;
	initialized = true;
	return true;
}
station_struct_type Station::get_station_struct()
{
	if (!initialized)
	{
		printf("Error! get_station_struct called on uninitialized Station. Bailing.\n");
		exit(0);
	}
	return info;
}
string Station::toString()  const
{
	string A1, A2;
	string ret;
	u_int8_t t1[6];
	getStaAddr(t1);
	u_int8_t t2[6];
	getAssocAddr(t2);

	A1 = mac_to_string(t1);
	A2 = mac_to_string(t2);

	ret = A1 + "\t" + A2;
	//Broke. see comment at top. same applies to WDS.
	/* 
	if (info.ad_hoc)
		ret = ret +  "\t" + "AD-HOC";
	*/
	if (info.WDS)
		ret = ret + "\t" + "WDS";
	
	return ret;
}
bool Station::ad_hoc() const
{
	return info.ad_hoc;
}
bool Station:: operator == (Station &Right)
{
	
/*
	if ( (memcmp(info.addr, Right.info.addr, 6) == 0))
		return true;
	else
		return false;
*/	
//---tricky
	if ( (memcmp(info.addr, Right.info.addr, 6) == 0) && (memcmp(info.assoc_with, Right.info.assoc_with, 6) == 0) )
		return true;
	else
		return false;

}
void Station::getStaAddr(u_int8_t addy[6]) const
{
	memcpy(addy, info.addr, 6);
}
void Station::getAssocAddr(u_int8_t addy[6]) const
{
	memcpy(addy, info.assoc_with, 6);
}

bool Station:: operator < (const Station &Right) const
{
	u_int8_t his_sta_addr[6];
	Right.getStaAddr(his_sta_addr);

	u_int8_t his_assoc_addr[6];
	Right.getAssocAddr(his_assoc_addr);

	int ret1 = memcmp(info.addr, his_sta_addr, 6);
	int ret2 = memcmp(info.assoc_with, his_assoc_addr, 6);


	if (ret1 == 0 && ret2 == 0)
            return false;

	if (ret1 < 0) //mac mac < his mac
      {
            //printf(" (%d, %d) < (%d %d)\n", type, subtype, right.type, right.subtype);
            return true;
      }
		
     else if (ret1 > 0) //his mac >
     {
            return false;
     }

        //at this point mac addysare equal
        if ( ret2 < 0) //my bssid < his
        {
            return true;
        }
        else if (ret2 > 0) //my bssid > his
        {
            //printf(" (%d, %d) >=  (%d %d)\n", type, subtype, right.type, right.subtype);
            return false;
        } 
		else
		{
			printf("unhandled case is Station::operator <\n");
			printf("ret1 = %d, ret2= %d\n", ret1, ret2);
			exit(0);
		}

}

bool WriteStationSetToFile(char *fname, StationSetType &List)
{
	FILE *tmp;
	tmp = fopen(fname, "w+");
	if (!tmp)
		return false;

	bool ret;
	ret = WriteStationSetToFile(tmp, List);
	fclose(tmp);
	return ret;
}

bool ReadStationSetFromFile(char *fname, StationSetType &List)
{
	FILE *tmp;
	tmp = fopen(fname, "r");
	if (!tmp)
		return false;

	bool ret;
	ret = ReadStationSetFromFile(tmp, List);
	fclose(tmp);
	return ret;
}

bool WriteStationSetToFile(FILE *fp, StationSetType &List)
{
	
	StationSetType::iterator my_iter;
	my_iter = List.begin();
	station_struct_type tmp_struct;
	Station tmp;
	while (my_iter != List.end())
	{
		tmp = *my_iter;
		tmp_struct = tmp.get_station_struct();
		if ( fwrite( &tmp_struct, sizeof(station_struct_type), 1, fp) != 1)
			return false;
		my_iter++;
	}
	return true;
}

string StationSetToString(StationSetType &List)
{
	StationSetType::iterator my_iter;
	my_iter = List.begin();
	string ret;
	Station tmp;
	while (my_iter != List.end())
	{
		tmp = *my_iter;
		ret = ret + tmp.toString() + "\n";
		my_iter++;
	}
	

	return ret;
	
}

bool ReadStationSetFromFile(FILE *fp, StationSetType &List)
{
	
	StationSetType::iterator my_iter;
	station_struct_type curr_station_struct;
	Station CurrStation;
	while ( fread( &curr_station_struct, sizeof(station_struct_type), 1, fp) ==1)
	{
		CurrStation.Init(curr_station_struct);
		List.insert(CurrStation);
	}
	if (feof(fp))
		return true;
	else
	{
		printf("Error in ReadStationSet.\n");
		perror("fread");
		exit(0);
		return false;
	}
}

//returns: -1 (Error, not enough info from packet);
//returns:  0 (STa aleady in list)
//returns:  1 (adding new station)
int UpdateStationSet(StationSetType &StaSet, Packet_80211 &P)
{
	Station CurrSta;
	StationSetType::iterator it;

	if (!PacketToStation(P, CurrSta))
		return -1; 

	it = StaSet.find(CurrSta);
	if (it != StaSet.end())
	{
		//XXX update cumulative statistics here:
		//printf("Station already in set.\n");
		return 0;
	}
	else
	{
		//printf("Adding Station\n");
		StaSet.insert(CurrSta);
		return 1;
	}
}

//returns the number of uniques STA's that were created
int UpdateStationSet(StationSetType &S, Packet_80211List &List)
{
	Packet_80211List::iterator my_iter;

	my_iter = List.begin();
	int ret = 0;
	while (my_iter != List.end())
	{
		if (UpdateStationSet(S, *my_iter) > 0)
			ret++;
		my_iter++;
	}
	return ret;	
}
	

