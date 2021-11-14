#include "timepiece.h"
#include <stdlib.h>

void timepiece::normalize()
{
    while (Time.tv_usec >= 1000000 || Time.tv_usec < 0) //nasty signedness.
     {
		Time.tv_sec++;
		Time.tv_usec -= 1000000;
     }
}

timepiece::timepiece(const struct timeval &T)
{
	Time = T;
	normalize();
}

timepiece::timepiece(const timepiece &T)
{
	Time = T.Time;
	normalize();
}

timepiece::timepiece()
{
	Time.tv_sec = Time.tv_usec = 0;
}

timepiece timepiece:: operator + (const timepiece &right) const
{
	timepiece result;

    result.Time.tv_sec = Time.tv_sec + right.Time.tv_sec;
    result.Time.tv_usec = Time.tv_usec + right.Time.tv_usec;
    if (result.Time.tv_usec >= 1000000)
     {
		result.Time.tv_sec++;
		result.Time.tv_usec -= 1000000;
      }
	return result;
}

timepiece timepiece:: operator * (int x) const
{
	timepiece result;

    result.Time.tv_sec = Time.tv_sec * x;
    result.Time.tv_usec = Time.tv_usec *x;
    while (result.Time.tv_usec >= 1000000)
     {
		result.Time.tv_sec++;
		result.Time.tv_usec -= 1000000;
      }
	return result;
}

timepiece timepiece:: operator / (int x) const
{
	timepiece result;

    result.Time.tv_sec = Time.tv_sec / x;
    result.Time.tv_usec = Time.tv_usec /x;
    while (result.Time.tv_usec >= 1000000)
     {
		result.Time.tv_sec++;
		result.Time.tv_usec -= 1000000;
      }
	return result;
}

timepiece timepiece:: operator - (const timepiece &right) const
{
	timepiece result;

    result.Time.tv_sec = Time.tv_sec - right.Time.tv_sec;
    result.Time.tv_usec = Time.tv_usec - right.Time.tv_usec;
    if (result.Time.tv_usec < 0)
     {
		result.Time.tv_sec--;
		result.Time.tv_usec += 1000000;
      }
	return result;
}

bool timepiece::operator == (const timepiece &right) const
{
	if (Time.tv_sec == right.Time.tv_sec && (Time.tv_usec == right.Time.tv_usec))
		return true;
	else
		return false;
}

bool timepiece::operator != (const timepiece &right) const
{
	return (!  (*this == right));
}
		
bool timepiece::operator > (const timepiece &right) const
{
	if ( Time.tv_sec - right.Time.tv_sec > 0)
		return true;
	else if (Time.tv_sec - right.Time.tv_sec < 0)
		return false;

	//--if we reach this point, sec values equal.
	if  (Time.tv_usec - right.Time.tv_usec > 0)
		return true;
	else if (Time.tv_usec - right.Time.tv_usec < 0)
		return false;
	else 
		return false; // (Time.tv._usec == right.tv_usec)
	
}

bool timepiece::operator >= (const timepiece &right) const
{
	if(   (*this) == right)
		return true;
	if ( (*this) > right)
		return true;
	else
		return false;
}
bool timepiece::operator < (const timepiece &right) const
{
	return ! ( (*this >= right));
}

bool timepiece::operator <= (const timepiece &right) const
{
	return ! ( (*this) > right);
}

timepiece  timepiece::operator = (struct timeval right)
{
	(*this).Time = right;
	return *this;
}

string timepiece::toString()
{
	char tmp[256];
	snprintf(tmp, sizeof(tmp), "Secs: %u, u_secs: %u [(remainder of  %.6f secs]", Time.tv_sec, Time.tv_usec, 
	 ( (Time.tv_usec / (float) 1000000)) );
	string s = tmp;
	return s;
}

//--Thats it for mthods, now for outside helper functions


/*
int IteratePktList(PktListType &PktList, PktListType::iterator start, PktListType::iterator& stop, int u_secs)
{
    timepiece tmp;
    tmp.Time.tv_usec = u_secs;
    return IteratePktList(PktList, start, stop, tmp);
}

// This function will return the number of packets that were sent After start
// Who's initial transmission time (recorded in the MAC time stamp) falls within the
// window of time start.mac_time + duration. 
// e.g. if you have 
//  [P1    ]  [P2    ] [P3    ]
//  ^start    1s       1s 
//IteratePktList(List, P1, stop, 0.5s) will return 0 and stop will point at P2
//IteratePktLisT(List, P1, stop, 1.0s) will return 1 and stop will point at P3
//Returns -1 on error. 

int IteratePktList(PktListType &PktList, PktListType::iterator start, PktListType::iterator& stop, timepiece duration)
{
    PktListType::iterator Walker;
    Walker = start;

    struct mac_time start_packet_mac_time;
    timepiece start_packet_mactimepiece;

    Packet_80211 StartPacket;

    StartPacket.Init(*start);
    StartPacket.get_mac_time(&start_packet_mac_time);
    MacTimeConverter(start_packet_mac_time, start_packet_mactimepiece);


    struct mac_time next_packet_mac_time;
    timepiece next_packet_mactimepiece;
    timepiece time_passed;
    int NumWalked = 0;
	Walker++; //Get to first packet.
	if (Walker == PktList.end())
	{
		printf("Called IteratePktList with a list of one..prolly an error.\n");
		stop=start;
		return -1;
	}

    do
    {
		printf("Walking\n");
        Packet_80211 NextPacket;
        NextPacket.Init(*Walker);
		if (!NextPacket.is_initialized())
		{
			printf("IteratePktList::Errror, Couldnt initialize NextPacket from:\n");
			Walker->print();
			printf("Bailing.\n");
			exit(0);
		}
							
        NextPacket.get_mac_time(&next_packet_mac_time);
        if (! MacTimeConverter(next_packet_mac_time, next_packet_mactimepiece))
        {
            printf("Error converting mac time.\n"); //XXX: Could fall back on pcap timestamps, maybenota  good idea though.
            exit(0);
        }
        time_passed = next_packet_mactimepiece - start_packet_mactimepiece;
		printf("Time passed so far: %s\n", time_passed.toString().c_str());
        Walker++;
        NumWalked++;
    }
    while (( time_passed < duration) && Walker != PktList.end());
	NumWalked--;
	Walker--;
	stop = Walker;
	return NumWalked; 
	//NumWalked is --[start ... stop] stop - start. If numwalked returns one then there is nothing between start and stop
}
*/
