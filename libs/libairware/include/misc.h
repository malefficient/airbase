#ifndef _MISC_H
#define _MISC_H
#include <errno.h>
#include "airware.h"
extern int errno;

/*
void SWAP2(u_int16_t &x) 
{
	u_int_16_t t = x;
	u_int8_t 

}
*/
#define SWAP2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
#define SWAP4(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )
//Warning: This function is for convenience only in simple scripts. 
//It will open a pcap file for you if its not opened, but since you
//dont get a handle returned you cant do two files at once like this.
//none the less, its handy for simple script like programs that just
//iterate over a single file
bool airware_supported_dlt(int dlt);
bool GetPcapPacket( char *filename, Pcap_Packet& Passed_Packet);
bool GetPcapPacket( PcapFileHandle &FileHandle, Pcap_Packet &Passed_Packet);
bool OpenReadPcapFile(char *filename, PcapFileHandle &PcapFile);
bool ClosePcapFileHandle(PcapFileHandle &P);
bool OpenLivePcapDevice(char *device, PcapFileHandle &PcapHandle,  bool promisc=1, int snaplen = 65535, int to_ms=0);
//Opens up a pcap file for writing and puts a pcap_file header in front using DLT_IEEE80211
FILE *OpenWritePcapFile(char *filename, u_int32_t linktype= DLT_IEEE802_11, char *mode="w+");






//returns the number of bytes successfully convered from ascii to binary.
//that means 0 would indicate an error.  Caller specifys
//max # of bytes to be written via buffsize
//
int hexstring2value(char *string, u_int8_t *buff, int buffsize );

//SOME cards/drivers (Atheros - madwifi20050707  and aerojack (on OSX) append the 
//FCS to the pcap packet! These packets with the extra checksum on the end share the
//exact same DLT in pcap so you can not deterministically check at runtime. The best
//solution seems to be to compute our own checksum over Length - 4 bytes and compare 
//it to the last 4 bytes if the packet. If they match then with some relly high prob
//the packet already has a FCS we need to strip off. This could result in certain
//(highly unlikely) packets getting 4 bytes torn off. :( But theres nothing
//else we can do until dlt IEEE_802_11 becomes more precise. 
bool BufferHasFCSAlready(u_int8_t * buff, int length);
#endif
