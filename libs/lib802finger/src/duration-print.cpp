#include "duration-print.h"

DurationPrint :: DurationPrint()
{
	file_format_version_number  =  CURRENT_FILE_FORMATVERSION;
	finalized = false;
	CTS = pwrmgmt =frag = order = 0;
	identifier.id = 0; //XXX identifier intimacy
	total_number_of_packets_durations_analyzed = 0;
}

//If we have already seen this duration before do nothing
//If we haven't, add it to the Map
bool DurationPrint :: add_duration_observation(u_int8_t type, u_int8_t sub, u_int16_t duration)
{
	type_subtype_t t_s;
	t_s.type = type;
	t_s.subtype = sub;

	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;

	if (finalized == true)
	{
		printf("DurationPrint::add_duration_observation:Error, Finalized set. Illegal to add any more info.\n");
		exit(0);
	}
	total_number_of_packets_durations_analyzed++;
	
	map_iter = TS2DurationProbRecList.find(t_s);
	if (map_iter == TS2DurationProbRecList.end())
	{
		duration_prob_record temp_record;
		temp_record.value = duration;
		temp_record.cnt= 0; //this wil be incremented shortly.
		DurationProbRecListType TemporaryList;
		TemporaryList.push_back(temp_record);
		//printf("Type: %d, Sub %d not found. Adding\n", t_s.type, t_s.subtype);
		TS2DurationProbRecList[t_s] = TemporaryList; //The map makes its own copy of this list.
		//printf("The new size is %d\n", TS2DurationProbRecList.size());

	}

	map_iter = TS2DurationProbRecList.find(t_s);

	if(map_iter == TS2DurationProbRecList.end())
	{
		printf("DurationPrint::add_observation Failed to locate type_subtype that we just added to map!\n");
		printf("DurationPrint::add_observation (this should never happen.\n");
		OutputText();
		exit(0);
	}
	//printf("Search for %d %d returned %d %d.\n", type, sub, map_iter->first.type, map_iter->first.subtype);

	//At this point map_iter unequivocally points to the entry for type,sub in the map.
	//The list may or may not already have the value we are trying to add to it.
	list_iter = map_iter->second.begin();

	while (list_iter != map_iter->second.end())
	{
		if (list_iter->value ==duration) 
		{								
			list_iter->cnt++; //get_prob divides this out.
			break;
		}
		list_iter++;
	}
	if (list_iter == map_iter->second.end()) // hit the end
	{
		//printf("Adding a new duration for an existing record: <%d %d>, %d\n", type, sub, duration);
		duration_prob_record temp_record;
        	temp_record.value = duration;	
		temp_record.cnt = 1; 
		map_iter->second.push_back(temp_record);
	}
	//At this point the record in question was added, or it already existed.


}
int DurationPrint :: UpdateWithPackets(u_int8_t src[6], Packet_80211List &pList)
{
	int num_tossed = 0;
	int num_analyzed = 0;
	if (finalized == true)
	{
		printf("DurationPrint::UpdateWithPackets:Error, Finalized set. Illegal to add any more info.\n");
		exit(0);
	}
	
	Packet_80211List::iterator myIter;
	myIter = pList.begin();
	u_int8_t packet_addy[6];
	while (myIter != pList.end())
	{
		//First, check it see if its a ctrl packet, 
		//the only kind of ctrl packet we try to parse is a CTS.
		if (myIter->isControl())
		{
			myIter->getRecvAddr(packet_addy);
			if (myIter->subtype() != CTS) //Skip, not a CTS
			{
				num_tossed++;
				myIter++;
				continue;
			}

			if (memcmp(packet_addy, src, 6) != 0) //CTS packet, but its not directed at me
			{
				num_tossed++;
				myIter++;
				continue;
			}	
			//If we get here then it is a CTS packet, and it is directed at us.
			CTS = true;
			num_analyzed++; 
			myIter++;
			continue;
		}	//---end control branch

		myIter->getSrcAddr(packet_addy); //--At this point, packet_addy contains the source
										 //--address of the packet we are looking at.
	if ( memcmp(packet_addy, src, 6) != 0) //not control, and not transmitted from me.
	{
		num_tossed++;
		myIter++;
		continue;
	}

	//At this point the packet is either a management or data frame with my address in the src.	
	//The first thing to do is check for a few bits, frags most notably.
	if (myIter->pwrmgmt())
		pwrmgmt = true;
	if (myIter->more_frag())
		frag = true;
	//if (myIter->more_data()) //AP telling client its got stuff buffereed. unsupported for now	
		//PowerSaving==true
	if (myIter->order())
		order = true;
	
	//add_obser
	//TS2DurationProbRecListType::iterator map_iter;
	//DurationProbRecListType::iterator list_iter;
	add_duration_observation(myIter->type(), myIter->subtype(), myIter->getDuration());
	num_analyzed++;


	myIter++;
	}//end while.

	//printf("DurationPrint::analyze::\n");
	//printf("\tnum_skipped: %d\n", num_tossed);
	//printf("\tnum_analyzed: %d\n", num_analyzed);
	return num_analyzed;
	
}

void DurationPrint::OutputText()
{
	OutputText(stdout);	
}

void DurationPrint::OutputText(FILE *fp)
{
	if (finalized != true)
	{
		printf("DurationPrint::OutputText:Error, Finalized not set. Illegal to access this member function.\n");
		exit(0);
	}


	fprintf(fp, "Identifier: %d, Num-durations-analyed: %d\n", identifier.id, total_number_of_packets_durations_analyzed);
	fprintf(fp, "CTS: %d\n", CTS);	
	fprintf(fp, "pwrmgmt: %d\n", pwrmgmt);	
	fprintf(fp, "frag: %d\n", frag);	
	fprintf(fp, "order: %d\n", order);	
	fprintf(fp, "---------\n");


	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	map_iter = TS2DurationProbRecList.begin();
	printf("Total packet types: %d\n", TS2DurationProbRecList.size());
	printf("Unique (packet_type, duration) tuples:  %d\n", get_unique());
	int total_volume = 0;
	fprintf(fp, "-----------------+------------------------\n");
	printf("%16s | (duration value [prob])\n", "Packet-Type");
	fprintf(fp, "-----------------+------------------------\n");

	while (map_iter != TS2DurationProbRecList.end())
	{
		list_iter = map_iter->second.begin();
		printf("%16s | ", type_subtype2string(map_iter->first).c_str());
		while (list_iter != map_iter->second.end())
		{
			printf("(%3d [%3.0f%%]) ", list_iter->value, 
									   list_iter->cnt * 100.0 / get_type_subtype_count(map_iter->first));	
		//	printf(" (%d [ (%d/%d)%2.0f%%]) ", list_iter->value, list_iter->cnt, get_type_subtype_count(map_iter->first),
		//								 	 list_iter->cnt * 100.0 / get_type_subtype_count(map_iter->first));	
			total_volume++;
			list_iter++;
		}
		printf("\n");
		map_iter++;
	}
	set<int>::iterator s_iter;
	set<int> S;
	S = get_simple_duration_list();
	s_iter = S.begin();
	fprintf(fp, "-----------------+------------------------\n");
	printf("probs with get_count:  ");
	while (s_iter != S.end())
	{	
		printf("(%3d: [%3.0f%%]) ", *s_iter,  get_duration_count(*s_iter)*100.0/get_count() );
		//printf("(%d: [%d/%d=%2.4f%%])  ", *s_iter, get_duration_count(*s_iter), get_count(),  get_duration_count(*s_iter)*100.0/get_count() );
		s_iter++;
	}
	printf("\n");

	s_iter = S.begin();
	printf("probs with get_unique: ");
	while (s_iter != S.end())
	{	
		printf("(%3d: [%3.0f%%]) ", *s_iter, get_unique(*s_iter)*100.0/get_unique() );
		//printf("(%d: [%d/%d=%2.4f%%])  ", *s_iter, get_unique(*s_iter), get_unique(), get_unique(*s_iter)*100.0/get_unique() );
		s_iter++;
	}
	printf("\n");


	//printf("probabilitiy of seeing a probe request <0, 4> with duration 0, relative to any other beacon: %d / %d\n",
	//		 get_type_subtype_duration_count(0,4,0),get_type_subtype_count(0,4));

	//printf("probabilitiy of seeing a duration of value 0 relative to any other packet: %d / %d\n",
	//		 get_duration_count(0),get_count());


}
bool DurationPrint :: AppendToFp(FILE *fp)
{
	if (finalized != true)
	{
		printf("DurationPrint::AppendToFp:Error, Finalized not set. Illegal to access this member function.\n");
		exit(0);
	}


	bool ret = true;

	u_int32_t int_to_write;
	u_int16_t short_to_write;
	
	int_to_write = htonl(file_format_version_number);
	fwrite( (void *) &int_to_write, sizeof(int_to_write), 1, fp);

	int_to_write = htonl(identifier.id);
	fwrite( (void *) &int_to_write, sizeof(int_to_write), 1, fp);

	int_to_write = htonl(total_number_of_packets_durations_analyzed);
	fwrite( (void *) &int_to_write, sizeof(int_to_write), 1, fp);

	fwrite( (void *) &CTS, sizeof(CTS),1,fp); //1 byte, no ordering issues.
	fwrite( (void *) &pwrmgmt, sizeof(pwrmgmt),1,fp);
	fwrite( (void *) &frag, sizeof(frag),1,fp);
	fwrite( (void *) &order, sizeof(order),1,fp);

	u_int32_t num_total = TS2DurationProbRecList.size();
	int_to_write = htonl(num_total);
	//fprintf(stderr, "writing out num_total %d (type_subtype, <list>) record\n", num_total);	
	fwrite( (void *) &int_to_write, sizeof(int_to_write),1,fp);
	//fwrite( (void *) &num_total, sizeof(num_total),1,fp);


	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	map_iter = TS2DurationProbRecList.begin();


	for (int i = 0; i < num_total; i++)
	{
		//the first thing we write is the type_subtype info out.
		fwrite( (void *) & map_iter->first.type, sizeof (map_iter->first.type), 1, fp);
		fwrite( (void *) & map_iter->first.subtype, sizeof (map_iter->first.subtype), 1, fp);
		//the next thing to do is write out the number of duration_prob_records
		//both of these are 1 byte, so no ordering issues

		u_int32_t num_in_this_list = map_iter->second.size();
		int_to_write = htonl(num_in_this_list);
		fwrite( (void *) &int_to_write, sizeof(int_to_write),1,fp);
		//fwrite( (void *) &num_in_this_list, sizeof(num_in_this_list) , 1, fp);

		list_iter = map_iter->second.begin();
		for (int j = 0; j < num_in_this_list; j++) //lay down the [duration-value, prob] pairs.
		{
			duration_prob_record temp_list_record = *list_iter; //if i take that address of *(iter) will i get what i want?
			short_to_write = htons(temp_list_record.value);
			if (fwrite( (void *) &short_to_write, sizeof(short_to_write), 1, fp) != 1)
			{
				fprintf(stderr, "DurationPrint::AppendToFp::Errror. Error writing value in list.\n");
				perror("fwrite");
				return false;
			}

			//XXX: if doubles have endian issues, this breaks.
			int_to_write = htonl(temp_list_record.cnt);
			if (fwrite( (void *) &int_to_write, sizeof(int_to_write), 1, fp) != 1)
			{
				fprintf(stderr, "DurationPrint::AppendToFp::Errror. Error writing value in list.\n");
				perror("fwrite");
				return false;
			}

			list_iter++;
		}
		map_iter++;
		
	}

	return true;
}

bool DurationPrint :: ReadFromFp(FILE *fp)
{

	//When calling this, we overwrite the current state, we do not add to it.
	//Though in theory we could have this set finalized to false, until we have a program
	//that can acutally merge print files, we call finalize at the bottom.
	
	if (finalized == true)
	{
		//printf("DurationPrint::ReadFromFp:Warning, Finalized set. Calling ReadFromFp is overwriting the state.to access this member function.\n");
	}

	//XXX:is this important? does it break things!. not sure. keep an eye on it.
	TS2DurationProbRecList.clear();


	int fread_total = 0;

	u_int32_t int_to_read;
	u_int16_t short_to_read;
	
	fread( (void *) &int_to_read, sizeof(int_to_read), 1, fp); 
	file_format_version_number = ntohl(int_to_read);
	if (file_format_version_number !=  CURRENT_FILE_FORMATVERSION)
	{
		printf("Error, this print has file version: %d\n", file_format_version_number);
		printf("But This version of the library can only read %d version. Exiting\n", CURRENT_FILE_FORMATVERSION);
		exit(0);
	}

	fread( (void *) &int_to_read, sizeof(int_to_read), 1, fp); 
	identifier.id = ntohl(int_to_read);//XXX: identifier intimacy
	


	//fread( (void *) &identifier.id, sizeof(identifier.id), 1, fp); //XXX: identifer intimacy 
	fread( (void *) &int_to_read, sizeof(int_to_read), 1, fp); 
	total_number_of_packets_durations_analyzed = ntohl(int_to_read);

	fread( (void *) &CTS, sizeof(CTS),1,fp);//XXX: on byte, no ordering issues
	fread( (void *) &pwrmgmt, sizeof(pwrmgmt),1,fp);
	fread( (void *) &frag, sizeof(frag),1,fp);
	fread( (void *) &order, sizeof(order),1,fp);

	u_int32_t num_total = 0;
	fread( (void *) &int_to_read, sizeof(int_to_read), 1, fp); 
	num_total = ntohl(int_to_read);
	//fread( (void *) &num_total, sizeof(num_total),1,fp);

	//fprintf(stderr, "reading  out num_total  %d  (type_subtype, <list>) record\n", num_total);	

	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	map_iter = TS2DurationProbRecList.begin();

	for (int i = 0; i < num_total; i++)
	{
		type_subtype_t CurrRec;
		DurationProbRecListType CurrList;

		duration_prob_record temp_list_entry;
		//the first thing we read is the type_subtype info out.
		fread( (void *) &CurrRec.type, sizeof (CurrRec.type), 1, fp); //One byte, no ordering issues
		fread( (void *) &CurrRec.subtype, sizeof (CurrRec.type), 1, fp);
		//fprintf(stderr,"Read (%d %d)\n", CurrRec.type, CurrRec.subtype);
		//the next thing to do is read out the number of duration_prob_records
		u_int32_t num_in_this_list;

		int ret = fread( (void *) &int_to_read, sizeof(int_to_read), 1, fp); 
		if (ret != 1)
		{
			fprintf(stderr, "Error reading in num_in_this_list.\n");
			fprintf(stderr, "fread returned %d\n", ret);
			perror("fread");
			exit(0);

		}
		num_in_this_list = ntohl(int_to_read);
		//fread( (void *) &num_in_this_list, sizeof(num_in_this_list), 1, fp);
		//printf("reading in a list of size %d\n", num_in_this_list);


		for (int j = 0; j < num_in_this_list; j++) //read in the written out [duration-values, prob] list,
		{

			int ret = fread( (void *) &short_to_read, sizeof(short_to_read), 1, fp);
			if (ret != 1)
			{
				fprintf(stderr, "DurationPrint::ReadFromFp::Errror. Error readingvalue in list.\n");
				fprintf(stderr, "fread retunred %d\n", ret);
				perror("fread");
				return false;
			}
			temp_list_entry.value = ntohs(short_to_read);

			ret = fread( (void *) &int_to_read, sizeof(int_to_read), 1, fp); 
			if (ret != 1)
			{
				fprintf(stderr, "DurationPrint::ReadFromFp::Errror. Error reading cnt in list.\n");
				printf("fread returned %d\n", ret);
				perror("fread");
				return false;
			}
			temp_list_entry.cnt = ntohl(int_to_read);
		

			//printf(" Duration: %d\n", temp_list_entry.value);
			CurrList.push_back(temp_list_entry);	
		}
		//That does it for this map, entry, set it up.
		//printf("Adding (%d %d)\n", CurrRec.type, CurrRec.subtype);
		TS2DurationProbRecList[CurrRec] = CurrList;		
		//printf("New map size: %d\n", TS2DurationProbRecList.size());
	}
	//printf("--All done reading FP\n");
	//printf("Map.size: %d\n", TS2DurationProbRecList.size());
	Finalize();
	return true;
}

set<int> DurationPrint::get_simple_duration_list()
{
	//Actually, it looks like accessing this before finalize is called 
	//would result in accurate behavior. Still, its bad form and i cant
	//imagine it ever being useful
	if (finalized != true)
	{
		printf("DurationPrint::get_simple_duration_list:Error, Finalized not set. Illegal to access this member function.\n");
		exit(0);
	}
	
	set<int> S;
	
	TS2DurationProbRecListType::iterator my_map_iter;
	DurationProbRecListType::iterator current_duration;
	my_map_iter = TS2DurationProbRecList.begin();
	while (my_map_iter != TS2DurationProbRecList.end())
	{
		current_duration = my_map_iter->second.begin();
		while (current_duration != my_map_iter->second.end())
		{
			S.insert(current_duration->value);
			current_duration++;
		}
		my_map_iter++;
	}

	return S;	
}

u_int32_t DurationPrint::get_unique() //count unique (type_subtype, durations)
{
	u_int32_t sum = 0;

	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	
	map_iter = TS2DurationProbRecList.begin();
	while (map_iter != TS2DurationProbRecList.end())
	{

		list_iter = map_iter->second.begin();
		while (list_iter != map_iter->second.end())
		{
			sum += 1; //new type_subtype, duration 
			list_iter++;
		}
		map_iter++;
	}
	return sum;
}

u_int32_t DurationPrint::get_unique(int d) //count unique (type_subtype, durations), where duration = d, in print
{
	u_int32_t sum = 0;

	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	
	map_iter = TS2DurationProbRecList.begin();
	while (map_iter != TS2DurationProbRecList.end())
	{

		list_iter = map_iter->second.begin();
		while (list_iter != map_iter->second.end())
		{
			if(d ==list_iter->value)
				sum += 1; //new type_subtype, duration that matches.
			list_iter++;
		}
		map_iter++;
	}
	return sum;
}

u_int32_t DurationPrint::get_unique(int t, int s)
{
	type_subtype_t t_s;
	t_s.type = t;
	t_s.subtype = s;
	return get_unique(t_s);
}
u_int32_t DurationPrint::get_unique(struct type_subtype_t t_s)
{
	u_int32_t sum = 0;
	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;

	map_iter = TS2DurationProbRecList.find(t_s);
	if (map_iter == TS2DurationProbRecList.end())
		return 0;

	return map_iter->second.size();
}

u_int32_t DurationPrint::get_unique(int t, int s, int dur)
{
	type_subtype_t t_s;
	t_s.type = t;
	t_s.subtype = s;
	return get_unique(t_s, dur);
}

u_int32_t DurationPrint::get_unique(struct type_subtype_t t_s, int dur)
{
	u_int32_t sum = 0;
	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;

	map_iter = TS2DurationProbRecList.find(t_s);
	if (map_iter == TS2DurationProbRecList.end())
		return 0;

	list_iter = map_iter->second.begin();
	
	while (list_iter != map_iter->second.end())
	{
		if (list_iter->value == dur)
			sum += 1;
		list_iter++;
	}	

	return sum;
}




u_int32_t DurationPrint::get_count() //return total number of packets who'se durations ahve been analyzed 
{
	if (finalized != true)
	{
		printf("DurationPrint::get_count:Error, Finalized not set. Illegal to access this member function.\n");
		exit(0);
	}
	return total_number_of_packets_durations_analyzed;
}

u_int32_t DurationPrint::get_duration_count(int d) //total number of packets who have duration d
{
	u_int32_t sum = 0;

	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	
	map_iter = TS2DurationProbRecList.begin();
	while (map_iter != TS2DurationProbRecList.end())
	{

		list_iter = map_iter->second.begin();
		while (list_iter != map_iter->second.end())
		{
			if(d ==list_iter->value)
				sum += list_iter->cnt;
			list_iter++;
		}
		
		map_iter++;
	}
	return sum;
}


u_int32_t DurationPrint::get_type_subtype_count(int t, int sub)  //return total number of packets who'se durations have been analyzed of type t, subype s
{
	type_subtype_t tmp;
	tmp.type =t;
	tmp.subtype = sub;
	return get_type_subtype_count(tmp);
}


u_int32_t DurationPrint::get_type_subtype_count(struct type_subtype_t t_s) //return total number of packets who'se durations have been analyzed of type t, subype s 
{
	u_int32_t sum = 0;
	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;

	map_iter = TS2DurationProbRecList.find(t_s);
	if (map_iter == TS2DurationProbRecList.end())
		return 0;

	list_iter = map_iter->second.begin();
	
	while (list_iter != map_iter->second.end())
	{
		sum += list_iter->cnt;	
		list_iter++;
	}	

	return sum;
}

//return total number of packets who'se durations have been analyzed of type t, subype s, duration d
u_int32_t DurationPrint::get_type_subtype_duration_count(int t, int s, int dur)
{
	type_subtype_t tmp;
	tmp.type = t;
	tmp.subtype = s;

	return get_type_subtype_duration_count(tmp, dur);

}

//return total number of packets who'se durations have been analyzed of type t, subype s, duration d
u_int32_t DurationPrint::get_type_subtype_duration_count(struct type_subtype_t t_s, int dur)
{
	u_int32_t sum = 0;
	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;

	map_iter = TS2DurationProbRecList.find(t_s);
	if (map_iter == TS2DurationProbRecList.end())
		return 0;

	list_iter = map_iter->second.begin();
	
	while (list_iter != map_iter->second.end())
	{
		if (list_iter->value == dur)
			sum += list_iter->cnt;	
		list_iter++;
	}	

	return sum;
}





double  DurationPrint::get_prob(int duration)
{
	//This is where the obvious problem is. until finalize() is called, get_prob is unknown
	if (finalized != true)
	{
		printf("DurationPrint::get_prob:Error, Finalized not set. Illegal to access this member function.\n");
		exit(0);
	}

	float sum = 0;
	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;

	map_iter = TS2DurationProbRecList.begin();
	while (map_iter != TS2DurationProbRecList.end())
	{
		list_iter = map_iter->second.begin();
		while (list_iter != map_iter->second.end())
		{
			if (list_iter->value == duration)
				sum+= list_iter->cnt;
			list_iter++;
		}
		map_iter++;
	}

	return sum / total_number_of_packets_durations_analyzed;
}

double DurationPrint::get_prob(int t, int s, int duration)
{

	type_subtype_t tmp;
	tmp.type =t;
	tmp.subtype = s;
	return get_prob(tmp, duration);
}


double DurationPrint::get_prob(struct type_subtype_t t_s, int duration)
{
	if (finalized != true)
	{
		printf("DurationPrint::get_prob:Error, Finalized not set. Illegal to access this member function.\n");
		exit(0);
	}

	TS2DurationProbRecListType::iterator map_iter;
	DurationProbRecListType::iterator list_iter;
	
	map_iter = TS2DurationProbRecList.find(t_s);
	if (map_iter == TS2DurationProbRecList.end())
		return 0.0;


	//if we make it here, then duration may be in the list, but maybe not.

	list_iter = map_iter->second.begin();
	while (list_iter != map_iter->second.end())
	{
		if (list_iter->value == duration)
			return list_iter->cnt / total_number_of_packets_durations_analyzed;

		list_iter++;
	}
	return 0.0;

}


//once this is call the duration print is frozen in time, and cant have any 
//new data passed at it to analyze in anyway. 
void DurationPrint::Finalize()
{
	
	finalized = true;
}

DurationPrintListType ReadInPrints(char *dir_path, bool verbose)
{
    DurationPrintListType ret;
	struct dirent *dp;
	DIR *dirp;


	char full_path[1024];
	if (dir_path == NULL)
	{
		printf("lib802finger::ReadInPrints: dir_path == NULL. Invalid path name.\n");	
		return ret;
	}
	if (verbose)
		printf("Reading prints from: %s\n", dir_path);
    dirp = opendir(dir_path);
	if (dirp == NULL)
	{
		printf("lib802finger::ReadInPrints: dirp == NULL. Invalid path name.\n");	
		return ret;
	}
	char *c;
	DurationPrint CurrPrint;
    while ((dp = readdir(dirp)) != NULL)
	{
		c = dp->d_name + strlen(dp->d_name) - 5; 
		if (strcmp(c, ".prnt") != 0) //filter out non prints, including "." and ".."
			continue;
		snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, dp->d_name);

 		FILE *fp;
		if (verbose)
        	printf("loading %s\n", full_path);
         fp = OpenReadPrintFile(full_path);
         if (fp == NULL)
         {
         	printf("Error opening %s\n", full_path);
            continue;
         }
         if (CurrPrint.ReadFromFp(fp) == false)
         {
         	printf("Error reading from %s..not adding to list\n", full_path);
            fclose(fp);
            continue;
         }
         ret.push_back(CurrPrint);
         fclose(fp);                               
     }
     (void)closedir(dirp);
    return ret;
} 

int ComputeDurationPrint( u_int8_t addy[6], char *in_filename, DurationPrint &P)
{

    Pcap_Packet PcapPacket;
    Packet_80211 CurrPacket;
    FILE *out_fp;
    int num_read = 0;
    int num_wrote = 0;
    PcapFileHandle PcapHandle;


    if (!OpenReadPcapFile(in_filename, PcapHandle))
    {
        printf("Error opening file. %s\n", in_filename);
        exit(0);
        return -1;
    }

    Packet_80211List pList;
    //printf("Read in ..");
    //fflush(stdout);

    //PcapFileHandleToPacket_80211List(PcapHandle, pList);
	//We do this ourselves so we can skip packets that dont have our addres..
	while (GetPcapPacket(PcapHandle, PcapPacket))
	{
		u_int8_t curr_addr[6];

		if (!PcapPacket.is_initialized())
			continue;
		CurrPacket.Init(PcapPacket);
		if (!CurrPacket.is_initialized())
			continue;
		if (CurrPacket.fromDS() && CurrPacket.toDS())
		{
			printf("Skipping WDS frame.\n");
			continue;
		}	
		if (CurrPacket.isControl())	
		{
			pList.push_back(CurrPacket);
		}
		else if (CurrPacket.isMgmt())
		{
			CurrPacket.getSrcAddr(curr_addr);
			if (memcmp(curr_addr, addy, 6) != 0)
				continue;
			//--This is where we do hacky, pre-defcon filtering becauase the database hasnt been populated
			//--with de-auths and disassocaites yet.:
			
	/*		
			if (CurrPacket.subtype() == Disassoc)
				continue;
			if (CurrPacket.subtype() == Deauth)
				continue;
*/			
				pList.push_back(CurrPacket);
	
	}
		else if (CurrPacket.isData())
		{
			CurrPacket.getSrcAddr(curr_addr);
			if (memcmp(curr_addr, addy, 6) == 0)
				pList.push_back(CurrPacket);
		}	

	}
    //printf("%d packets\n", ret);
    P.identifier.id = 0; //unknown.
    ClosePcapFileHandle(PcapHandle);
    int ret = P.UpdateWithPackets(addy, pList);
	 P.Finalize(); //Omni
    pList.clear(); // does this help memory usage at all? i hope not..
    return ret;
}






string type_subtype2string(type_subtype_t t_s)
{
	string ret;
	char tmp[1024];
	if (t_s.type == 0 && t_s.subtype ==4)
		ret = "probe request";
	else if (t_s.type == 0 && t_s.subtype == 0)
		ret = "Assoc Request";
	else if (t_s.type == 0 && t_s.subtype == 1)
		ret = "Assoc Response";
	else if (t_s.type == 0 && t_s.subtype == 2)
		ret = "Reassoc Request";
	else if (t_s.type == 0 && t_s.subtype == 3)
		ret = "Reassoc Response";
	else if (t_s.type == 0 && t_s.subtype == 4)
		ret = "Probe Request";
	else if (t_s.type == 0 && t_s.subtype == 5)
		ret = "Probe response";
	else if (t_s.type == 0 && t_s.subtype == 8)
		ret = "Beacon";
	else if (t_s.type == 0 && t_s.subtype == 9)
		ret = "ATIM";
	else if (t_s.type == 0 && t_s.subtype == 10)
		ret = "Disassociation";
	else if (t_s.type == 0 && t_s.subtype == 11)
		ret = "Authentication";
	else if (t_s.type == 0 && t_s.subtype == 12)
		ret = "De-Auth";
	else if (t_s.type == 1 && t_s.subtype == 10)
		ret = "PS-Poll";
	else if (t_s.type ==1 && t_s.subtype == 11)
		ret = "RTS";
	else if (t_s.type == 1 && t_s.subtype == 12)
		ret = "CTS";
	else if (t_s.type == 1 && t_s.subtype == 13)
		ret = "ACK";
	else if (t_s.type == 1 && t_s.subtype == 14)
		ret = "CF-END";
	else if (t_s.type == 1 && t_s.subtype == 15)
		ret = "CF-END-ACK-POLL";
	else if(t_s.type == 2 && t_s.subtype == 0)
		ret = "Data";
	else if(t_s.type ==2 && t_s.subtype ==1)
		ret = "Data + CF-Ack";
	else if(t_s.type ==2 && t_s.subtype ==2)
		ret = "Data + CF-Poll";
	else if(t_s.type ==2 && t_s.subtype ==3)
		ret = "Data + CF-Ack + CF-Poll";
	else if (t_s.type == 2 && t_s.subtype == 4)
		ret = "Null Function";
	else if (t_s.type == 2 && t_s.subtype ==5)
		ret = "CF-Ack";
	else if (t_s.type ==2 && t_s.subtype == 6)
		ret = "CF-Poll";
	else if (t_s.type ==2 && t_s.subtype == 7)
		ret = "CF-Ack + CF-Poll";
	else
		{
			sprintf(tmp, "<%d,%d>", t_s.type, t_s.subtype);
			ret = tmp;
		}
	return ret;
}
