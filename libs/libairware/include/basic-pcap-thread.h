#ifndef _BASIC_PCAP_THREAD_H
#define _BASIC_PCAP_THREAD_H
#include <pcap.h>
#include <pthread.h>
#include <airware.h>

//Private, internal state
struct  thread_options
{
	bool currently_running; /* read only in sniff thread*/
    pcap_t *pcap_handle;
	int dlt;
    char my_err_buff[PCAP_ERRBUF_SIZE];
	int num_pkts_to_sniff;
    int num_captured; 		/*only writable member in sniff_thread */
	PcapPacketList *packet_list_ptr;  /* thread will store all initializable pcap packets here */

};

class BasicPcapThread
{
    private:
	pthread_t SnifferThread;
	struct thread_options ThreadOptions;
    public:
	//--stuff that tweaks run-time behavior
	//modifiable before calling start.
	
    BasicPcapThread(char *dev, int snaplen, int promisc, int to_ms, PcapPacketList *packet_list);
    BasicPcapThread(pcap_t *_pcap_handle, PcapPacketList *packet_list);
    int start_capture(int num_pkts = -1);
    int stop_capture(); //returns num captured.

    private:
    int get_num_captured() { return ThreadOptions.num_captured;} //static cause they are callbacks for C code..
};
    void *  thread_func(void *arg);

#endif
