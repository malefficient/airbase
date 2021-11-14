#ifndef _CRACK_H
#define _CRACK_H
#include "airware.h"
#include <algorithm>
#include <netinet/in.h> //ntohl
#include <unistd.h> // sleep

//This file contains misc routines that could be useful to various
//programs analyzing statistical attacks on wep.

#define MAX_BSSIDS_WITH_IVMAPS_PER_LIST 20 
//we set a max because each one may allocate 16M of memory for iv maps.
//if this is set to 0 it is ignored.


//----------Functions for working with aircrack compatible iv files-------
//Opens up IV file. reads and verifys magic num. returns NULL on error.
FILE *OpenReadIvFile(char *filename, char *mode="r");
//Opens up a iv file for writing and puts the iv-file magic number in front.
FILE *OpenWriteIvFile(char *filename, char *mode="w+"); 


//If the bssid contained in data is -already- in List then this function
//will append the IVS in data to the bssid in list. If the bssid is not
//already in List it is simply added.
//This function will never fail to add IV data. It may fail to add a entirely
//new bssid if it cant allocate memory for an iv_map for it, or if dont_create_bssids
//is true.
//
//running out of memoryshould never happen under normal circumstance. Only if someone was specifically
//trying to DoS the code. It only really makes sense to check the return vale of this
//the first time you call it with data from many bssids. 

//NOTE: bssid_data's do not allocate there own iv_present_map! 
//if you want 16M's of memory you have to allocate it yourself.
//
int AddIvsToBssidData(bssid_data & dest, bssid_data & source, bool iv_filtering);
bool AddBssidDataToList(BssidData_List &List, bssid_data &data, bool iv_filtering );
void FreeBssidList(BssidData_List &List);

//This function will read as many ivs as it can from fp. If a fread in it fails
//it gracefully backs the FP back up to a place where it can restart, adds all the
//data is has gathered so far, and returns. This is function is NOT thread-safe.
//Do NOT call it from multiple threads. It is, however, safe to call it on a file
//being written by an arbitrary program while the other program is writing to the iv file
//and there is no syncronization/communication on the two. (e.g. airodump)

void initialize_read_iv_data();
int ReadIvData(FILE *fp, BssidData_List &MyBssidList, bool iv_filtering);
int ReadPcapData(char *filename, BssidData_List &List, bool iv_filtering, bool verbose=true );

int WriteBssidListToFile(char *filename, BssidData_List &List);
int AppendBssidDataToFile(FILE *iv_file, BssidData_List & BssidList);

bssid_data ChooseBssidFromList(BssidData_List &List);

bool get_small_wep_pkts(char *filename, u_int8_t *bssid, Packet_80211_data &Smallest, Packet_80211_data &SecondSmallest);
void Iv2Offset(unsigned char *IV, unsigned int &offset);
#endif
