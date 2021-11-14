#include <math.h>
#include "comparators.h"



bool plus_or_minus(float a, float b, float margin)
{
	if ( (b < (a + margin)) && (b > (a - margin)) )
		return true;
	else
		return false;
}

void MegaComparator::Init(DatabaseWideStats *_DbStats,  Coefficient_Table *_Coeff_table, bool _compare_durations, bool _compare_packettype_durations, bool _bayes_skip_zeros)
{
	DbStats = _DbStats;
	Coeff_table = _Coeff_table;
	compare_durations = _compare_durations;
	compare_packettype_durations = _compare_packettype_durations;
	bayes_skip_zeros = _bayes_skip_zeros;

}

double MegaComparator::Compare(DurationPrint L, DurationPrint R)
{
	switch (Comparator_number)
	{
		case SimpleComparator: return JonsSimpleCompare(L,R);
		case MediumComparator: return  JonsMediumCompare(L, R, *DbStats, *Coeff_table);
		case ComplexComparator: return JonsComplicatedCompare(L, R, *DbStats, *Coeff_table);
		case FuzzyComparator_one: return JonsFuzzyCompare(L,R,  *Coeff_table, *DbStats);
		case BayesianComparator: return BayesianCompare(L,R);
		case OverlyPickyBayesianComparator:	 return OverlyPickyBayesianCompare(L,R);

		default: printf("MegaComparator::Compare:Error. invalid algorithm number!\n"); exit(0);
	};

}

double MegaComparator::OverlyPickyBayesianCompare(DurationPrint L, DurationPrint R)
{
	double L_duration_only_score;
	double R_duration_only_score;
	double duration_only_score;

	double L_type_subtype_duration_score;
	double R_type_subtype_duration_score;
	double type_subtype_duration_score;

	L_duration_only_score = duration_bayesian_compare(L,R);
	R_duration_only_score = duration_bayesian_compare(R,L);
	duration_only_score = L_duration_only_score * R_duration_only_score;

	L_type_subtype_duration_score = duration_type_subtype_bayesian_compare(L,R);
	R_type_subtype_duration_score = duration_type_subtype_bayesian_compare(R,L);
	type_subtype_duration_score = L_type_subtype_duration_score * R_type_subtype_duration_score; //XXX: overly picky.
	

//printf("(%d-%d) bayesian-compare: %1.8f + %1.8f = %1.8f\n", L.identifier.id, R.identifier.id, duration_only_score, type_subtype_duration_score,
	//															 duration_only_score + type_subtype_duration_score);

	double ret  = 0;
	if (compare_durations)
		ret += duration_only_score;
	if (compare_packettype_durations)
		ret += type_subtype_duration_score;

	return ret;
	
}

double MegaComparator::BayesianCompare(DurationPrint L, DurationPrint R)
{
	double L_duration_only_score = duration_bayesian_compare(L,R);
	double L_type_subtype_duration_score = duration_type_subtype_bayesian_compare(L,R);

	double ret = 0;
	if (compare_durations)
		ret += L_duration_only_score;
	if (compare_packettype_durations)
		ret += L_type_subtype_duration_score;
	return ret;

}

//this deals solely with durations, not (type_subtype_t', durations)
double MegaComparator::duration_bayesian_compare(DurationPrint L, DurationPrint R )
{
	double ret = 1.0;

	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			//As soon as we see a single non-match, the whole score goes to zero.
			//Originally I experimented with just skipping nonmatchers (which lead
			//to he Bayesian algorithms with skip in their name. but this did terrible
			if ( R.get_duration_count(L_list_iter->value) == 0)
			{
				if (!bayes_skip_zeros)
					return 0;
				else
				{
		   			L_list_iter++; //"Skip-Bayesian" flavors
					continue;
				}
			}
			//If we get here both L and R have a matching duration value.

			float R_prob; //the odds of see a -packet-  (type_subtype = *, duration=d) across all -packets-
			R_prob = R.get_duration_count( L_list_iter->value) * 1.0 /
					 R.get_count();

			ret *= R_prob;
            L_list_iter++;
        }
        L_map_iter++;
    } 


	//we need this thing to return such that higher == better	
	//return ret * TABLE_LOOKUP[id];
	return ret;

}

double MegaComparator::duration_type_subtype_bayesian_compare(DurationPrint L, DurationPrint R)
{
	double ret = 1.0;

	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			//As soon as we see a single non-match, the whole score goes to zero.
			//Originally I experimented with just skipping nonmatchers (which lead
			//to he Bayesian algorithms with skip in their name. but this did terrible
			if ( R.get_type_subtype_count(L_map_iter->first) == 0)
			{
				if (!bayes_skip_zeros)
					return 0;//XXX				
				else
				{
		   			L_list_iter++; //"skip"
					continue;
				}
			}

			//If we get here both L and R have a matching duration value.
			float R_prob; //the odds of see a  -packet- (type_subtype, duration) across all matching -packets- with (type_subtype)
			R_prob = R.get_type_subtype_duration_count(L_map_iter->first, L_list_iter->value) * 1.0 /
					 R.get_type_subtype_count(L_map_iter->first);


			ret *= R_prob;
            L_list_iter++;
        }
        L_map_iter++;
    } 


	return ret;

}




//---Stat jons SimpleCompare--
//double Intersect_type_subtype_duration(DurationPrint L, DurationPrint R);
//double Intersect_duration(DurationPrint L, DurationPrint R);
double MegaComparator::Simple_Intersect_duration(DurationPrint L, DurationPrint R)
{
	double running_total = 0;
	double ret = 0;
	//For every (dur) in L
		//Does R have a matching entry?
		//Great: ret +=  1.0 - fabs (L.probability of seeing a packet with dur, relative to any packets at all -
		//					   R.probability of seeing a packet with dur, relatvie to any packets at all)

	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			if ( R.get_duration_count(L_list_iter->value) == 0)
			{//R has no packets of this duration look at. moving on instead of 
			 //deducting points. 
		   		L_list_iter++;
				continue;
			}


			float L_prob;  //the odds of seeing a -packet- (type_subtype=*, duration=d) across all -packets-
			L_prob = L.get_duration_count( L_list_iter->value) * 1.0 /
					 L.get_count();

			float R_prob; //the odds of see a -packet-  (type_subtype = *, duration=d) across all -packets-
			R_prob = R.get_duration_count( L_list_iter->value) * 1.0 /
					 R.get_count();

			ret = (1.0 - fabs(L_prob - R_prob));	//1.0 = perfectm 0 = opposites
			//printf("ret = 1.0 - fabs(%f - %f) = %f\n", L_prob, R_prob, ret);

			running_total += ret;
            L_list_iter++;
        }
        L_map_iter++;
    } 

	return running_total;
}
double MegaComparator::Simple_Intersect_type_subtype_duration(DurationPrint L, DurationPrint R)
{
	double running_total = 0;
	double ret = 0;
	//For every (type_subtype_t, dur) in L
		//Does R have a matching entry?
		//Great: ret +=  1.0 - fabs ( L.probability of seeing a packet with duration dur amoung all packets of type_subtype -
		//					        R.probability of seeing a packet with duration dur amoung all packets of type_subype)
	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			if ( R.get_type_subtype_count(L_map_iter->first) == 0)
			{//R has no packets of this type_subtype to look at. moving on instead of 
			 //deducting points. 
		   		L_list_iter++;
				continue;
			}


			float L_prob; //the odds of seeing a -packet - (type_subtype, duration), across all -packets- (type_subtype)
			L_prob = L.get_type_subtype_duration_count(L_map_iter->first, L_list_iter->value) * 1.0 /
					 L.get_type_subtype_count(L_map_iter->first);

			float R_prob; //the odds of see a  -packet- (type_subtype, duration) across all matching -packets- with (type_subtype)
			R_prob = R.get_type_subtype_duration_count(L_map_iter->first, L_list_iter->value) * 1.0 /
					 R.get_type_subtype_count(L_map_iter->first);

			ret =  (1.0 - fabs(L_prob - R_prob));	//1.0 = perfectm 0 = opposites
			running_total += ret;

			
            L_list_iter++;
        }
        L_map_iter++;
    } 

	return running_total;
}

double MegaComparator::JonsSimpleCompare(DurationPrint L, DurationPrint R)
{
	//if (L.pwrmgmt !=  R.pwrmgmt)
	//	return 0; //smae as V_s_Bayesian
	//double duration_matchiness= compute_duration_matchiness(L,R, C.alg_num);
	//---start doing specific <type sub> comparisons
		
	double intersect_type_subtype_durations  =   Simple_Intersect_type_subtype_duration(L,R);
	double intersect_durations 	= Simple_Intersect_duration(L,R );
	

	//printf("%d-%d:  %f + %f = %f \n", L.identifier.id, R.identifier.id, intersect_type_subtype_durations, intersect_type_subtype_durations);
	//return intersect_type_subtype_durations;
	//ret = intersect_durations;

	double  ret = 0;
	if (compare_durations)
		ret += intersect_durations;
	if (compare_packettype_durations)
		ret += intersect_type_subtype_durations;
	return ret;
}



//-----Medium/ComplexCompare lie below----
double MegaComparator::JonsMediumCompare(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats, Coefficient_Table &C)
{
	//if (L.pwrmgmt !=  R.pwrmgmt)
	//	return 0; //smae as V_s_Bayesian
	//double duration_matchiness= compute_duration_matchiness(L,R, C.alg_num);
	//---start doing specific <type sub> comparisons
		
	double intersect_type_subtype_durations  =   Weighted_Intersect_type_subtype_duration(L,R, DbWideStats);
	double intersect_durations 	=  			 	 Weighted_Intersect_duration(L,R, DbWideStats);
	double  ret = 0;
	if (compare_durations)
		ret += intersect_durations;
	if (compare_packettype_durations)
		ret += intersect_type_subtype_durations;
	return ret;
	
}
double MegaComparator::JonsComplicatedCompare(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats, Coefficient_Table &C)
{
	//if (L.pwrmgmt !=  R.pwrmgmt)
	//	return 0; //smae as V_s_Bayesian
	//double duration_matchiness= compute_duration_matchiness(L,R, C.alg_num);
	//---start doing specific <type sub> comparisons
		
	double intersect_type_subtype_durations  =   Weighted_Intersect_type_subtype_duration(L,R, DbWideStats);
	double L_minus_R_type_subtype_durations  =   Weighted_Minus_type_subtype_duration(L,R, DbWideStats);
	double R_minus_L_type_subtype_durations  =   Weighted_Minus_type_subtype_duration(R,L, DbWideStats);


	//printf("%d-%d:  %f %f %f = %f \n", L.identifier.id, R.identifier.id, intersect_type_subtype_durations, 
	//								   L_minus_R_type_subtype_durations, R_minus_L_type_subtype_durations,
	//								   intersect_type_subtype_durations - L_minus_R_type_subtype_durations- R_minus_L_type_subtype_durations);

	double intersect_durations 	=  Weighted_Intersect_duration(L,R, DbWideStats);
	double L_minus_R_durations  =  Weighted_Minus_duration(L,R, DbWideStats);
	double R_minus_L_durations  =  Weighted_Minus_duration(R,L, DbWideStats);

	//printf("%d-%d:  %f %f %f = %f \n", L.identifier.id, R.identifier.id, intersect_durations, 
	//								   L_minus_R_durations, R_minus_L_durations,
	//								   intersect_durations - L_minus_R_durations- R_minus_L_durations);

	double  ret = 0;
	if (compare_durations)
			ret = ret + intersect_durations  - L_minus_R_durations - R_minus_L_durations;
	if (compare_packettype_durations)
		ret = ret + intersect_type_subtype_durations - L_minus_R_type_subtype_durations - R_minus_L_type_subtype_durations;
	
	return ret;
}

double MegaComparator::Weighted_Intersect_duration(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats)
{
	double running_total = 0;
	double ret = 0;
	//For every (dur) in L
		//Does R have a matching entry?
		//Great: ret +=  (1/ (DbWideStats.get_unique(dur) / DbWideStats.get_unique()) //how unique is this ( duration) relative to
																					//to other ( dur=*) in the database?
	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			if ( R.get_duration_count(L_list_iter->value) == 0)
			{//R has no packets of this duration look at. moving on instead of 
			 //deducting points. 
		   		L_list_iter++;
				continue;
			}


			float L_prob; //the odds of seeing a (type_subtype = *, duration = d), across all the (type_subtypes = *, duration = *)
			L_prob = L.get_duration_count( L_list_iter->value) * 1.0 /
					 L.get_count();

			float R_prob; //the odds of see a  (type_subtype = *, duration) across all matching (type_subtypes)
			R_prob = R.get_duration_count( L_list_iter->value) * 1.0 /
					 R.get_count();


			ret =  (1 / (DbWideStats.get_unique(L_list_iter->value) * 1.0 / //the odds of see a  (type_subtype = *, duration) across (type_subtypes=*, dur=*)
						     DbWideStats.get_unique()) ) *
					(1.0 - fabs(L_prob - R_prob));  					 //1.0 = perfectm 0 = opposites

			//experimental
			//ret =  (1 / (DbWideStats.get_duration_count(L_list_iter->value) * 1.0 / //the odds of see a  (type_subtype = *, duration) across (type_subtypes=*, dur=*)
			//		     DbWideStats.get_count()) ) *
					(1.0 - fabs(L_prob - R_prob));  					 //1.0 = perfectm 0 = opposites

			if ( DbWideStats.get_unique(L_list_iter->value)  == 0) 
			{
				
				printf("Intersect sanity checking triggered.\n");
				printf("DbWideStats.get_unique( dur = %d) = %d\n", L_list_iter->value, DbWideStats.get_unique(L_list_iter->value));
				printf("This means you have found a packet with a ( duration)  the database has never sen.\n");
				printf("This is a unique print. Please submit it for assimiliation.\n");
							
			}
			else
			{
				running_total += ret;
			}

			
            L_list_iter++;
        }
        L_map_iter++;
    } 

	return running_total;
}

double MegaComparator::Weighted_Minus_duration(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats)
{
	double ret = 0;
	double running_total = 0;
	int num_skipped = 0;
	int num_docked = 0;
	//For every  dur) in L -not- in R
		//ret += DB.get_unique(type_subtype, dur) / DB.get_unqie(type_subtype)
	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			if ( R.get_duration_count(L_list_iter->value) != 0)
			{//R has packets of this ( duration.)
			 //they were already analyized using Intersec. Continue on.
		   		L_list_iter++;
				num_skipped++;
				continue;
			}
			num_docked++;

			//If L has an extra duration that is very very unique, the fact that R lacks it
			//means that it should lose points. How many points? 
			//the points lost should be proportional to the odd of seeintg the (dur) aross all () in the DB.			
			//if (dur) is very common this will be  a small value.
			//if (dur) is very unqiue, this will be a large value

			ret =  (1 / (DbWideStats.get_unique( L_list_iter->value) * 1.0 /
					   DbWideStats.get_unique()) );


			if ( DbWideStats.get_unique( L_list_iter->value)  == 0) 
			{
				
				printf("Duration onlySubtract sanity checking triggered.\n");
				printf("DbWideStats.get_unique(dur = %d) = %d\n", L_list_iter->value, DbWideStats.get_unique(L_list_iter->value));
				printf("This means you have found a packet with a (duration)  the database has never sen.\n");
				printf("This is a unique print. Please submit it for assimiliation.\n");
							
			}
			else
				running_total += ret;

            L_list_iter++;
        }
        L_map_iter++;
    } 

	return running_total;
}


double MegaComparator::Weighted_Intersect_type_subtype_duration(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats)
{
	double running_total = 0;
	double ret = 0;
	//For every (type_subtype_t, dur) in L
		//Does R have a matching entry?
		//Great: ret +=  (1/ (DbWideStats.get_unique(type_subtype, dur) / DbWideStats.get_unique(type_subtype)) //how unique is this (type_subtype, duration) relative to
																												//to other (type_subtypes, dur=*) in the database?
				// *100 - (fabs L.dur->prob, R.dur->prob) // perfect match = 100 scalar, 0 = none.
				//* 100 for good good measure?
	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			if ( R.get_type_subtype_count(L_map_iter->first) == 0)
			{//R has no packets of this type_subtype to look at. moving on instead of 
			 //deducting points. 
		   		L_list_iter++;
				continue;
			}


			float L_prob; //the odds of seeing a (type_subtype, duration), across all the (type_subtypes)
			L_prob = L.get_type_subtype_duration_count(L_map_iter->first, L_list_iter->value) * 1.0 /
					 L.get_type_subtype_count(L_map_iter->first);

			float R_prob; //the odds of see a  (type_subtype, duration) across all matching (type_subtypes)
			R_prob = R.get_type_subtype_duration_count(L_map_iter->first, L_list_iter->value) * 1.0 /
					 R.get_type_subtype_count(L_map_iter->first);



			ret = (1 / (DbWideStats.get_unique(L_map_iter->first, L_list_iter->value) * 1.0 /
					    DbWideStats.get_unique(L_map_iter->first)) ) *
					(1.0 - fabs(L_prob - R_prob));  					 //1.0 = perfectm 0 = opposites



			if ( DbWideStats.get_unique(L_map_iter->first, L_list_iter->value)  == 0) 
			{
				
				printf("Intersect sanity checking triggered.\n");
				printf("DbWideStats.get_unique(type = %d, sub = %d, dur = %d) = %d\n",
										L_map_iter->first.type, L_map_iter->first.subtype, L_list_iter->value, 
								DbWideStats.get_unique(L_map_iter->first, L_list_iter->value));
				printf("This means you have found a packet with a (type_subtype, duration)  the database has never sen.\n");
				printf("This is a unique print. Please submit it for assimiliation.\n");
							
			}
			else if ( DbWideStats.get_unique(L_map_iter->first) == 0)
			{
				printf("Intersect sanity checking triggered.\n");
				//printf("DbWideWstats.get_unique(type =%d, sub = %d) = %d\n",
				//L_map_iter->first.type, L_map_iter->first.subtype, DbWideStats.get_unique(L_map_iter->first));
				//printf("This means you have found a packet with a type_subtype the database has never sen.\n");
				//printf("What a keeper. Please submit pcap for assimilation into the borg.\n");

			}
			else
			{
				running_total += ret;
			}

			
            L_list_iter++;
        }
        L_map_iter++;
    } 

	return running_total;
}

double MegaComparator::Weighted_Minus_type_subtype_duration(DurationPrint L, DurationPrint R, DatabaseWideStats &DbWideStats)
{
	double ret = 0;
	double running_total = 0;
	int num_skipped = 0;
	int num_docked = 0;
	//For every (type_subtype_t, dur) in L -not- in R
		//ret += DB.get_unique(type_subtype, dur) / DB.get_unqie(type_subtype)
	TS2DurationProbRecListType::iterator L_map_iter;
    DurationProbRecListType::iterator L_list_iter;
    
    L_map_iter = L.TS2DurationProbRecList.begin();
    while (L_map_iter != L.TS2DurationProbRecList.end())
    {
        L_list_iter = L_map_iter->second.begin();
        while (L_list_iter != L_map_iter->second.end())
        {

			if ( R.get_type_subtype_duration_count(L_map_iter->first, L_list_iter->value) != 0)
			{//R has packets of this type_subtype, duration.
			 //they were already analyized using Intersec. Continue on.
		   		L_list_iter++;
				num_skipped++;
				continue;
			}
			num_docked++;

			//If L has an extra duration that is very very unique, the fact that R lacks it
			//means that it should lose points. How many points? 
			//the points lost should be proportional to the odd of seeintg the (type_subtype, dur) aross all (type_subtypes) in the DB.			
			//if (type_subtype, dur) is very common this will be  a small value.
			//if (type_subtype, dur) is very unqiue, this will be a large value

			ret =	(1 / (DbWideStats.get_unique(L_map_iter->first, L_list_iter->value) * 1.0 /
				    DbWideStats.get_unique(L_map_iter->first)) );

			if ( DbWideStats.get_unique(L_map_iter->first, L_list_iter->value)  == 0) 
			{
				
				printf("Subtract sanity checking triggered.\n");
				printf("DbWideStats.get_unique(type = %d, sub = %d, dur = %d) = %d\n",
										L_map_iter->first.type, L_map_iter->first.subtype, L_list_iter->value, 
								DbWideStats.get_unique(L_map_iter->first, L_list_iter->value));
				printf("This means you have found a packet with a (type_subtype, duration)  the database has never sen.\n");
				printf("This is a unique print. Please submit it for assimiliation.\n");
							
			}
			else if ( DbWideStats.get_unique(L_map_iter->first) == 0)
			{
				printf("DbWideWstats.get_unique(type =%d, sub = %d) = %d\n",
				L_map_iter->first.type, L_map_iter->first.subtype, DbWideStats.get_unique(L_map_iter->first));
				printf("This means you have found a packet with a type_subtype the database has never sen.\n");
				printf("What a keeper. Please submit pcap for assimilation into the borg.\n");
							

			}
			else
				running_total += ret;

            L_list_iter++;
        }
        L_map_iter++;
    } 
	//printf("DurationSubtract Report: %d (%d) vs %d (%d)\n", L.identifier.id, L.get_unique(), R.identifier.id, R.get_unique());;
	//printf("num skipped: %d\n", num_skipped);
	//printf("num docked: %d\n", num_docked);
	//getchar();

	return running_total;
}


//OKay, so heres the simple grading scheme.
//Look through every duration value in each . Do the match? Great. + 100.
//Do they match and are they at the same type ? + 100
//Do they match and have the same subtype ? + another 150 (this reward direct matches mroe)

double MegaComparator::JonsFuzzyCompare(DurationPrint L, DurationPrint P, Coefficient_Table &C, DatabaseWideStats &DbStats)
{
	double  ret = 0;
	if (L.CTS &&  P.CTS)
		ret += C.flag_coefficient * (1.0 / ( DbStats.num_with_cts_set() * 1.0 / DbStats.num_prints())) ;
	if (L.CTS == 0 && P.CTS == 0)
		ret += C.flag_coefficient * ( 1 / ( 1.0 * ( DbStats.num_prints() - DbStats.num_with_cts_set() ) / DbStats.num_prints()));


	if (L.pwrmgmt &&  P.pwrmgmt)
		ret += C.flag_coefficient * (1.0 / ( DbStats.num_with_pwrmgmt_set() * 1.0 / DbStats.num_prints()) );
	if (L.pwrmgmt == 0  && P.pwrmgmt == 0)
		ret += C.flag_coefficient * ( 1 / ( 1.0 * ( DbStats.num_prints() - DbStats.num_with_pwrmgmt_set() ) / DbStats.num_prints()));


	if (L.frag && P.frag)
		ret += C.flag_coefficient * (1.0 / ( DbStats.num_with_frag_set() * 1.0 / DbStats.num_prints()) );
	if (L.frag == 0 && P.frag == 0)
		ret += C.flag_coefficient * ( 1 / ( 1.0 * ( DbStats.num_prints() - DbStats.num_with_frag_set() ) / DbStats.num_prints()));


	if (L.order && P.order)
		ret += C.flag_coefficient * (1.0 / ( DbStats.num_with_order_set() * 1.0 / DbStats.num_prints()) );
	if (L.order == 0  && P.order == 0)
		ret += C.flag_coefficient * ( 1 / ( 1.0 * ( DbStats.num_prints() - DbStats.num_with_order_set() ) / DbStats.num_prints()));


	//ret = 1.0; //XXX;
	double duration_matchiness= compute_duration_matchiness(L,P, C.alg_num);	

	//---start doing specific <type sub> comparisons
	TS2DurationProbRecListType::iterator Left_map_iter;
	DurationProbRecListType::iterator current_duration_prob_rec;
	Left_map_iter = L.TS2DurationProbRecList.begin();

	while (Left_map_iter != L.TS2DurationProbRecList.end())
	{
		current_duration_prob_rec = Left_map_iter->second.begin();
		while (current_duration_prob_rec != Left_map_iter->second.end())
		{
			ret += JonsDurationMapAnalysisFunction( Left_map_iter->first, *current_duration_prob_rec, 
				P.TS2DurationProbRecList, C, DbStats); //type_subtype_t, duration_prob_r

			current_duration_prob_rec++;
		}
		Left_map_iter++;
	}


	return ret * duration_matchiness; //alg1 just sets this to 1.0.
}


//This function is just a helper to JonsFuzzyCompare. Its interface and the results
//it returns should be considered private and only meaningful to JonsFuzzyCompare.
//That said, its job is to take an input type subtype duration_prob_record tuple, and
//compare it against every tuple inside 
double MegaComparator::JonsDurationMapAnalysisFunction(type_subtype_t t_s, duration_prob_record dur, TS2DurationProbRecListType CompareAgainst,  Coefficient_Table &C, DatabaseWideStats &NewStats)
{

	double running_total = 0;
	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	map_iter = CompareAgainst.begin();
	//printf("grading-functionWalking Map with size %d\n", CompareAgainst.size());
	while (map_iter != CompareAgainst.end())
	{
		list_iter = map_iter->second.begin();
		//printf("<%d %d>\t Duration(", map_iter->first.type, map_iter->first.subtype);
		while (list_iter != map_iter->second.end())
		{
			//printf("comparing (%d %d)=>(%d) to (%d %d)=>(%d)\n",
			//		t_s.type, t_s.subtype, dur.value,
			//		 map_iter->first.type, map_iter->first.subtype, list_iter->value);
			//printf(" (%d [%f]) ", list_iter->value, list_iter->prob);
			if (dur.value == list_iter->value) // duration values match;
			{
				//printf("matched duration!\n");
				//running_total +=   (C.durations_match_coefficient * OldDbWideStats.get_weight_for_duration(dur.value)); 
									    //* (1/probability of seeing this duration over universe of known durations)
				running_total +=   (C.durations_match_coefficient * 
									(1.0/ (NewStats.get_unique(dur.value)*1.0/NewStats.get_unique()) ) );
			}
			else
			{
				//running_total -=  (C.durations_not_match_coefficient * OldDbWideStats.get_weight_for_not_duration(dur.value)); 
				running_total -=  C.durations_not_match_coefficient * 
									( 1.0 / (1.0 - ( (NewStats.get_unique(dur.value)  * 1.0) / NewStats.get_unique()) ) )  ;
									//1.0 / (1.0 - [small/big]) ~~ 
									//1.0 / (1.0 - epsilon) ~~
									// 1.epsilon

			}
				

			if (t_s.type == map_iter->first.type && t_s.subtype == map_iter->first.subtype)
			{
				//This seems to actually reduce performance a little..
				//printf("matched subtype!\n");
				//running_total += 150;

				//XXX: is this a conditional probability? sort-of..
				if (dur.value == list_iter->value) 
					//running_total +=  C.durations_type_subtype_match_coefficient * (OldDbWideStats.get_weight_for_duration(dur.value)) ; 
					running_total +=  C.durations_type_subtype_match_coefficient * 
									(1.0/ (NewStats.get_unique(dur.value)*1.0/NewStats.get_unique()) );

				else
				{
					//printf("subtraacting points..\n");
					//running_total -= C.durations_type_subtype_not_match_coefficient * (OldDbWideStats.get_weight_for_not_duration(dur.value));
					running_total -= C.durations_type_subtype_not_match_coefficient * 
									( 1.0 / (1.0 - ( (NewStats.get_unique(dur.value)  * 1.0) / NewStats.get_unique()) ) )  ;

				}
			}
						
			list_iter++;
		}
		map_iter++;
	}
	
	//printf("Returning %f\n", running_total);
	return running_total;
}


//--do generic comparisons
//does the print we are comparing ourselves to contain any durations that we
//ourselves dont have? -minus points.
double compute_duration_matchiness(DurationPrint L, DurationPrint P, int matchy_alg_num)
{
	double duration_matchiness;

	set<int> left_flattened_list =  L.get_simple_duration_list();
	set<int> right_flattened_list = P.get_simple_duration_list();
	set<int> results;
	set<int> right_extra_durations;
	set<int> left_extra_durations;
	set<int> matching_durations;

	set_union( left_flattened_list.begin(), left_flattened_list.end(),
			right_flattened_list.begin(), right_flattened_list.end(), 
			inserter(results, results.begin()) );
	set_difference( results.begin(), results.end(),
			left_flattened_list.begin(), left_flattened_list.end(),
			inserter(right_extra_durations, right_extra_durations.begin()) );
	set_difference( results.begin(), results.end(),
			right_flattened_list.begin(), right_flattened_list.end(),
			inserter(left_extra_durations, left_extra_durations.begin()) );

	set<int> t1;

	set_difference(results.begin(), results.end(),
			right_extra_durations.begin(), right_extra_durations.end(),
			inserter(t1, t1.begin()) );
	
	set_difference( t1.begin(), t1.end(),
			left_extra_durations.begin(), left_extra_durations.end(),
			inserter ( matching_durations, matching_durations.begin()) );

	set<int>::iterator s_iter;
	s_iter = right_extra_durations.begin();

	if (matchy_alg_num < 0 ||matchy_alg_num > 4)
	{
		printf("JonsFuzzyCompare Error. matchy_alg_num passed in out of bounds: %d\n", matchy_alg_num);
		exit(0);
	}
	if (matchy_alg_num == 1)
	{
		duration_matchiness = 1.0;
	}

	if (matchy_alg_num == 2)
	{
		//printf("JonsFuzzyCompare, using Algorithm 2 (matching.size / matching.size + left_extras.size + right_extras.size\n");
		duration_matchiness = (1.0 * matching_durations.size()) /
					(1.0 * matching_durations.size() +  left_extra_durations.size() + right_extra_durations.size()   );

		//printf("Alg-2  fingerprint matchiness: %d / %d = %f\n", matching_durations.size(), 
		//			 matching_durations.size() + left_extra_durations.size() + right_extra_durations.size(),
		//			 duration_matchiness);
	}


	if (matchy_alg_num ==3)
	{
		//printf("JonsFuzzyCompare, using Algorithm 3 (matching.size / matching.size + left_extras.size\n");
		duration_matchiness = (1.0 * matching_durations.size()) /
					(1.0 * matching_durations.size() +  left_extra_durations.size() );

		//printf("Alg-3:  fingerprint matchiness: %d / %d = %f\n", matching_durations.size(), 
		//			 matching_durations.size() + left_extra_durations.size(),
		//			 duration_matchiness);
	}
	if (matchy_alg_num ==4)
	{
		duration_matchiness = (1.0 * matching_durations.size()) /
					(1.0 * matching_durations.size() +  right_extra_durations.size()  );

		//printf("Alg-4: fingerprint matchiness: %d / %d = %f\n", matching_durations.size(), 
		//			 matching_durations.size() + right_extra_durations.size(),
		//			 duration_matchiness);
	}

	return duration_matchiness;
}

string MegaComparator::current_algorithm()
{
	string ret;
	ret += algorithm_number2name(Comparator_number);
	if (compare_durations && compare_packettype_durations)
		ret += "-Combined";
	else if (compare_durations)
		ret += "-Duration-only";
	else if (compare_packettype_durations)
		ret += "-Packettype-duration-only";
	else
		ret += "no-comparisons-enabled";

	if (Comparator_number == BayesianComparator || Comparator_number == OverlyPickyBayesianComparator)
	{
		if (bayes_skip_zeros)
			ret += "-Skip Zeros";
	}
	return ret;

}
string algorithm_number2name(int num)
{
	switch (num)
	{
		case SimpleComparator: return "SimpleCompare";
		case MediumComparator: return  "MediumCompare";
		case ComplexComparator: return "ComplexCompare";
		case FuzzyComparator_one: return "FuzzyComparator_one";
		case OverlyPickyBayesianComparator:	 return "OverlyPickyBayesianCompare";
		case BayesianComparator: return "BayesianCompare";
		default: return "Invalud algorithm number";
	};
}
	
string list_megacomparator_algorithms()
{
	string ret;
	char tmp[2048];
	for (int i = 1; i < MAX_MEGA_COMPARATOR_NUM; i++)
	{
		snprintf(tmp, sizeof(tmp), "%d\t%s",i, algorithm_number2name(i).c_str() );
		ret += tmp;
		if (MEGA_COMPARATOR_DEFAULT == i)
			ret += "*\n";
		else ret += "\n";
	}
	
	return ret;

}


