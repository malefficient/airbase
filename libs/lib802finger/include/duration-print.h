#ifndef _DURATIONPRINT_H
#define _DURATIONPRINT_H
#include <sys/types.h>
#include <dirent.h>
#include "airware.h"
#include <list>
#include <map>
#include <set>
#include "fingerlib-datatypes.h"
#include "coefficient-table.h"

#define CURRENT_FILE_FORMATVERSION 1
using namespace std;

class DatabaseWideStats;
class DurationPrint
{
	private:
	bool finalized;
	u_int32_t file_format_version_number;

	public:
	driver_identifier identifier;
	u_int32_t total_number_of_packets_durations_analyzed;
	u_int8_t CTS;  //set if we see a CTS destined to ourselves.

	//any of the following fields are set then atleast one packet was seen
	//with teh flag true throughout the exchange
	u_int8_t pwrmgmt; 
	u_int8_t frag; //set if we see MF set on any outgoing packets.
	u_int8_t order;

	TS2DurationProbRecListType TS2DurationProbRecList; // (type, subtype) => (list of duration prob records)
	private:
	bool add_duration_observation(u_int8_t type, u_int8_t sub, u_int16_t duration);

	public:
	void Finalize();
	int UpdateWithPackets(u_int8_t src[6],  Packet_80211List &Plist); //returns number of packets analyzed
	//all functions below this line cant be called unless finalized is set;
	set<int> get_simple_duration_list(); 


	bool AppendToFp(FILE *fp);
	bool ReadFromFp(FILE *fp);
	DurationPrint();
	void OutputText();	
	void OutputText(FILE *);	


	double get_prob(int duration); //get probability of seeing duration anywhere
	double get_prob(int type, int sub, int duration); //probability of seeing (t,s,duration) relative to every other (t,s,duration)
	double get_prob(type_subtype_t , int duration);
	//-----

	u_int32_t get_unique(); //return total numver of unqiue (type_subtype, duration) pairs in print; 			 t_s = *, dur = *

	//does this implementation ever use this duration value?
	//of so, how many unique type_subtypes does it show up in.
	u_int32_t get_unique(int duration); ///return total number of unique (type_subtype, duration) paris with 	 t_s = * dur = d;


	//does this implementation ever use this type_subtype?
	//if so, ho wmany unique durations get used with it.
	u_int32_t get_unique(int t, int s); //return total number of unique (type_subtpe, duration) pairs with 	 	 t_s = t,s, d=*
	u_int32_t get_unique(type_subtype_t t_s); //type_subtype fixed, 

	//does this implementation ever use this type_subtype, with specified duration?
	//can only be 1 (yes) or 0 (no)
	u_int32_t get_unique(int t, int s, int dur); //total number of unique(type_subtype, duration) pairs with 	  t_s = t,s, dur = d
	u_int32_t get_unique(type_subtype_t t_s, int dur); //This can be only one or zero.

	u_int32_t get_count(); //return total number of packets who'se durations ahve been analyzed
	u_int32_t get_duration_count(int d); //total number of packets who have duration d

	u_int32_t get_type_subtype_count(int t, int s); //return total number of packets who'se durations have been analyzed of type t, subype s
	u_int32_t get_type_subtype_count(type_subtype_t t_s); //return total number of packets who'se durations have been analyzed of type t, subype s

	u_int32_t get_type_subtype_duration_count( int t, int s, int d); //return total number of packets of type t, subtyps, s whit dur value d
	u_int32_t get_type_subtype_duration_count( type_subtype_t t_s, int d); //return total number of packets of type t, subtyps, s whit dur value d
	
};	

string type_subtype2string(type_subtype_t t_s) ;


//typedef list<DurationPrint> DurationPrintListType; //moved to extras.h to resolve magic-stats deps

//returns the number of packets actually analyzed to compute print., < 0 == error
int ComputeDurationPrint( u_int8_t addy[6], char *in_filename, DurationPrint &P);
int ComputeDurationPrint( u_int8_t addy[6],  DurationPrint &P);
DurationPrintListType ReadInPrints(char *dir_path, bool verbose=false);

#endif
	
