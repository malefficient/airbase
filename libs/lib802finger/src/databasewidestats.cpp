#include "databasewidestats.h"



u_int32_t DatabaseWideStats::num_prints()
{
	return DurationPrintsList->size();
}

u_int32_t DatabaseWideStats::num_with_cts_set()
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		if (my_iter->CTS)
			ret++;
		my_iter++;
	}
	return ret;

}

u_int32_t DatabaseWideStats::num_with_pwrmgmt_set()
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		if (my_iter->pwrmgmt)
			ret++;
		my_iter++;
	}
	return ret;
}

u_int32_t DatabaseWideStats::num_with_frag_set()
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		if (my_iter->frag)
			ret++;
		my_iter++;
	}
	return ret;
}

u_int32_t DatabaseWideStats::num_with_order_set()
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		if (my_iter->order)
			ret++;
		my_iter++;
	}
	return ret;
}

void DatabaseWideStats ::PrintFlagsSummary()
{   
    printf("--DatabaseWideStats flag summarry---\n");
    int total_volume = num_prints();
    printf("num prints (total volume): %d\n", total_volume);
    
//  printf("CTS: %d\n", num_total_with_cts_set);


	double cts_prob = (1.0 * num_with_cts_set() / total_volume);
    printf("CTS:\t\t1 %3d/%3d%\t%2.4f\t%2.4f  \n", num_with_cts_set(), total_volume,
                                             cts_prob,
                                             1.0 / cts_prob);

	double not_cts_prob = (1.0 * (total_volume - num_with_cts_set()) / total_volume);
    printf("CTS:\t\t0 %3d/%3d\t%2.4f\t%2.4f  \n", (total_volume - num_with_cts_set()), total_volume,
                                             not_cts_prob,
                                             1.0 / not_cts_prob);
    
    printf("\n"); 
	double pwrmgmt_prob = (1.0 * num_with_pwrmgmt_set() / total_volume);
    printf("PwrMgmt:\t1 %3d/%3d%\t%2.4f\t%2.4f  \n", num_with_pwrmgmt_set(), total_volume,
                                             pwrmgmt_prob,
                                             1.0 / pwrmgmt_prob);

	double not_pwrmgmt_prob =  (1.0 * total_volume - num_with_pwrmgmt_set()) / total_volume;
    printf("PwrMgmt:\t0 %3d/%3d\t%2.4f\t%2.4f  \n", (total_volume - num_with_pwrmgmt_set()), total_volume,
                                              not_pwrmgmt_prob,
                                             1.0 / not_pwrmgmt_prob);


    printf("\n");

	double frag_prob = (1.0 *num_with_frag_set() / total_volume);
    printf("frag:\t\t1 %3d/%3d%\t%2.4f\t%2.4f  \n", num_with_frag_set(), total_volume,
											frag_prob,
                                             1.0 / frag_prob);

	double not_frag_prob = (1.0 * (total_volume - num_with_frag_set()) / total_volume);
    printf("frag:\t\t0 %3d/%3d\t%2.4f\t%2.4f  \n", (total_volume - num_with_frag_set()), total_volume,
												not_frag_prob,
                                             1.0 / not_frag_prob);

    printf("\n");

	double order_prob =  (1.0 *num_with_order_set() / total_volume);
    printf("order:\t\t1 %3d/%3d%\t%2.4f\t%2.4f  \n", num_with_order_set(), total_volume,
                                             order_prob,
											1.0 / order_prob);

	double not_order_prob = (1.0 * (total_volume - num_with_order_set()) / total_volume);
    printf("order:\t\t0 %3d/%3d\t%2.4f\t%2.4f  \n", (total_volume - num_with_order_set()), total_volume,
													not_order_prob,
													1.0 / not_order_prob);
}
void DatabaseWideStats ::PrintDurationSummary()
{
    printf("--DatabaseWideStats Duration summarry---\n");

    //loop through once, just to compute total_volume.
    int total_volume = get_unique();
	map<int, int> Duration2UniqueCount;
	map<int, int> Duration2PacketCount;

	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();
	while(my_iter != DurationPrintsList->end()) //for every print we have
	{
		set<int> simple_duration_list = my_iter->get_simple_duration_list();
		set<int>::iterator simple_iter;
	
		simple_iter = simple_duration_list.begin();
		while (simple_iter != simple_duration_list.end())
		{
			Duration2UniqueCount[*simple_iter] = get_unique(*simple_iter);
			Duration2PacketCount[*simple_iter] = get_duration_count(*simple_iter);
			simple_iter++;
		}
		my_iter++;
	}
	


	total_volume = get_unique();
    printf("-------------- counting (implementation = *, packet_type = *, dur = d) tuples----------\n");
    printf("Total number of unique durations: %d\n", Duration2UniqueCount.size());
    printf("Total volume: %d\n", get_unique());
    printf("---------------------------------------------------------------------------------------\n");
    printf("dur\tnum_times_seen\t\tprob\t\tweight\t not-dur-weight\n");

	map<int, int>::iterator Duration_iter;
	Duration_iter = Duration2UniqueCount.begin();
    while (Duration_iter != Duration2UniqueCount.end())
    {
		double curr_duration_prob_using_unique = (Duration_iter->second * 1.0) / total_volume;
		double not_curr_duration_prob_using_unique = (total_volume - Duration_iter->second * 1.0) / total_volume;

        printf("%2d,\t\t%2d,\t\t%2.4f,\t\t%2.4f,\t\t%2.4f\n", Duration_iter->first, Duration_iter->second,
											curr_duration_prob_using_unique,
											1 / curr_duration_prob_using_unique,
											1 / not_curr_duration_prob_using_unique);
        Duration_iter++;
    }
    printf("---------------------------------------------------------------------------------------\n\n");

	total_volume = get_count();
    printf("----------- counting packets with (implementation = *, packet_type = * dur = d)----------\n");
    printf("Total number of unique durations: %d\n", Duration2PacketCount.size());
    printf("Total volume: %d\n", get_count());
    printf("-----------------------------------------------------------------------------------------\n");
    printf("dur\tnum_times_seen\t\tprob\t\tweight\t not-dur-weight\n");
	Duration_iter = Duration2PacketCount.begin();
    while (Duration_iter != Duration2PacketCount.end())
    {
		double curr_duration_prob_using_count = (Duration_iter->second * 1.0) / total_volume;
		double not_curr_duration_prob_using_count = (total_volume - Duration_iter->second * 1.0) / total_volume;

        printf("%2d,\t\t%2d,\t\t%2.4f,\t\t%2.4f,\t\t%2.4f\n", Duration_iter->first, Duration_iter->second,
											curr_duration_prob_using_count,
											1 / curr_duration_prob_using_count,
											1 / not_curr_duration_prob_using_count);
        Duration_iter++;
    }
    printf("---------------------------------------------------------------------------------------\n\n");

}


//return total numver of unqiue (driver, type_subtype, duration) pairs in DB;       
u_int32_t DatabaseWideStats::get_unique()					//imp=*, t_s = *, dur= * 
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_unique();
		my_iter++;
	}
	return ret;
}

//does any implementation ever use this duration value?
//if so, how many unique (implementations, type_subtype) does it show up in.
u_int32_t DatabaseWideStats::get_unique(int duration)		//imp=*,  t_s=*, dur=d
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_unique(duration);
		my_iter++;
	}
	return ret;
}

//does any implementation ever use this type_subtype?
//if so, how many unique (implementation, type_subtype, durations) get used with it.
u_int32_t DatabaseWideStats::get_unique(int t, int s)		//imp=*, t_s = t,s, d=*
{
	type_subtype_t t_s;
	t_s.type =t;
	t_s.subtype = s;
	return get_unique(t_s);

}
u_int32_t DatabaseWideStats::get_unique(type_subtype_t t_s)
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_unique(t_s);
		my_iter++;
	}
	return ret;
}
//does any implementation ever use this type_subtype, with specified duration?
//if yes, how many?
u_int32_t DatabaseWideStats::get_unique(int t, int s, int dur) 	//imp=*, t_s=t,s, dur = d
{
	type_subtype_t t_s;
	t_s.type = t;
	t_s.subtype = s;
	get_unique(t_s, dur);

}
u_int32_t DatabaseWideStats::get_unique(type_subtype_t t_s, int dur) 
{
	
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_unique(t_s, dur);
		my_iter++;
	}
	return ret;

}
///-------------
u_int32_t DatabaseWideStats::get_count()					//imp=*, t_s = *, dur= * 
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_count();
		my_iter++;
	}
	return ret;
}


u_int32_t DatabaseWideStats::get_duration_count(int duration)		//imp=*,  t_s=*, dur=d
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_duration_count(duration);
		my_iter++;
	}
	return ret;
}
u_int32_t DatabaseWideStats::get_type_subtype_count(int t, int s)		//imp=*,  t_s=t,s, dur=*
{
	type_subtype_t t_s;
	t_s.type =t;
	t_s.subtype =s;
	return get_type_subtype_count(t_s);
}
u_int32_t DatabaseWideStats::get_type_subtype_count(type_subtype_t t_s)		//imp=*,  t_s=t,s, dur=*
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_type_subtype_count(t_s);
		my_iter++;
	}
	return ret;
}


u_int32_t DatabaseWideStats::get_type_subtype_duration_count(int t, int s, int duration)		//imp=*,  t_s=t,s, dur=d
{
	type_subtype_t t_s;
	t_s.type =t;
	t_s.subtype =s;
	return get_type_subtype_duration_count(t_s, duration);
}

u_int32_t DatabaseWideStats::get_type_subtype_duration_count(type_subtype_t t_s,int duration)		//imp=*,  t_s=t,s, dur=d
{
	u_int32_t ret = 0;
	DurationPrintListType::iterator my_iter;
	my_iter = DurationPrintsList->begin();

	while(my_iter != DurationPrintsList->end())
	{
		ret += my_iter->get_type_subtype_duration_count(t_s, duration);
		my_iter++;
	}
	return ret;
}

