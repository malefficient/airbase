#include "802fingerlib.h"
#include "802fingertest.h"

bool timepiece_Tester();
bool stationlist_Tester(char *fname);


int main(int argc, char **argv)
{
	printf("lib802fingertest.h: johnycsh@gmail.com\n");
	if (argc != 2)
		printf("usage: ./802fingertest filename\n");
	//stationlist_Tester(argv[1]);
	exit(0);
	timepiece_Tester();
}

//removed until i feel like fixing it. i only vaguely remember writing this code..
/*
bool stationlist_Tester(char *fname)
{

	printf("fname is %s\n", fname);
	u_char a_addr[] = { 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6};
	u_char b_addr[] = { 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6};
	u_char c_addr[] = { 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6};
	u_char d_addr[] = { 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6};

	Station S1(a_addr, b_addr);
	Station S1_prime = S1;

	if (S1_prime == S1)
		printf("Station:: == and = operator pass test\n");
	else
		printf("Station:: Error in == or = operator\n");

	Station S2(c_addr, d_addr);
	
	printf("S1\n %s\n", S1.toString().c_str());
	printf("S2\n %s\n", S2.toString().c_str());
	
	StationSetType L1, L2;
	L1.insert(S1);
	L1.insert(S1_prime);
	L1.insert(S2);
	if (WriteStationSetToFile("Test1.dat", L1) == false)
		printf("Error Writing Station list to file.\n");

	if (ReadStationSetFromFile("Test1.dat", L2) == false)
		printf("Error reading staiton list from file.\n");

	printf("List 1:\n");
	printf("%s", StationSetToString(L1).c_str());
	printf("List 2:\n");
	printf("%s", StationSetToString(L2).c_str());


	int ret;
	
	Packet_80211List List;
	Packet_80211List::iterator iter;
	ret = PcapFileToPacket_80211List(fname, List);
	printf("Read in %d good 80211 packets from %s\n", ret, fname);
	printf("list returns %d for size()\n", List.size());
	StationSetType S;
	ret = UpdateStationSet(S, List);
	printf("found %d clients\n", ret);
	printf("New StationSet: %s\n", StationSetToString(S).c_str());

	return true;
}
*/

bool timepiece_Tester()
{

    timepiece T1, T2;
	timepiece T3;
    gettimeofday(&T1.Time, NULL);
	sleep(1);
    gettimeofday(&T3.Time, NULL);
	

	T2 = T3 - T1;
	T2.Time.tv_usec=1000000  / 100;

    printf("T1: %s\nT2: %s\n", T1.toString().c_str(), T2.toString().c_str());

	if ( (T1 < T2))
		printf("T1 < T2\n");
	else
		printf("T1 not < T2\n");

	if ( (T1 <= T2))
		printf("T1 <= T2\n");
	else
		printf("T1 not <= T2\n");

	if ( (T1 == T2))
		printf("T1 == T2\n");
	else
		printf("T1 not == T2\n");
	
	if ( (T1 != T2))
		printf("T1 != T2\n");
	else
		printf("T1 not != T2\n");


	if ( (T1 > T2))
		printf("T1 > T2\n");
	else
		printf("T1 not > T2\n");

	if ( (T1 >= T2))
		printf("T1 >= T2\n");
	else
		printf("T1 not >= T2\n");

	T1 = T2;
	if (! (T1 == T2))
		printf("Failed = operator or on == operator..Test\n");
	else
		printf("Passed = operator and == operator Test.\n");	
	
	if ( ! (T1 != T2))
		printf("Passed != operator Test\n");
	else
		printf("Failed != operator Test\n");
}


