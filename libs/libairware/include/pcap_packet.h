#ifndef _PACKET_H
#define _PACKET_H

using namespace std;
//MAX_PCAP_DATA_SIZE will be allocated for every Pcap_Packet type. 
//This just makes memory mgmt easier as we dont have to grow it
//with routines who append arbitrary data.
#define MAX_PCAP_DATA_SIZE 2312
#include "jc-util.h"
#include "datatypes.h"
// If you have build issues with pcap.h read pcap_error.readme
#include  <pcap.h>
#include <iostream>
#include <string>
#include <list>
using std::string;
class Pcap_Packet
{
	private:
	void BreakHere(); //convenient place for debugger.
	protected: 
	bool initialized;
	//u_char_t *data;
	public:
	void check_initialized(char *s);
	bool is_initialized() { return initialized;}

	//By any OOP thinking data oughta be private. But, you're
	//b
	//a 3rd degree blackbelt and know what your, doing. Right?
	u_char_t *data;
	u_int8_t *payload_ptr; //payload here starts with the first byte of data.
	//struct pcap_pkthdr pcap_hdr;
	//----Instead of keeping the pcap_pkthdr struct we have our own
	//fields and re-create it when serializing.
	
	struct timeval ts;
	u_int32_t tot_length; //pcap_hdr.len (total length of packet)
	u_int32_t length; //pcap_hdr.caplen  (length of packet present);
	u_int32_t dlt;

	Pcap_Packet(); //for ninjas only.
	
	//Makes its own copy of data.
	Pcap_Packet(const Pcap_Packet &temp); //copy ctor
	bool Init(const Pcap_Packet &temp);
	Pcap_Packet(u_int32_t data_len, u_int8_t *data, u_int32_t dlt);	//make a packet from a user controlled buffer.
	bool Init(u_int32_t data_len, u_int8_t *data, u_int32_t dlt);	//make a packet from a user controlled buffer.
	
	//makes its own copy of data.
	Pcap_Packet(struct pcap_pkthdr p, u_char_t *data, u_int32_t dlt);
	bool Init(struct pcap_pkthdr p, u_char_t *data, u_int32_t dlt);
	void AppendData(u_int32_t size, u_int8_t *new_data);	
	

	struct pcap_pkthdr get_pcap_hdr();
	void set_pcap_hdr(struct pcap_pkthdr hdr);

	Pcap_Packet& operator = (const Pcap_Packet &right);
	//the << oper might be implmented to append to a file.
	//friend std::ostream &operator<<(std::ostream &o,Pcap_Packet &f);
	bool friend operator ==(const Pcap_Packet &left, const Pcap_Packet &right);	
	// the hex_dump version is the same in the base class, however this allows
	// descendants to override print while leaving the generic hex_dump alone
	string get_hex_dump();
	void print_hex_dump();
	void print_hex_dump(ostream &s);
	void print();
	void print(ostream &s);
	string toString(bool recurse = true );
	
	//write itself out as a pcap packet, appended to fp which is the caller must already
	//have opened.
	bool append_to_file(FILE *fp);
	~Pcap_Packet();


};

typedef list<Pcap_Packet> PcapPacketList;
typedef list <PcapPacketList> PcapPacketChain;
void ReallyBreakHere();
#endif
