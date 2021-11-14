#ifndef _COEFFICIENT_TABLE_H
#define _COEFFICIENT_TABLE_H
#include "airware.h"
#include <list>
#include <map>
#include <string>

using namespace std;
class DurationPrint; //forward decl


class Coefficient_Table
{
	public:
	u_int32_t alg_num; //1-4
	u_int32_t flag_coefficient;
	u_int32_t durations_match_coefficient;
	u_int32_t durations_not_match_coefficient;

	u_int32_t durations_type_subtype_match_coefficient;
	u_int32_t durations_type_subtype_not_match_coefficient;
	//--negative coefficients?
	Coefficient_Table();
	bool AppendToFile(char *fname, char *header=NULL);
	bool ReadFromFile(char *fname);

	bool AppendToFp(FILE *FP, char *header=NULL);
	bool ReadFromFp(FILE *FP);
	string toString();
	void OutputText(FILE *fp=stdout);
};

class Computation_Parameters
{
	public:
	int max_algnum_to_try;
	
	int flag_coefficient_bottom;
        int flag_coefficient_top;
        int flag_coefficient_delta;

        int durations_match_bottom;
        int durations_match_top;
        int durations_match_delta;

        int durations_not_match_bottom;
        int durations_not_match_top;
        int durations_not_match_delta;

        int type_sub_match_bottom;
        int type_sub_match_top;
        int type_sub_match_delta;

        int type_sub_not_match_bottom;
        int type_sub_not_match_top;
        int type_sub_not_match_delta;

	string toString();
	
};
void generate_coefficient_tables(list<Coefficient_Table> &L, Computation_Parameters P);


#endif
	
