#include "crack.h"


bool operator == (const IV & left, const IV & right)
{
	if (memcmp( &left.IV[0], &right.IV[0], 3) ==0) //match
		return true;
	else
		return false;
}  
bool operator < (const IV & left, const IV & right)
{
	int ret;
	ret = memcmp( &left.IV[0], &right.IV[0], 3);
	
	if (ret < 0)
		return true;
	else
		return false;
}

FILE *OpenReadIvFile(char *filename, char *mode)
{
    FILE *fp;

    fp = fopen(filename, mode);
	if (fp == NULL)
	{
		printf("OpenReadIvFile: Error opening %s with mode %s\n", filename, mode);
		perror("fopen");
		return fp;

	}
	u_int32_t ivmagic;
	if ( fread (&ivmagic , sizeof(ivmagic), 1, fp) != 1)
	{
		printf("OpenReadIvFile: Error reading magic number from file.");
		fclose(fp);
		fp =NULL;
		perror("fread");
		return fp;

	}
	if (ivmagic != IVFILE_MAGIC)
	{
		printf("OpenReadIvFile: Error. Wrong magic number in ivfile. got 0x%x, expected 0x%x\n", ivmagic, IVFILE_MAGIC);
		fclose(fp);
		fp = NULL;
		return fp;
	}

	return fp;

}
FILE * OpenWriteIvFile(char *filename, char *mode)
{
    FILE *fp;

    fp = fopen(filename, mode);
	if (fp == NULL)
	{
		printf("OpenWriteIvFile: Error opening %s with mode %s\n", filename, mode);
		perror("fopen");
		return fp;
	}

	u_int32_t ivmagic = IVFILE_MAGIC;
	//printf("Writing IVmagic %8.8X\n", ivmagic);
	if ( fwrite( &ivmagic , sizeof(ivmagic), 1, fp) != 1)
	{
		printf("OpenWritIvFile: Error opening %s with mode %s\n", filename, mode);
		perror("fwrite");
		exit(0);
	}
	return fp;
}                         

void FreeBssidList(BssidData_List &BssidList)
{
	BssidData_List::iterator bssIter;
	for(bssIter = BssidList.begin(); bssIter != BssidList.end(); bssIter++)
	{
		if ( bssIter->iv_present_map != NULL)
			delete bssIter->iv_present_map;
		bssIter->IvList.clear();
	}
	BssidList.clear();
}

bool AddBssidDataToList(BssidData_List &BssidList, bssid_data &new_data, bool iv_filtering)
{
	BssidData_List::iterator bssIter;
	u_int32_t offset;

	
	for(bssIter = BssidList.begin(); bssIter != BssidList.end(); bssIter++)
	{
		//If we alardy have an entry for this bssid.
		if ( memcmp(new_data.bssid, bssIter->bssid, 6) == 0) //match
		{
			AddIvsToBssidData(*bssIter, new_data, iv_filtering);
			return true;
		}	

	}
	printf("AddBssidDataToList::New bssid passed in:\n");
	print_hex(new_data.bssid, 6);
	printf("\n");
	sleep(1);

	//didnt find new_data's bssid in List. Add it as a new entry.
	//printf("didnt find bssid in list, adding as new\n");
	
	//We didnt know about this BSSID before. Add it to the list
	//of All BSSID's we've seen. We also take this opportunity

	if (iv_filtering)
	{
			
		if (MAX_BSSIDS_WITH_IVMAPS_PER_LIST !=  0 && BssidList.size() + 1 > MAX_BSSIDS_WITH_IVMAPS_PER_LIST)
		{
			printf("Error. To many bssids in list!. Exiting before running out of memory.\n");
			return false;
			//exit(0);
		}

		new_data.iv_present_map = new u_int8_t[0x00FFFFFF];
		if ( !new_data.iv_present_map)
		{
			printf("Couldnt allocate mem for iv_map.\n");
			exit(0);
		}
		memset(new_data.iv_present_map, 0, 0x00FFFFFF);
	}
	else
		new_data.iv_present_map = NULL;

	IV_List::iterator tmp_iter;
	
	//If the new data -already- has dupes in it among itself, what do we do?
	if (iv_filtering)
	{
		for (tmp_iter = new_data.IvList.begin(); tmp_iter != new_data.IvList.end(); tmp_iter++)
		{
			Iv2Offset( &tmp_iter->IV[0], offset);
			//printf("offset = %8.8X\n", offset);
			if ( new_data.iv_present_map[offset]) //already have this iv in the list.
			{
				//printf("DUPE in new_data %8.8X\n", offset);
				new_data.num_ivs--;
				new_data.IvList.erase( tmp_iter );
				tmp_iter--; // if we dont do this we skip things..
			}
			else
				new_data.iv_present_map[offset] = 1;
		}
	}
	
	BssidList.push_back(new_data); 

	return true;
}
bool OldAddBssidDataToList(BssidData_List &BssidList, bssid_data &new_data, bool iv_filtering)
{
	BssidData_List::iterator bssIter;
	u_int32_t offset;

	
	for(bssIter = BssidList.begin(); bssIter != BssidList.end(); bssIter++)
	{
		//If we alardy have an entry for this bssid.
		if ( memcmp(new_data.bssid, bssIter->bssid, 6) == 0) //match
		{
		//	printf("AddBssidDataToBssidList::Adding new_data to existing bssid.\n");
			bool found_iv;
			IV_List::iterator ivIter;
			//for every iv in the new list (for a bssid we've already seen)
			//check to see if we have the IV already. if so, drop it. if not. addit.
			for( ivIter = new_data.IvList.begin(); ivIter != new_data.IvList.end(); ivIter++)
			{
				Iv2Offset( & (ivIter->IV[0]), offset);
				if( iv_filtering && bssIter->iv_present_map[offset]) 
				{
				//	printf("Skiping %8.8X\n", offset);
					continue;
				}
				else
				{
					bssIter->IvList.push_back( *ivIter);
					bssIter->num_ivs++;
					if (iv_filtering)
						bssIter->iv_present_map[offset] = 1;
				}
			}
			return true;
		}	

	}

	//didnt find new_data's bssid in List. Add it as a new entry.
	//printf("didnt find bssid in list, adding as new\n");
	
	//We didnt know about this BSSID before. Add it to the list
	//of All BSSID's we've seen. We also take this opportunity

	if (iv_filtering)
	{
			
		if (MAX_BSSIDS_WITH_IVMAPS_PER_LIST !=  0 && BssidList.size() + 1 > MAX_BSSIDS_WITH_IVMAPS_PER_LIST)
		{
			printf("Error. To many bssids in list!. Exiting before running out of memory.\n");
			return false;
			//exit(0);
		}

		new_data.iv_present_map = new u_int8_t[0x00FFFFFF];
		if ( !new_data.iv_present_map)
		{
			printf("Couldnt allocate mem for iv_map.\n");
			exit(0);
		}
		memset(new_data.iv_present_map, 0, 0x00FFFFFF);
	}
	else
		new_data.iv_present_map = NULL;

	IV_List::iterator tmp_iter;
	
	//If the new data -already- has dupes in it among itself, what do we do?
	if (iv_filtering)
	{
		for (tmp_iter = new_data.IvList.begin(); tmp_iter != new_data.IvList.end(); tmp_iter++)
		{
			Iv2Offset( &tmp_iter->IV[0], offset);
			//printf("offset = %8.8X\n", offset);
			if ( new_data.iv_present_map[offset]) //already have this iv in the list.
			{
				//printf("DUPE in new_data %8.8X\n", offset);
				new_data.num_ivs--;
				new_data.IvList.erase( tmp_iter );
				tmp_iter--; // if we dont do this we skip things..
			}
			else
				new_data.iv_present_map[offset] = 1;
		}
	}
	
	BssidList.push_back(new_data); 

	return true;
}



//Only for internal use. 
void Iv2Offset(u_int8_t *IV, u_int32_t &offset)
{
	offset = 0;
	memcpy( ((char *)&offset + 1), IV, 3);
	offset = htonl(offset);
}  



int AddIvsToBssidData(bssid_data & dest, bssid_data & source, bool iv_filtering)
{   
    int num_ret = 0;
    u_int32_t offset;
    IV_List::iterator ivIter;
    if (memcmp(dest.bssid, source.bssid, 6) != 0)
    {
        //fprintf(stderr, "Dropping whole list. Invalid BSSID.\n");
        //sleep(1);
        return 0;
    }

  for( ivIter = source.IvList.begin(); ivIter != source.IvList.end(); ivIter++)
  {
     Iv2Offset( & (ivIter->IV[0]), offset);
     if( iv_filtering && dest.iv_present_map[offset])
     {
     	//  printf("Skiping %8.8X\n", offset);
        continue;
      }
      else
      {
      	num_ret++;
       dest.IvList.push_back( *ivIter);
       dest.num_ivs++;
       if (iv_filtering)
       	dest.iv_present_map[offset] = 1;
        }
   }
    return num_ret;
}


//XXX: DO NOT CALL THIS FUNCTION REPEATEDLY ON A SINGLE FILE
//		If the iv file does not end on a strut iv boundary on the
//		first call it will not know and return garbage!
//		see the nasty code in jc-aircrack for 'graceful' recovery from that.
int ReadIvData(FILE *f_in,  BssidData_List &MyBssidList, bool iv_filtering)
{

    static unsigned char curr_bssid[6] = {0, 0, 0, 0, 0, 0};
    static unsigned char prev_bssid[6] = {0, 0, 0, 0, 0, 0};

    IV currIv;
    int num_read = 0;
    int ret;

    //tmpbssid is a list of all the concurrent ivs we've just read
    //in the file. it will then be added to MyBssidList under the correct
    //entry. 
    bssid_data tmpbssid;
	//which f_read failed?
    while( 1 )
    {

		
        if (  (fread(curr_bssid, 1, 1, f_in) != 1) )
        {
			//Error re
            //If the file is well formed we should get a 
            //read error here when we run outta data.
            AddBssidDataToList(MyBssidList, tmpbssid, iv_filtering);//true enables iv filtering.
            tmpbssid.IvList.clear();
            //MyBssidList.push_back(tmpbssid);  
            printf( "Read %d ivs.\n", num_read );
            printf("From %d Bssids:\n", MyBssidList.size());

           	break; 
            //return(MyBssidList);
        }                                                                     

        if (curr_bssid[0] != 0xff) // new bssid 
        {

            int ret;
            //next 5 bytes of  bssid
            ret = fread(&curr_bssid[1], 1, 5, f_in);
            //now we have all 6 bytes of the new bssid in curr_bssid
            if (ret != 5)
            {
                printf("Error reading new BSSID. File corrupted! read %d byts\n", ret);
				break;
            }
            //printf("%d New bssid:", num_read);
            //print_hex(curr_bssid, 6);

            if (num_read> 0) //dont push our initial empty tmpbssid. later check if this is  already in list
                AddBssidDataToList(MyBssidList, tmpbssid, iv_filtering); //true enables iv filtering.
                //MyBssidList.push_back(tmpbssid);  

            memcpy(&tmpbssid.bssid, curr_bssid, 6);
            tmpbssid.num_ivs = 0;
            tmpbssid.IvList.clear();

        } //if (new bssid read)



        char buff[5];
        ret = fread((void *) buff, 1, 5, f_in);
        if (ret != 5)
        {

            printf("Error reading middle of IV. adding what data we can.V\n");
            if (num_read> 0) //dont push our initial empty tmpbssid. later check if this is  already in list
                AddBssidDataToList(MyBssidList, tmpbssid, iv_filtering); //true enables iv filtering.
			break;
            
        }                                                  
     	memcpy(currIv.IV, buff, 3);
        currIv.b1 = buff[3];
        currIv.b2 = buff[4];

        num_read++;
        tmpbssid.IvList.push_back(currIv);
        tmpbssid.num_ivs++;

    }
            //exit(0);
	return num_read;

}


int ReadPcapData(char *filename, BssidData_List &MyBssidList, bool iv_filtering,  bool verbose)
{

    FILE *f_in;
    unsigned char curr_bssid[6];
    unsigned char prev_bssid[6];

    memset( curr_bssid, 0, 6 );
    memset( prev_bssid, 0, 6 );


    IV currIv;

    printf( "reading %s\n", filename );

    int num_read = 0;
    int ret;

    //tmpbssid is a list of all the concurrent ivs we've just read
    //in the file. it will then be added to MyBssidList under the correct
    //entry. 
    bssid_data tmpbssid;

    Pcap_Packet PcapPacket;
    Packet_80211 currPacket;
    Packet_80211_data DataPacket;
    PcapFileHandle PcapHandle;

    IV tmpIv;

    if ( !OpenReadPcapFile(filename, PcapHandle) )
        return -1;
    printf("Handle is on type %s\n", PcapHandle.dlt_name);

	if (!airware_supported_dlt(PcapHandle.dlt))
    {
        printf("Error. Pcap link type  not supported (0x%02x), %s\n", PcapHandle.dlt, PcapHandle.dlt_name);
        exit(0);
    }

    //In the IV file we we're a little more efficient about this. Here we just add a 
    //single bssid_data at a time to the list. 
    while (GetPcapPacket(PcapHandle, PcapPacket))
    {
        num_read++;                                                                                                     
        tmpbssid.IvList.clear();
        tmpbssid.num_ivs = 0;

        if (!PcapPacket.is_initialized()) // Should i make GetPcapPacket check this..maybe. adds 1 memcpy overhead.
        {
            if (verbose)
                printf("Skipping malfored Pcap Packet # %d\n", num_read);
            continue;
        }

        currPacket.Init(PcapPacket);
        if (!currPacket.is_initialized()) // Should i make GetPcapPacket check this..maybe. adds 1 memcpy overhead.
        {
            if (verbose)
                printf("Skipping 80211  Packet # %d\n", num_read);
            continue;
        }

        if (! currPacket.isData())
        {
            if (verbose)
                printf("Skipping non data 80211  Packet # %d\n", num_read);
            //exit(0);
            continue;
        }

        DataPacket.Init(currPacket);
        if (!DataPacket.is_initialized()) // Should i make GetPcapPacket check this..maybe. adds 1 memcpy overhead.
        {
            if (verbose)
                printf("Skipping not initialized Data 80211  Packet # %d\n", num_read);
            continue;
        }

        if (!DataPacket.wep())
        {
            if (verbose)
                printf("Skipping non wep Data Packet # %d\n", num_read);
            continue;
        }

        //printf ("\n");                                                                                                
        DataPacket.getBssId(tmpbssid.bssid);
        DataPacket.getWepIV(tmpIv);
        tmpbssid.num_ivs = 1;
        tmpbssid.IvList.push_back(tmpIv);
        AddBssidDataToList(MyBssidList, tmpbssid, iv_filtering);

    }

    return num_read;
}

int OldWriteBssidListToFile(char *iv_filename, BssidData_List & BssidList)
{
    BssidData_List::iterator bssIter;
    IV_List::iterator ivIter;
   	FILE *iv_file;
	iv_file = OpenWriteIvFile(iv_filename);
  	u_int8_t same_bssid_flag = 0xff; 
	int num_ivs_written =0;
	if (!iv_file)
		return -1; 

	bool skip_same_bssidflag;

    for(bssIter = BssidList.begin(); bssIter != BssidList.end(); bssIter++)
	{
		if (fwrite(&bssIter->bssid, sizeof(bssIter->bssid), 1, iv_file) != 1)
        {
        	perror("fwrite");
           	exit(0);
         }        

		skip_same_bssidflag = true;
        for( ivIter = bssIter->IvList.begin(); ivIter != bssIter->IvList.end(); ivIter++)
		{
			if (skip_same_bssidflag == true)
				skip_same_bssidflag = false;

            else if (fwrite(&same_bssid_flag, sizeof(same_bssid_flag), 1, iv_file) != 1)
            {
            	perror("WriteBssidList::fwrite");
				return -1;
             }         

			
             if (fwrite(&ivIter->IV[0], 3, 1, iv_file) != 1)
             {
            	perror("WriteBssidList::fwrite");
				return -1;
             }
             if (fwrite(&ivIter->b1, 1, 1, iv_file) != 1)
             {
            	perror("WriteBssidList::fwrite");
				return -1;
             }
             if (fwrite(&ivIter->b2, 1, 1, iv_file) != 1)
             {
            	perror("WriteBssidList::fwrite");
				return -1;
             }                                   
			num_ivs_written++;
		}
	}	

	return num_ivs_written;
}

//return -1 on error
int WriteBssidListToFile(char *iv_filename, BssidData_List & BssidList)
{
    BssidData_List::iterator bssIter;
    IV_List::iterator ivIter;
   	FILE *iv_file;
	iv_file = OpenWriteIvFile(iv_filename);
	if (!iv_file)
		return -1; 

	return AppendBssidDataToFile(iv_file, BssidList);
	fclose(iv_file);
}

/* We assume in this taht FILE iv_file was left off at the end of a record cleanly. 
 * If not, data corruption will follow.
 */
int AppendBssidDataToFile(FILE *iv_file, BssidData_List & BssidList)
{
    BssidData_List::iterator bssIter;
    IV_List::iterator ivIter;
  	u_int8_t same_bssid_flag = 0xff; 
	int num_ivs_written =0;
	if (!iv_file)
		return -1; 

	bool skip_same_bssidflag;

    for(bssIter = BssidList.begin(); bssIter != BssidList.end(); bssIter++)
	{
		if (fwrite(&bssIter->bssid, sizeof(bssIter->bssid), 1, iv_file) != 1)
        {
        	perror("fwrite");
           	exit(0);
         }        

		skip_same_bssidflag = true;
        for( ivIter = bssIter->IvList.begin(); ivIter != bssIter->IvList.end(); ivIter++)
		{
			if (skip_same_bssidflag == true) //this is true if we layed down the 6 byte bssid immediately before.
				skip_same_bssidflag = false; //so we dont want to write an extra 0xff

            else if (fwrite(&same_bssid_flag, sizeof(same_bssid_flag), 1, iv_file) != 1)
            {
            	perror("WriteBssidList::fwrite");
				return -1;
             }         

			
             if (fwrite(&ivIter->IV[0], 3, 1, iv_file) != 1)
             {
            	perror("WriteBssidList::fwrite");
				return -1;
             }
             if (fwrite(&ivIter->b1, 1, 1, iv_file) != 1)
             {
            	perror("WriteBssidList::fwrite");
				return -1;
             }
             if (fwrite(&ivIter->b2, 1, 1, iv_file) != 1)
             {
            	perror("WriteBssidList::fwrite");
				return -1;
             }                                   
			num_ivs_written++;
		}
	}	

	return num_ivs_written;
}


bool  get_small_wep_pkts(char *filename, u_int8_t * match_bssid, Packet_80211_data &SmallestPacket, 
						Packet_80211_data &SecondSmallestPacket)
{
 	Pcap_Packet PcapPacket;
    Packet_80211 currPacket;
    Packet_80211_data DataPacket;
	u_int8_t tmp_bssid[6];
	Packet_80211_data Small1DataPacket; //smallest 
	Packet_80211_data Small2DataPacket; // Second Smallest
	//Small1DataPacket.length = 0xffffffff;
	//Small2DataPacket.length = 0xffffffff;
	
	 int packets_to_prime = 2;
	 PcapFileHandle PcapHandle;
	 if (! OpenReadPcapFile(filename, PcapHandle))
		return -1;

	//Read in two initial packets. if we cant find any return false.
	int cnt = 2;
	 while (GetPcapPacket(PcapHandle, PcapPacket))
    {
		if (cnt == 0)
			break;

        if (! PcapPacket.is_initialized())
            continue;
        currPacket.Init(PcapPacket);
        if (! currPacket.is_initialized())
            continue;
		if (! currPacket.isData())
			continue;
		if (!currPacket.wep())
			continue;
		currPacket.getBssId(tmp_bssid);
		if (memcmp(match_bssid, tmp_bssid, 6) != 0)
			continue;
		DataPacket.Init(currPacket);	
		if (!DataPacket.is_initialized())
			continue;

		if (cnt == 2)
		{
			Small2DataPacket.Init(DataPacket);
			cnt--;	
		}
		else if (cnt == 1)
		{
			Small1DataPacket.Init(DataPacket);
			cnt--;
		}
	}	
	pcap_close(PcapHandle.p);


	if (Small1DataPacket.is_initialized() && Small2DataPacket.is_initialized())
	{
		//talk about a corner case..
		if (Small2DataPacket.length < DataPacket.length) //if first packet we read was smaller than 2nd
		{
			Small1DataPacket.Init(Small2DataPacket);
			Small2DataPacket.Init(DataPacket);
		}

		printf("Primed with two packets:\n");
		Small1DataPacket.print();
		printf("----\n");
		Small2DataPacket.print();
		sleep(2);
	}
	else
		return false;

	 while (GetPcapPacket(filename, PcapPacket))
    {
        if (! PcapPacket.is_initialized())
            continue;
        currPacket.Init(PcapPacket);
        if (! currPacket.is_initialized())
            continue;
		if (! currPacket.isData())
			continue;
		if (!currPacket.wep())
			continue;
		currPacket.getBssId(tmp_bssid);
		if (memcmp(match_bssid, tmp_bssid, 6) != 0)
			continue;
		DataPacket.Init(currPacket);	
		if (!DataPacket.is_initialized())
			continue;

		
		if (DataPacket.length < Small1DataPacket.length)
        {
			Small2DataPacket.Init(Small1DataPacket);
			Small1DataPacket.Init(DataPacket);
		}
		
	}   
	SmallestPacket.Init(Small2DataPacket);
	SecondSmallestPacket.Init(Small2DataPacket);
	return true;
}

bssid_data ChooseBssidFromList(BssidData_List &List)
{

    BssidData_List::iterator myIter;

    printf("  #\tBSSID:\t\t\t num iv's.\n");
    int user_input;

    int cnt = 1;
    if (List.size() == 0)
    {
        printf("Error. No Bssid's found. Exiting.\n");
        exit(0);
    }
    for (myIter = List.begin(); myIter != List.end(); myIter++)
    {
            printf("%3d\t", cnt);
            print_hex(myIter->bssid, 6);
            printf("\t");
            printf("%u\n", myIter->num_ivs);
            cnt++;
    }

    if (List.size() == 1)
    {
        printf("Choosing #1\n");
        return List.front(); //dont prompt user with one choice.
    }

    sleep(1);
    do
    {
        printf("Select network Number ");
        fscanf(stdin, "%d", &user_input);
    }
    while  (user_input <  1 || user_input >= cnt); //get valid input

    user_input--; // start at 0..
    myIter = List.begin();
    while (user_input > 0) 
 	{
        myIter++;
        user_input--;
    }

    return *myIter;

} 
