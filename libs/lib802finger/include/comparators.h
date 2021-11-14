#ifndef _COMPARATORS_H
#define _COMPARATORS_H
#include "airware.h"
#include <list>
#include <map>
#include <set>
#include "fingerlib-datatypes.h"
#include "databasewidestats.h"
#include "coefficient-table.h"


const int SimpleComparator = 1;
const int MediumComparator = 2;
const int ComplexComparator = 3;
const int FuzzyComparator_one = 4;
const int BayesianComparator =5;
const int OverlyPickyBayesianComparator  = 6; 
const int MEGA_COMPARATOR_DEFAULT = MediumComparator;
const int MAX_MEGA_COMPARATOR_NUM = 7;
using namespace std;

class DatabaseWideStats;
class DurationPrint;

class MegaComparator
{
	private:
	int Comparator_number;
	bool compare_durations;
	bool compare_packettype_durations;
	bool bayes_skip_zeros;
	DatabaseWideStats *DbStats;
	Coefficient_Table *Coeff_table; //FuzzyCompare
	
	protected: //These are all the comparators implemented
	//--My twisted weird one that has coefficients brute forced to get good results.
	double JonsFuzzyCompare(DurationPrint L, DurationPrint P, Coefficient_Table &C, DatabaseWideStats &NewDbStats);

	
	//--V's straight forward, but ill-performing algorithms. Based on basic bayesian probabilities.
	//--This one only compares duration values in L to R, not both.
	double BayesianCompare(DurationPrint L, DurationPrint R);

	//This one compares all duration values contained in L, to those contained in R
	//it also compares all those in R to those in L. Hence, overly-picky
	double OverlyPickyBayesianCompare(DurationPrint L, DurationPrint R);

	//--This is by far the easiest algorithm to understand, and it also performs very well.
	double JonsSimpleCompare(DurationPrint L, DurationPrint R);
	double JonsMediumCompare(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats, Coefficient_Table &C);
	double JonsComplicatedCompare(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats, Coefficient_Table &C);

	private: //these are all helper functions, used in one or more comparator above.
	double JonsDurationMapAnalysisFunction(type_subtype_t t_s, duration_prob_record dur,
											 TS2DurationProbRecListType CompareAgainst, 
											 Coefficient_Table &C, DatabaseWideStats &NewDbStats); 
	//used by both bayesian algorithms.
	double duration_bayesian_compare(DurationPrint L, DurationPrint R ); 
	double duration_type_subtype_bayesian_compare(DurationPrint L, DurationPrint R );

	//--Helpers to SimpleCompare
	double Simple_Intersect_duration(DurationPrint L, DurationPrint R);  
	double Simple_Intersect_type_subtype_duration(DurationPrint L, DurationPrint R);

	//--Helpers to Medium/ComplexCompare
	double Weighted_Intersect_duration(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats);
	double Weighted_Intersect_type_subtype_duration(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats);
	//---Medium/ComplexCompare are the only algorithms that attempt to analyze data points
	//---not in the intersection of the two prints. In order to subtract something they
	//---need to know something about the global probabilitiy of each un-matching data point.
	double Weighted_Minus_duration    (DurationPrint L, DurationPrint R,  DatabaseWideStats &DbWideStats); 	
	double Weighted_Minus_type_subtype_duration(DurationPrint L, DurationPrint R,     DatabaseWideStats &DbWideStats);
	//----

	public:
	void set_comparator(int c) { Comparator_number = c;}
	void set_compare_durations(bool b) { compare_durations = b;}
	void set_compare_packettype_durations(bool b) { compare_packettype_durations = b;}
	void set_bayes_skip_zeros(bool b) { bayes_skip_zeros = b;}
	void Init(DatabaseWideStats *_NewDbStats,  Coefficient_Table *_Coeff_table, bool durations=true, bool packettype_durations=true, bool bayes_skip_zeros = false);
	double Compare(DurationPrint L, DurationPrint R);
	string current_algorithm();
};
	

//---helpers
bool plus_or_minus(float a, float b, float margin);
double compute_duration_matchiness(DurationPrint L, DurationPrint R, int alg_num);
string algorithm_number2name(int num);
string list_megacomparator_algorithms();
#endif
	
