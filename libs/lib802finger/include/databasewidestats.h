#ifndef _DATABASEWIDESTATS_H
#define _DATABASEWIDESTATS_H
#include <map>
#include "fingerlib-datatypes.h"
#include "duration-print.h"



//this class takes a pointer to a list of duration prints.
//this means it must compute everything dyanmically, no caching.
class DatabaseWideStats
{

	public:
	DurationPrintListType *DurationPrintsList;

	//--flags related stuff.
	u_int32_t num_prints();
	u_int32_t num_with_cts_set();
	u_int32_t num_with_pwrmgmt_set();
	u_int32_t num_with_frag_set();
	u_int32_t num_with_order_set();
	void PrintFlagsSummary();
	
	//return total number of unqiue (driver, type_subtype, duration) pairs in DB;       
    u_int32_t get_unique(); 															//imp=*, t_s = *, dur= *
    //does any implementation ever use this duration value?
    //if so, how many unique (implementations, type_subtype) does it show up in.
	u_int32_t get_unique(int duration); 												//imp=*,  t_s=*, dur=d

    //does any implementation ever use this type_subtype?
    //if so, ho wmany unique (implementation, type_subtype, durations) get used with it.
    u_int32_t get_unique(int t, int s); 												//imp=*, t_s = t,s, d=*
    u_int32_t get_unique(type_subtype_t t_s); 

    //does any implementation ever use this type_subtype, with specified duration?
	//if yes, how many?
    u_int32_t get_unique(int t, int s, int dur); 										//imp=*, t_s=t,s, dur = d
    u_int32_t get_unique(type_subtype_t t_s, int dur); 
	 void PrintDurationSummary();

	void analyze(DurationPrintListType *P) { DurationPrintsList = P;}


    u_int32_t get_count(); //return total number of packets who'se durations ahve been analyzed
    u_int32_t get_duration_count(int d); //total number of packets who have duration d

//  u_int32_t get_type_count(int t); //return total number of packets who'se durations have been analyzed with type == t
//  u_int32_t get_type_duration_count(int t, int d);//return total number of packets who's type = t, dur = d

    u_int32_t get_type_subtype_count(int t, int s); //return total number of packets who'se durations have been analyzed of type t, subype s
    u_int32_t get_type_subtype_count(type_subtype_t t_s); //return total number of packets who'se durations have been analyzed of type t, subype s

    u_int32_t get_type_subtype_duration_count( int t, int s, int d); //return total number of packets of type t, subtyps, s whit dur value d
    u_int32_t get_type_subtype_duration_count( type_subtype_t t_s, int d); //return total number of packets of type t, subtyps, s whit dur value d 

};
#endif
