#ifndef _STATION_H
#define _STATION_H
#include "airware.h"
#include <set>
using namespace std;

typedef struct _station_struct
{
		u_char addr[6];
		u_char assoc_with[6];
		bool ad_hoc;
		bool WDS; //ToDs and FromDS set
} station_struct_type;

class Station
{
		public: //XXX, should be private.
		bool initialized;
		station_struct_type info;
		public:
		bool is_initialized() { return initialized;}
		Station();
		bool ad_hoc() const;
		Station(u_char*addy, u_char*assoc_with);
		string toString() const;
		bool Init(station_struct_type);
		station_struct_type get_station_struct();
		bool operator == (Station &right);
		bool operator < (const Station &right) const;
		void getStaAddr(u_int8_t[6]) const;
		void getAssocAddr(u_int8_t[6]) const;

};

bool PacketToStation(Packet_80211 &P, const Station &S);
/*
class StationLessThan
{
	public:
  bool operator()(const Station* s1, const Station* s2) const
  {
	Station t1 = * s1;
	Station t2 = *s2;
	station_struct_type a1 = t1.get_station_struct();
	station_struct_type a2 = t2.get_station_struct();

    return (memcmp(a1.addr, a2.addr, 6) < 0);
  }

  bool operator()(const Station& s1, const Station& s2) const
  {
	Station t1 =  s1;
	Station t2 = s2;
	station_struct_type a1 = t1.get_station_struct();
	station_struct_type a2 = t2.get_station_struct();
    return memcmp(a1.addr, a2.addr, 6) < 0;
  }


};
*/
//typedef set<Station, struct StationLessThan> StationSetType;
typedef set<Station> StationSetType;

bool WriteStationSetToFile(char *filename, StationSetType &List);
bool ReadStationSetFromFile(char *filename, StationSetType &List);

bool WriteStationSetToFile(FILE *fp, StationSetType &List);
bool ReadStationSetFromFile(FILE *fp, StationSetType &List);

string StationSetToString(StationSetType &List);

int UpdateStationSet(StationSetType &StaSet, Packet_80211 &P);
int UpdateStationSet(StationSetType &Set, Packet_80211List &List);

bool PacketToStation(Packet_80211 &P, Station &Sta);

#endif
