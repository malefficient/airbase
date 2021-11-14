#include "packet_list.h"

int GetPacketWindow(Packet_80211List &PktList, Packet_80211List::iterator start, Packet_80211List::iterator& stop, int secs, int u_secs, bool PcapTsOk)
{
	timepiece tmp;
	tmp.Time.tv_sec = secs;
	tmp.Time.tv_usec = u_secs;
	return GetPacketWindow(PktList, start, stop, tmp, PcapTsOk);
}


// This function will return the number of packets that were sent After start
// Who's initial transmission time (recorded in the MAC time stamp) falls within the
// window of time start.mac_time + duration. 
// e.g. if you have 
//  [P1    ]  [P2    ] [P3    ]
//  ^start    1s       1s 
//GetPacketWindow(List, P1, stop, 0.5s) will return 0 and stop will point at P2
//IteratePktLisT(List, P1, stop, 1.0s) will return 1 and stop will point at P3
//GetPacketWindow(List, P1, stop, BigTime) will set stop equal to PktList.end() if allpackets fall withing the time range.
//Passing a PacketList of a single packet causes stop PktList.end and returns 0;
//Returns -1 on error. 
int GetPacketWindow(Packet_80211List &PktList, Packet_80211List::iterator start, Packet_80211List::iterator& stop, timepiece duration, bool PcapTsOk)
{
	bool using_mac_ts; //can fall back to pcap if OK.
    Packet_80211List::iterator Walker;
    Walker = start;
	int cnt = 0;

    timepiece start_packet_timepiece;
    Packet_80211 StartPacket;

    StartPacket = *start;

	if (StartPacket.mac_time_available())
		using_mac_ts = true;			
	else
		using_mac_ts = false;
	if (!PcapTsOk && using_mac_ts == false)
	{
			printf("GetPacketWindow::Error, PcapTsOk = False but mac_time not available. Exiting\n");
			exit(0);
	}

	if (using_mac_ts)
		start_packet_timepiece = StartPacket.get_mac_ts();
	else
		start_packet_timepiece = StartPacket.get_pcap_ts();

	Walker++; //Get to next packet 	
	//note how we dont ++cnt here.

	Packet_80211 CurrPacket;
	while (Walker != PktList.end())
	{
		//printf("Walking.\n");
		//CurrPacket.Init(*Walker); //XXX FIXERME! INIT PACKET_80211 FROM PACKET_80211 BORKEN.
		CurrPacket = (*Walker); //XXX FIXERME! INIT PACKET_80211 FROM PACKET_80211 BORKEN.
		timepiece curr_ts;
		if (using_mac_ts)
			curr_ts = CurrPacket.get_mac_ts();
		else
			curr_ts = CurrPacket.get_pcap_ts();

		//if the current packet falls outside the first packet start time + duration
		if (start_packet_timepiece + duration < curr_ts)
			break;

		cnt++;
		Walker++;
	}
	//Either we hit the end of the list or bailed early.
	//If first thing we hit was end of list, cnt = 0;

	stop = Walker; 
	return cnt;
}

int PcapFileToPcapList(char *fname, PcapPacketList &List)
{
	Pcap_Packet CurrPkt;

	while (GetPcapPacket(fname, CurrPkt))
	{
		if (! CurrPkt.is_initialized())
			continue;
		List.push_back(CurrPkt);
	}

	return List.size();
}

int PcapFileHandleToPcapList(PcapFileHandle &file, PcapPacketList &List)
{
	Pcap_Packet CurrPkt;

	while (GetPcapPacket(file, CurrPkt))
	{
		if (! CurrPkt.is_initialized())
			continue;
		List.push_back(CurrPkt);
	}

	return List.size();
}


int PcapFileToPacket_80211List(char *fname, Packet_80211List &List)
{
	Pcap_Packet CurrPkt;
	PcapPacketList PcapList;
	while (GetPcapPacket(fname, CurrPkt))
	{
		if (!CurrPkt.is_initialized())
				continue;
		PcapList.push_back(CurrPkt);
	}
	return PcapPacketListToPacket_80211List(PcapList, List);	
}

int PcapFileHandleToPacket_80211List(PcapFileHandle &file, Packet_80211List &List)
{
	Pcap_Packet CurrPkt;
	PcapPacketList PcapList;
	while (GetPcapPacket(file, CurrPkt))
	{
		if (!CurrPkt.is_initialized())
				continue;
		PcapList.push_back(CurrPkt);
	}
	return PcapPacketListToPacket_80211List(PcapList, List);	
}
	

int PcapPacketListToPacket_80211List(PcapPacketList & PcapList, Packet_80211List &New80211List)
{
	PcapPacketList::iterator my_iter;
	Packet_80211 Tmp;
	for (my_iter= PcapList.begin(); my_iter != PcapList.end(); my_iter++)
    {
        Tmp = * my_iter;
        if (!Tmp.is_initialized())
        {
            printf("Error converting PcapList into Packet_80211List. failed to initialize from pcap packet.\n");
            exit(0);
        }
        New80211List.push_back(Tmp);
    }
	return New80211List.size();
} 
Packet_80211List Fragmentify(Packet_80211 Orig, u_int32_t num_pieces)
{
	Packet_80211List Plist;

	int curr_offset = 0;

	//The code here inentionally bypasses some of the helper functions
	//in libairware. In particular, it could call getPDUData, and getPDUDataLength(),
	//However, since the whole point of this is to be used in a fuzzer, we might
	//want to apply this logic to packets that -aren't- data packets.
	//This makes it a lot more ugly. We have to get int
	int payloadlen = Orig.length - sizeof(struct ieee80211);
 	int fragsize = payloadlen / num_pieces;
	int lastfragsize;
	int wholefrags;

  /* Ensure payload length is greater then the number of fragments */
     if (payloadlen < num_pieces)
	{
		fprintf(stderr, "Fragmentify::Error, payloadlength < num_pieces");
		exit(0);
	}

	if ( (payloadlen % num_pieces) != 0) 
	{
       lastfragsize = (payloadlen - (fragsize * (num_pieces- 1)));
    }
	else 
	{
    	lastfragsize = 0;
    }   
	//Skip over the header initially.
	curr_offset = sizeof(struct ieee80211); //Careful. Pcap_packets data ptr points to the iee80211 header. this is what we offset from.

	 // Number of fragments to send 
    if (lastfragsize > 0) 
	{
    	wholefrags = num_pieces-1;
    }
	else 
	{
    	wholefrags = num_pieces;
    } 		

	for (int i = 0; i < wholefrags; i++)
	{
		Packet_80211 CurrPacket = Orig; // copy the header.
		CurrPacket.length = fragsize + sizeof(struct ieee80211); //perform airware internal structure surgery.
																  //this allocates enough room for a header and the fragmented payload.
		memcpy(CurrPacket.data + sizeof(struct ieee80211), Orig.data + curr_offset, fragsize);
		CurrPacket.set_more_frag();
		CurrPacket.setFragNum(i);
	
		// Test for last fragment and no final partial fragment
       if(i == wholefrags-1 && lastfragsize == 0) 
		{
			CurrPacket.clear_more_frag();
			CurrPacket.setFragNum(i);
        } 	

		Plist.push_back(CurrPacket);
		curr_offset += fragsize;
	}
		
	if (lastfragsize != 0)
	{
		Packet_80211 LastPacket = Orig;
		Orig.length = sizeof(struct ieee80211) + lastfragsize;
		memcpy(LastPacket.data + sizeof(struct ieee80211), Orig.data + curr_offset, lastfragsize);
		LastPacket.clear_more_frag();
		Plist.push_back(LastPacket);
	}
	return Plist;

}

	

