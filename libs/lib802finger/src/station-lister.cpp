#include "802fingerlib.h"
#include "802fingertest.h"
#include "airware.h"
#include <signal.h>
//#include <net/if.h> //IFNAMSIZ
#include <linux/wireless.h> //IW_MODE_MONITOR.
extern "C"
{
#include <tx80211.h>
#include <tx80211_packet.h>
}


void parse_args(int argc, char **argv, struct global_cfg_type &cfg);
bool stationlist_Tester(char *fname);
void PrintStationSet(char *fname);
void GenerateStationSetFromPcapFile(PcapFileHandle &handle, StationSetType &s);
void GenerateStationSetFromInterface(PcapFileHandle &handle, StationSetType &S, int secs);
void GenerateStationSet();

bool still_running = true;
void sig_handler(int sig)
{
	still_running= false;
}

struct global_cfg_type
{
	bool using_pcap_file;
	PcapFileHandle pcap_handle;
	char device[IFNAMSIZ+1];
	char output_filename[1024]; 
	char print_clientlist_fname[1024]; //input filename for printing.
	int channel;
	int time;
	int drivertype;
	int verbose;
	int cnt;
	bool just_printing;
	struct tx80211 in_tx;
} global_cfg;

void usage()
{
	printf("station-lister:\t");
	printf("-i (iface | pcapfile)  -o outfile [-c chan -r driver -T secs -v verbose] \n");
	printf("station-lister:\t-P client-listf-file\n");

	printf("  -i  (input)\t\t\tInput (pcap file or interface)\n");
	printf("  -o  (output)\t\t\tOutput client list to file (probably for other utlities to use)\n");
	printf("  -c  (channel)\t\t\tSet channel\n");
	printf("  -r  (driver)\t\t\tSet lorcon driver (for channel changing)\n");
	printf("  -T  (Time)\t\t\tHow many sec's to capture packets for\n");
	printf("  -v  (verbose)\t\t\tPrint client info to screen \n");
	printf("  -P  (Print)\t\t\tPrint a previously saved client list\n");
	
}

void parse_args(int argc, char **argv, struct global_cfg_type &cfg)
{
	char option;
	bool i_flag, c_flag, r_flag, o_flag, v_flag, P_flag;
	i_flag = c_flag = r_flag = o_flag = v_flag = P_flag = false;
	

	while(1)
	{
		

		option = getopt(argc, argv, "i:o:c:r:T:P:v");
		if (option == -1)
			break;

		switch(option)
		{
			case 'i':
			i_flag = true;
			snprintf(cfg.device, sizeof(cfg.device), "%s", optarg);

			if ( OpenLivePcapDevice(optarg, cfg.pcap_handle) ==false ) 
				fprintf(stderr, "Couldnt open %s as device, trying file\n", optarg);
			else
			{
				snprintf(cfg.device, sizeof(cfg.device), "%s", optarg);
				cfg.using_pcap_file = false;
				break;
			}

			if ( OpenReadPcapFile(optarg, cfg.pcap_handle) == false)
				fprintf(stderr, "Couldnt open %s as pcap file.\n", optarg);
			else
			{
				snprintf(cfg.device, sizeof(cfg.device), "%s", optarg);
				cfg.using_pcap_file = true;
				break;
			}
			printf("Error. Couldnt open %s as a file or as a device.\n", optarg);
			exit(0);
			break;	
					
			case 'o':
			o_flag = true;
			snprintf(cfg.output_filename, sizeof(cfg.output_filename), "%s", optarg);
			break;

		case 'c':
			c_flag = true; 
			cfg.channel = atoi(optarg);
			break;

		case 'r':
			r_flag = true;
			cfg.drivertype = tx80211_resolvecard(optarg);
			break;
		case 'T':
			cfg.time = atoi(optarg);
			break;

		case 'v':
			v_flag = true;
			cfg.verbose++;
			break;
		case 'P':
			P_flag = v_flag = true; //enable vebose, else no display.
			cfg.just_printing = true;
			snprintf(cfg.print_clientlist_fname, sizeof(cfg.print_clientlist_fname), "%s", optarg);
			break;

		case '?':
		default:
				usage(); exit(0);
		}

	}

	if (!v_flag && !o_flag)
	{
		printf("Error. Must pass -o or -v to get any useful output.\n");
		exit(0);
	}
	if (c_flag && !r_flag)
	{
		printf("Error. driver must be passed to set channel.\n");
		usage();
		exit(0);
	}
	if (!i_flag && ! (cfg.just_printing || cfg.using_pcap_file))
	{
		printf("Error. Must pass input with -i\n");
		usage();
		exit(0);
	}

	int ret;
	
	if (r_flag && !cfg.using_pcap_file)
	{
		if (tx80211_init(&cfg.in_tx, cfg.device, cfg.drivertype) <0)
		{
			perror("tx80211_init\n");
			printf("device: %s, drivertype: %d\n", cfg.device, cfg.drivertype);
			exit(0);
		}

		ret = tx80211_setmode(&cfg.in_tx, IW_MODE_MONITOR);
		if (ret != 0)
		{
			fprintf(stderr, "Error setting mode, returned %d.\n", ret);
			exit(0);
		}
							
	}
	if (c_flag && !cfg.using_pcap_file)
	{
	 	ret = tx80211_setchannel(&cfg.in_tx, cfg.channel);
	    if (ret < 0) 
		{
			fprintf(stderr, "Error setting channel, returned %d.\n", ret);
			exit(0);
		}
	}
		
	
}

int main(int argc, char **argv)
{
	parse_args(argc, argv, global_cfg);
	StationSetType S;

	if (global_cfg.just_printing)
	{
		printf("Printing %s\n", global_cfg.print_clientlist_fname);
		PrintStationSet(global_cfg.print_clientlist_fname);
		exit(0);
	}

	if (global_cfg.using_pcap_file)
		GenerateStationSetFromPcapFile(global_cfg.pcap_handle, S);	 //device is overloaded.
	else if (global_cfg.time > 0)
		GenerateStationSetFromInterface(global_cfg.pcap_handle,S, global_cfg.time);	 //device is overloaded.
	else //want to loop forever on a real interface.
	{
		int old_size = 0;
		signal(SIGINT, sig_handler);
		signal(SIGTERM, sig_handler);
		while( still_running)
		{
			GenerateStationSetFromInterface(global_cfg.pcap_handle,S, 2);	 //device is overloaded.
			if (S.size() > old_size)
			{
				printf("%d New STA found.\n", S.size() - old_size);
				printf("%s\n", StationSetToString(S).c_str());
				old_size = S.size();
			}
			
		}
	}
	if (global_cfg.verbose)
	{
			printf("Got %d Stations:\n", S.size());
			printf("Stations:\n%s\n", StationSetToString(S).c_str());
	}

	if (strlen(global_cfg.output_filename) > 0)
		if (!WriteStationSetToFile(global_cfg.output_filename, S))
		{
			fprintf(stderr, "Error writing stationset file to %s\n", global_cfg.output_filename);
		}
		else
			fprintf(stderr, "Results written to %s\n", global_cfg.output_filename);
	
	exit(0);
}



void PrintStationSet(char *fname)
{
	StationSetType S;
	if (ReadStationSetFromFile(fname, S) == false)
		printf("Error reading staiton list from file.\n");  

	printf("%s\n", StationSetToString(S).c_str());
}

void GenerateStationSetFromPcapFile(PcapFileHandle &handle, StationSetType &S)
{
	
	PcapPacketList PktList;
	Packet_80211List WifiPktList;

	Pcap_Packet PcapPkt;
	while (GetPcapPacket(handle, PcapPkt))
		PktList.push_back(PcapPkt);
	PcapPacketListToPacket_80211List(PktList, WifiPktList);

	UpdateStationSet(S, WifiPktList);

}

void GenerateStationSetFromInterface(PcapFileHandle &handle, StationSetType &S, int secs)
{
	
	PcapPacketList PktList;
	Packet_80211List WifiPktList;

	BasicPcapThread PacketThread(global_cfg.pcap_handle.p, &PktList);
	PacketThread.start_capture();
	sleep(secs);
	PacketThread.stop_capture();
	
	PcapPacketListToPacket_80211List(PktList,WifiPktList);	
	printf("Got %d wifi packets\n", WifiPktList.size());

	UpdateStationSet(S, WifiPktList);
}

void GenerateStationSet()
{
	
	PcapPacketList PktList;
	Packet_80211List WifiPktList;
	StationSetType S;

	if (!global_cfg.using_pcap_file)
	{
		BasicPcapThread PacketThread(global_cfg.pcap_handle.p, &PktList);
		PacketThread.start_capture();
		sleep(global_cfg.time);
		PacketThread.stop_capture();
	}
	
	else
	{
		Pcap_Packet PcapPkt;
		while (GetPcapPacket(global_cfg.pcap_handle, PcapPkt))
			PktList.push_back(PcapPkt);
	}
	PcapPacketListToPacket_80211List(PktList, WifiPktList);

	UpdateStationSet(S, WifiPktList);

	if (strlen(global_cfg.output_filename) > 0)
	if (!WriteStationSetToFile(global_cfg.output_filename, S))
	{
			fprintf(stderr, "Error writing stationset file to %s\n", global_cfg.output_filename);
	}
	else
			fprintf(stderr, "just wrote file.\n");

	if (global_cfg.verbose)
	{
	
		printf("Got %d Stations:\n", S.size());
		printf("Stations:\n%s\n", StationSetToString(S).c_str());
	}

}



