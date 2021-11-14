#include "pcap_packet.h"

//if you do this you are responsible for setting up the packets interals yourself.
//you have to point data at something, and set length, tot_length, and ts.
Pcap_Packet :: Pcap_Packet()
{
	//printf("Pcap_Packet()\n");
	initialized = 0;
	data = new u_char_t[MAX_PCAP_DATA_SIZE];

	if (!data)
	{
			fprintf(stderr, "Could not allocate mem for packet. Dieing.\n");
			exit(-1);
	}
}



Pcap_Packet :: Pcap_Packet(u_int32_t data_len, u_int8_t *data, u_int32_t dlt)
{
	printf("Pcap_Packet(u_int32_t data_len, u_int8_T *data, u_int32_t dlt)\n");
	initialized = 0;
	data = new u_char_t[MAX_PCAP_DATA_SIZE];

	if (!data)
	{
			fprintf(stderr, "Could not allocate mem for packet. Dieing.\n");
			exit(-1);
	}

	Init(data_len, data, dlt);
}
//User wants to init without a pcap packet struct. we make a vanilla one for them.
bool Pcap_Packet ::Init(u_int32_t data_len, u_int8_t *data, u_int32_t dlt)
{
	struct pcap_pkthdr tmp;
	tmp.len = data_len;
	tmp.caplen = data_len;
	tmp.ts.tv_sec = 0;
	tmp.ts.tv_usec = 0;

	return Init(tmp, data, dlt);	

}
//You can create a Pcap_Packet straight from a pcap header and associated data
bool Pcap_Packet ::Init(struct pcap_pkthdr h, u_char_t * d, u_int32_t _dlt)
{

//	printf("Pcap_Packet::Init(pcap pkthdr, char *d)\n");
	initialized = 0;
	if (!d)
		{
			fprintf(stderr, "dont initalize packets /w null pointers!\n");
			exit(0);
		}
	ts = h.ts;
	tot_length = h.len;
	length = h.caplen; //amount of bytes captured
	dlt = _dlt;


	if (length > MAX_PCAP_DATA_SIZE)
		length = MAX_PCAP_DATA_SIZE;

	if (length < 0)
		return false;
	if (length > tot_length)
		return false;

	
	if (!data)
	{
			fprintf(stderr, "BigError. Data == null inside Pcap_Packet::Init(pkthdr, char *)\n");
			ReallyBreakHere();
			exit(-1);
	}

	//printf("memcpy: %p %p %d\n", data, d, length);
	memcpy(data, d, length); //get our own copy.

	payload_ptr = data;
	initialized = 1;
	return initialized;
}

Pcap_Packet :: Pcap_Packet(struct pcap_pkthdr h, u_char_t * d, u_int32_t _dlt)
{
	initialized = false;
//	printf("Pcap_Packet::Pcap_Packet(struct pcap_pkthdr..)\n");
	Init(h,d, _dlt);

}
//Or you can make one from another one.
Pcap_Packet :: Pcap_Packet(const Pcap_Packet &temp)
{
	initialized = false;
	data = new u_char_t[MAX_PCAP_DATA_SIZE];
	//printf("Pcap_packet::copy. temp.data = 0x%08X this.data = 0x%08X\n", temp.data, data);

	if (!data)
	{
			fprintf(stderr, "Could not allocate mem for packet. Dieing.\n");
			exit(-1);
	}
	
	Init(temp);
}

bool Pcap_Packet :: Init(const Pcap_Packet &temp)
{
	
//	printf("Pcap_Packet::Init(Pcap_Packet &)\n");
	initialized = false;
	dlt = temp.dlt;
	ts = temp.ts;
	tot_length = temp.tot_length;
	length = temp.length;

	if (length > MAX_PCAP_DATA_SIZE)
		length = MAX_PCAP_DATA_SIZE;

	if (length < 0)
		return false;
	if (length > tot_length)
		return false;


	if (!data)
	{
			fprintf(stderr, "BigError. Data == null inside Pcap_Packet::Init(Pcap_Packet &)\n");
			exit(-1);
	}
	
	//printf("memcpy: %p %p %d\n", data, temp.data, length);
	memcpy(data, temp.data, length); //get our own copy.

	payload_ptr = data;
	initialized = true;
	return initialized;	
}

//Note this isnt a member of the clas..
bool operator == (const Pcap_Packet &left, const Pcap_Packet &right)
{
	
	if (left.length != right.length)
	{
		//printf("== failed on length (left %d, right %d)\n", left.length, right.length);
		return false;
	}


	if (memcmp(left.data, right.data, left.length) == 0)
	{	
		//printf("== returning true.\n");
		return true;
	}
	else
	{
		//printf("== failed on memcp\n");
		return false;
	}
}

Pcap_Packet& Pcap_Packet :: operator = (const Pcap_Packet &temp)
{
	Init(temp);
	return *this;
}

string Pcap_Packet :: get_hex_dump()
{
		if (length <= 0 || length > MAX_PCAP_DATA_SIZE)
		{
			string s = "Pcap_Packet::get_hex_dump::Error. Invalid length." + length;
			return s;
		}
		return ::get_hex_dump(data, length); //scope out to global, defined into jc-util.
}
void Pcap_Packet :: print_hex_dump()
{

	print_hex_dump(cout);
}

void Pcap_Packet :: print_hex_dump(ostream &s)
{
	
	 s << get_hex_dump();
}
		
void Pcap_Packet :: print()
{
	print(cout);
}

void Pcap_Packet :: AppendData(u_int32_t size, u_int8_t *append_me)
{
	if (length + size > MAX_PCAP_DATA_SIZE)
	{
		printf("Error. Attempted to grow packet > %d In Pcap_Packet::AppendData\n", MAX_PCAP_DATA_SIZE);
		exit(0);
	}
	memcpy( &data[length], append_me, size);
	length += size;
	tot_length += size;
	
}
//Maybe we'll amke this append to a file..
//std::ostream &operator<<(std::ostream &o, Pcap_Packet &p)
//{
//	p.print(o);
//}

string Pcap_Packet :: toString( bool recurse)
{
	char tmp[256];
	if (length != tot_length)
		snprintf(tmp, sizeof(tmp), "Pcap:Length %d (Total-Length %d) ", length, tot_length);
	else
		snprintf(tmp, sizeof(tmp), "Pcap:Length %d ", length);
	string ret_str = tmp;

	snprintf(tmp, sizeof(tmp), "\tDLT: %s [%d]", pcap_datalink_val_to_name(dlt), dlt);
	ret_str += tmp;

	//nothing above us.
		return ret_str + "\n";
}

void Pcap_Packet :: print(ostream & s)
{
		
	check_initialized("Pcap_Packet::print");
	s << toString();
	return;
	
}

//write myself out as a pcap file.
bool Pcap_Packet::append_to_file(FILE *fp)
{
	int ret = 0;
	struct pcap_pkthdr pcap_hdr;
	
	if (!initialized)
	{
		cout << "Pcap_Packet::append_to_file called but not initialized.\n";
		exit(0);
	}
	pcap_hdr = get_pcap_hdr();	
	ret = fwrite(&pcap_hdr, sizeof(pcap_hdr), 1, fp);
	if (ret != 1)
	{
		perror("Pcap_Packet::append_to_file::fwrite::pcap_hdr\n");
		//exit(0);
		return false;
	}


	ret = fwrite(data, length, 1, fp);

	if (ret != 1)
	{
		perror("Pcap_Packet::append_to_file::fwrite::data\n");
		exit(0);
		return false;
	}

	return true;
}

struct pcap_pkthdr Pcap_Packet ::   get_pcap_hdr()
{
	check_initialized("get_pcap_hdr");
	struct pcap_pkthdr pcap_hdr;
	pcap_hdr.ts = ts;
	pcap_hdr.caplen = length;
	pcap_hdr.len = tot_length;
	return pcap_hdr;
}
      
void Pcap_Packet ::  set_pcap_hdr(struct pcap_pkthdr hdr)
{
	ts = hdr.ts;
	length = hdr.caplen;
	tot_length = hdr.len;
}

void Pcap_Packet :: check_initialized(char *s)
{
	if (initialized)
		return;
	//
	//If not, error so we can track down the bug.
	printf("Error! check_initialized failed. called from: %s\n", s);
	BreakHere();
	exit(0);

}
//This is here just do have something to make debugging easier.
void Pcap_Packet :: BreakHere()
{
	int i;
	i++;
	return;
}
void ReallyBreakHere()
{
	int i;
	i++;
	return;
}

Pcap_Packet::~Pcap_Packet()
{
		//printf("Pcap_Packet::~Pcap_Packet: Deleteing 0x%08X\n", data);
		//getchar();
		delete data;
}
