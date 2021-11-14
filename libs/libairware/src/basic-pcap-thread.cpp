#include "basic-pcap-thread.h"


BasicPcapThread::BasicPcapThread(char *dev, int snaplen, int promisc, int to_ms, PcapPacketList *packet_list_ptr)
{
	ThreadOptions.pcap_handle = pcap_open_live(dev, snaplen, promisc, to_ms, ThreadOptions.my_err_buff);
	ThreadOptions.packet_list_ptr = packet_list_ptr;
}

BasicPcapThread::BasicPcapThread(pcap_t * _pcap_handle, PcapPacketList *packet_list_ptr)
{
	ThreadOptions.pcap_handle = _pcap_handle;
	ThreadOptions.packet_list_ptr = packet_list_ptr;
}


int BasicPcapThread::start_capture(int num_pkts)
{
	int ret;
	ThreadOptions.num_pkts_to_sniff = num_pkts;
	ThreadOptions.num_captured = 0;
	if (ThreadOptions.pcap_handle == NULL)
	{
		printf("Error getting pcap_handle. %s\n", ThreadOptions.my_err_buff);
		exit(0);
		return false;
	}
	ThreadOptions.dlt = pcap_datalink(ThreadOptions.pcap_handle);

	//Add pthread foo here
	ret = pthread_create(&SnifferThread, NULL, thread_func, (void *) &ThreadOptions);
	printf("pthread_create returned: %d\n", ret);
	ThreadOptions.currently_running = true;
	return ret;
}

int BasicPcapThread::stop_capture()
{
	//stop pthread
	//
	ThreadOptions.currently_running = false;
	pthread_join(SnifferThread, NULL);
	pthread_detach(SnifferThread);
	return ThreadOptions.num_captured;
}
		

//BasicPcapThread gets a struct thread_options ptr passed to it. 
//This will then get passed to the pcap callback
//When we set a flag in the options for the callback to stop. it will
//exit on the next received packet.
void * thread_func(void *arg)
{
	struct thread_options * options = (struct thread_options *) arg;
	struct pcap_pkthdr * header_ptr;
	
	u_char *packet; //will be ptr to data returned by pcap_enxt_ex
	const u_char *asdf_packet;
	const u_char ** asdf = &asdf_packet;

	Pcap_Packet PcapPacket; 
	Packet_80211 CurrPkt;  

	int ret;
	//const u_char **packet_ptr = &packet;
top_thread_check:
	while (options->currently_running && 
		  (options->num_pkts_to_sniff == -1 || (options->num_pkts_to_sniff - options->num_captured) > 0))
	{
		//printf("waitingon pcap-next::\n");
		//packet = (u_char *) pcap_next(options->pcap_handle, &header);
		
		int ret =  pcap_next_ex(options->pcap_handle, &header_ptr, asdf);
		//printf("pcap_next_ex ret = %d\n", ret);
		if (ret == 0)
		{
		//	printf("time out expired.\n");
			goto top_thread_check;
		}
		
		packet = (unsigned char *) *asdf; //cast because of airawre prototype
		//printf("captured pkt caplen = %d\n", header_ptr->caplen);
		if (packet == NULL)
		{
			printf("pcap_next returned null..\n");
			continue;
		}
		PcapPacket.Init(*header_ptr, packet, options->dlt);
		if (!PcapPacket.is_initialized())
		{
			printf("Bad pcap packet.\n");
			continue;
		}
		/*
		CurrPkt.Init(PcapPacket);
		if (!CurrPkt.is_initialized())
		{
			printf("Bad 80211 packet.\n");
			continue;
		}
		*/
		options->num_captured++;
		//printf("pcap-thread captured a pkt.\n");
		options->packet_list_ptr->push_back(PcapPacket);
		//printf("*");
		//Add to list!

	}
	pthread_exit(NULL);
}

