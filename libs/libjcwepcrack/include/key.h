/*
 *  KeyBase.h
 *  jc-wepcrack
 *
 *  Created by johnny cache on Sun Jul 11 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _KEY_H
#define _KEY_H
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstring> //memcpy
#include "config.h"
#include "wepcrack_datatypes.h"
#include <iostream>
#include <string>
using namespace std;
// POSSIBLE WAY TO FIX EVERYTHING:
// STOP ALLOCATING MEMORY DYNAMICALLY IN THIS CLASS. I FUCKING GIVE UP
// JUST ALLOCATE A STATIC PORTION OF 256 BYTES ON EVERY CTOR AND LET SIZE
// ONLY BE <= THAT. IT MAKES LIFE SO MUCH EASIER.
//
// BUG: comparing two keys with difference sizes but identical values (say 0) fails.
// BUG: the key class will only work with keys > 1 byte. This is due to the incrementer logic.
// BUG: the += operator is implemented in terms of the ++. :( This doesnt really slow 
// things down until you start +='s by more than 0x0010000000 So its OK for us
// since it takes a lot of time to crack that many keys.


class Key
{
//private:
private:
	//NOTE: if these ever get changed be sure to look at serialize/unserialize
    u_int32_t size;
	u_char_t MAXCHAR; //value a byte can be between, most likely 0xFF but hey.
    
public:
    unsigned char value[MAX_KEY_SIZE_IN_BYTES]; //actual value. We leave it public because we know what we're doing.
	//MAX_KEY_SIZE TAKES on a double meaning here. It is intended to  mean the maximum size of
	//a wep key we will crack. This is also what we want a default key to construct as so 
	//
	Key(int _size =4, unsigned char max=0xff);
	void setSize(int _size) {if (size <= 1)  cout << "Invalid Key size\n"; else size = _size;}
	int getSize() { return size; }
	string getString();
	Key& operator =(const Key &right); // or this.
	bool operator <(const Key &right) const; // or this.
	bool operator ==(const Key &right); // or this.
	void operator ++(int in); // no idea why this needs to accept an int.
	void operator +=(int in);
	friend ostream &operator<<(ostream &o, const Key &k);
	serialized_object serialize();// save state to memory. serialized object returned. callee must delete memory returned
	u_int32_t serialize(FILE *fp);// save state to file . size returned. 0 if error. 
       ~Key();
    
};
Key unserializeKey(serialized_object);//make a key from a serialized key
Key unserializeKey(FILE *fp);//make a key from a serialized key

#endif

