#ifndef TIMEPIECE_H
#define TIMEPIECE_H

/*
 * As an interesting note, on i386 linux the struct timval structs 
 * are defined as signed ints. e.g. you can add two timepieces, one with
 * negative values and you will do subtraction
 */

// 1 10th of a second 
// 1 1,000,000 = micro-second
using namespace std;
#include "airware.h"
#include <sys/types.h>
#include <sys/time.h>
#include <string>
#include <strings.h>
typedef vector<Pcap_Packet>PktListType; //put me elsewhere soon.
class timepiece
{

	public:
	struct timeval Time;

	timepiece(); //initializes Time to 0.
	timepiece(const struct timeval &T);
	timepiece(const timepiece &P);

	timepiece operator = (struct timeval right);
	timepiece operator + (const timepiece& right) const;
	timepiece operator - (const timepiece& right) const;
	timepiece operator / (int x) const;
	timepiece operator * (int x) const;
	string toString();	
	bool operator < (const timepiece &right) const;
	bool operator <= (const timepiece &right) const;
	bool operator != (const timepiece &right) const; 
	bool operator == (const timepiece &right) const; //trailing const means wont modify left operand
	bool operator >= (const timepiece &right) const ;
	bool operator > (const timepiece &right) const;
	void normalize(); 
};




#endif

