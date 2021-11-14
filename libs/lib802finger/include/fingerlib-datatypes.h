#ifndef _FINGERLIB_DATATYPES_H
#define _FINGERLIB_DATATYPES_H
#include "airware.h"
#include <list>
#include <map>
#include "implementation-record.h"
using namespace std;
class DurationPrint; //forward decl
//class ImplementationRecord; //forward decl

typedef struct
{
	u_int32_t id;
} driver_identifier;


struct db_filename_struct
{
	char db_name[256]; //e.g. 'lexie'
        char path_to_db[256];
		char path_to_implementation_records[256];
        char path_to_prints[256]; //1.prnt, 2.prnt, etc
        char path_to_pcaps[256]; //1-1-lexie.pcap, 1-2-lexie.pcap, etc
        char pcap_id_to_macaddys_fname[256];
        char coefficient_fname[256]; //output
};

void fill_in_default_filenames(char *path_to_db, struct db_filename_struct &global_cfg);
void print_db_filename_struct(struct db_filename_struct &global_cfg);


/*
typedef struct
{
_ u_int8_t version;
} dur_print_fileheader;
*/
class duration_prob_record  //XXX: the DurationPrint::AppendToFp method is intimately familliar with this struct. should 
	   	//	either phase it out, or give it its own AppendToFp/ReadFromFP methods. Maybe someday. 
{
	public:
	u_int16_t value; //duration value
	u_int32_t cnt;
	//float prob;  	//unused.

	bool operator==( const duration_prob_record right)  const
	{
		if ( (value == right.value))
			return true;
		else
			return false;
	}

};	



class type_subtype_t
{
	public:
	u_int8_t type;
	u_int8_t subtype;


	bool operator<( const type_subtype_t right) const
	{
		if (type == right.type && subtype == right.subtype)
			return false;

		if ( (type < right.type))
		{
			//printf(" (%d, %d) < (%d %d)\n", type, subtype, right.type, right.subtype);
			return true;
		}
		else if (type > right.type)
		{
			//printf(" (%d, %d) >=  (%d %d)\n", type, subtype, right.type, right.subtype);
			return false;
		}	
		//at this point type == right.type
		if ( (subtype < right.subtype))
		{
			//printf(" (%d, %d) < (%d %d)\n", type, subtype, right.type, right.subtype);
			return true;
		}
		else if (subtype > right.type)
		{
			//printf(" (%d, %d) >=  (%d %d)\n", type, subtype, right.type, right.subtype);
			return false;
		}	
		else
		{
			printf("UNHANDLED CASE IN type_subtype_< opert. should never happen!\n");
			exit(0);
		}
		
	}



	bool operator==( const type_subtype_t right)  const
	{
		if ( (type == right.type) && (subtype == right.subtype))
			return true;
		else
			return false;
	}

};	



//typedef list<duration_prob_record> DurationProbListType;
typedef list<duration_prob_record> DurationProbRecListType;
//typedef map<type_subtype_t, DurationProbRecListType > TypeSubtypeToDurationMapType;
typedef map<type_subtype_t, DurationProbRecListType > TS2DurationProbRecListType;
typedef list<DurationPrint> DurationPrintListType;

typedef map<u_int32_t, ImplementationRecord> DriverId2ImplementationRecordType;

FILE * OpenReadPrintFile(char *fname);
FILE * OpenWritePrintFile(char *fname);
DriverId2ImplementationRecordType ReadInDriverInfo(char *fname, bool verbose = false);


#endif
	
