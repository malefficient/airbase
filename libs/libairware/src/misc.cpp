#include "misc.h"

bool airware_supported_dlt(int dlt)
{
 if (dlt != DLT_IEEE802_11 && dlt != DLT_IEEE802_11_RADIO  && dlt != DLT_PRISM_HEADER && dlt != DLT_IEEE802_11_RADIO_AVS)
	return false;
 else
	return true;
}


bool GetPcapPacket( char *filename, Pcap_Packet& Passed_Packet)
{

        static bool first_call = 1;
		static PcapFileHandle PcapFile;
        u_char_t *pkt;
        struct pcap_pkthdr h;

        if (first_call)
		{
			if ( !OpenReadPcapFile(filename, PcapFile))
				return false;
            first_call = 0;
        }

        if ((pkt = (u_char *) pcap_next(PcapFile.p, &h)) == NULL)
        {
			pcap_close(PcapFile.p);
            // no more packets.
            return false;
        }

    Passed_Packet.Init(h, pkt, PcapFile.dlt);
    return true;
}

bool GetPcapPacket(PcapFileHandle &PcapFile, Pcap_Packet &Passed_Packet)
{
        u_char_t *pkt;
        struct pcap_pkthdr h;

        if ((pkt = (u_char *) pcap_next(PcapFile.p, &h)) == NULL)
            return false;

    Passed_Packet.Init(h, pkt, PcapFile.dlt);
	return true;	
}


bool OpenLivePcapDevice(char *device, PcapFileHandle &PcapHandle, bool promisc, int snaplen, int to_ms)
{

	if ((PcapHandle.p = pcap_open_live(device, snaplen, (int) promisc, to_ms,  PcapHandle.errbuf)) == NULL)
    {
        fprintf(stderr, "error: unable to open devices : %s\n", PcapHandle.errbuf);
        return false;
    }
    PcapHandle.dlt = pcap_datalink(PcapHandle.p);
	strncpy(PcapHandle.dlt_name, pcap_datalink_val_to_name(PcapHandle.dlt), sizeof(PcapHandle.dlt_name));

	if (!airware_supported_dlt(PcapHandle.dlt))
	{
		printf("Error. Dlt not supported (0x%02x), %s\n", PcapHandle.dlt, PcapHandle.dlt_name);
		exit(0);
	}
	/*
    //These are all the dlt's we can parse :)
    //if (PcapHandle.dlt != DLT_IEEE802_11 &&  PcapHandle.dlt != DLT_IEEE802_11_RADIO  && PcapHandle.dlt != DLT_PRISM_HEADER && PcapHandle.dlt != DLT_IEEE802_11_RADIO_AVS)
    {
       printf("AirWare warning: invalid dlt (%d)  on device %s\n. Dont as me to parse it later.\n", PcapHandle.dlt, device);
       //exit(0);
       //return false;
    }
	*/
    return true;                
	
}
bool OpenReadPcapFile(char *filename, PcapFileHandle &PcapFile)
{

	if ((PcapFile.p = pcap_open_offline(filename, PcapFile.errbuf)) == NULL)
	{
		fprintf(stderr, "error: unable to open pcap input file: %s\n", PcapFile.errbuf);
		return false;
    }
    PcapFile.dlt = pcap_datalink(PcapFile.p);
	strncpy(PcapFile.dlt_name, pcap_datalink_val_to_name(PcapFile.dlt), sizeof(PcapFile.dlt_name));
    // If we were so inclined we could also handle the prism datalink type. It is just
    // this with debuginfo prepended.
   
    //These are all the dlt's we can parse :)
	if (!airware_supported_dlt(PcapFile.dlt))
	{
		printf("Error. Dlt not supported (0x%02x), %s\n", PcapFile.dlt, PcapFile.dlt_name);
		exit(0);
	}
	/*
    if (PcapFile.dlt != DLT_IEEE802_11 && PcapFile.dlt != DLT_PRISM_HEADER && PcapFile.dlt != DLT_IEEE802_11_RADIO_AVS && PcapFile.dlt 
!= DLT_IEEE802_11_RADIO)
    {
       printf("AirWare warning: invalid dlt on file %s\n. Dont as me to parse it later.\n", filename);
       //exit(0);
       //return false;
    }
	*/

	return true;
}

bool ClosePcapFileHandle(PcapFileHandle &P)
{
	pcap_close(P.p);	
	return true;
}

FILE * OpenWritePcapFile(char *filename, u_int32_t dlt, char *mode)
{
    FILE *fp;
    struct pcap_file_header file_header;
	file_header.magic = 0xa1b2c3d4;
    file_header.version_major = 2; //pulled of a mailing list....
    file_header.version_minor = 4;
    file_header.thiszone = 0; //not used
	file_header.sigfigs = 0; //unused?
	file_header.snaplen = 0xffffffff; 
	file_header.linktype = dlt;

    fp = fopen(filename, mode);
	if (fp == NULL)
			return fp;


	if ( fwrite( &file_header, sizeof(file_header), 1, fp) != 1)
	{
		printf("OpenWritePcapFile: Error opening %s with mode %s\n", filename, mode);
		perror("fwrite");
		exit(0);
	}
	return fp;
}                         

/*
void AddBssidDataToList(BssidData_List &List, bssid_data &data)
{
	BssidData_List::iterator bssIter;

	for(bssIter = List.begin(); bssIter != List.end(); bssIter++)
	{
		if ( memcmp(data.bssid, bssIter->bssid, 6) == 0) //match
		{
			bssIter->num_ivs += data.num_ivs;
			//printf("AddBssidDataToList::Adding data to existing bssid.\n");
			IV_List::iterator ivIter;
			for( ivIter = data.IvList.begin(); ivIter != data.IvList.end(); ivIter++)
					bssIter->IvList.push_back( *ivIter);
				
			return;
		}	

	}
	//didnt find data's bssid in List. Add it as a new entry.
//	printf("didnt find bssid in list, adding as new\n");
	List.push_back(data);
}
*/

int hexstring2value(char *string, u_int8_t *buff, int buffsize)
{
	char *c;
	c = string;
	long int temp;
	int i;
	//printf("hexstring2value passed: %s\n", string);
	int num_bytes_in_string = (strlen(string) + 1) / 3;

	if ( num_bytes_in_string < buffsize)
			buffsize = num_bytes_in_string;
	//printf("num_bytes_in_string = %d\n", num_bytes_in_string);
	//printf("buffsize = %d\n");
	for (i = 0; i < buffsize; i++)
	{

		//only way to do strto* error checking. doesnt seem very robust though.
		errno = 0;
		temp = strtoul(c, NULL, 16);	
		if (errno)
			return i;	

		c+= 3;
		buff[i] = (u_int8_t) temp;
	}
	//printf("Bottom return\n");
	return i;
}


bool BufferHasFCSAlready(u_int8_t *data, int length)
{   
    u_int8_t *icv_ptr;
    u_int32_t icv;
    u_int32_t computed_crc;
	
	if (length < 8) //Any format packet needs atleast this many bytes..	
		return false;


	//printf("Looking for spurious checksums..\n");
	//Read the last 4 bytes out of packet, treas as possible checksum
	icv_ptr = data + length - 4;
	memcpy(&icv, icv_ptr, 4); //note we are carfeul to keep these 4 bytes in network order.
	computed_crc = (bufcrc(data, length - 4)); 
/*
	printf("icv_ptr: ");
	print_hex( icv_ptr, 4);
	printf("\n");
	printf("last 4 bytes: ");
	print_hex( (u_int8_t *) &icv, 4);
	printf("\n");
	printf("computed_crc: ");	
	print_hex( (u_int8_t *) &computed_crc, 4);
	printf("\n");
*/	
    if ( memcmp(&computed_crc,  &icv, 4) == 0)
	{
		//printf("Packet has FCS already.\n");
        return true;
	}
    else
        return  false;
} 




