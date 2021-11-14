/*
 *  Key.cpp
 *  jc-wepcrack
 *
 *  Created by johnny cache on Sun Jul 11 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */
#include <iostream>
#include <stdio.h>
#include "key.h"

Key::Key(int _size, unsigned char _max) 
{
    size =_size;
    MAXCHAR = _max;
    if (size <= 1)
     {
		//throw Exception;
		cout << "Keys cant be <= 1 byte\n";
		exit(0);
		//value = NULL;
	return;
     }
	if (size > MAX_KEY_SIZE_IN_BYTES)
	{
		fprintf(stderr, "Key size to big. MAX_KEY_SIZE_IN_BYTES = %d\n", MAX_KEY_SIZE_IN_BYTES);
		exit(0);
	}	

    memset(value, 0, sizeof(value));
}

bool Key:: operator == (const Key &right)
{
#ifdef JC_DEBFG
	cout << "comparing myself to " << right << endl;
#endif
	if ( size != right.size)
	{
//		fprintf(stderr, "DONT COMPARE KEYS OF DIFFERENT SIZES TOGETHER\n");
		return false;
	}
	if (memcmp(value, right.value, size) == 0)
		return true;
	else
		return false;	
}

Key& Key:: operator = (const Key &right)
{
	#ifdef JC_DEBUG
	cout << "Key assign operator\n";
	#endif
	this->size = right.size;
	this->MAXCHAR = right.MAXCHAR;

	memcpy(this->value, right.value, right.size);
	return *this;
}

bool Key:: operator < (const Key &right) const //WTF is this const doing?
{
	int rc;

	#ifdef JC_DEBUG
	cout << "comparing myself" <<  (*this) << "to " << right << endl;
	#endif
	if ( size != right.size)
	{
		fprintf(stderr, "DONT COMPARE KEYS OF DIFFERENT SIZES TOGETHER\n");
		return false;
	}

	 rc = memcmp(value, right.value, size);
	 if (rc < 0)
		return true;
	else
		return false;	
}

void Key:: operator ++ (int i)
{
	this->operator +=(1);
}

//THE PROBLEM ON THE PS3 IS RIGHT HERE!
void Key:: operator += (int in)
{
	int i;
	u_int16_t carry;
	//unsigned long in_ = (unsigned long)in;
	u_int32_t in_ = (u_int32_t) in;
	u_int8_t  valueb[MAX_KEY_SIZE_IN_BYTES];
	bzero(valueb, MAX_KEY_SIZE_IN_BYTES);
	//fprintf(stderr, "Key::+= operator, in = %d in_ = %d\n", in, in_);
	//fprintf(stderr, "Key::+= operator size = %d,  sizeof(valueb) = %d, MAX_KEY_SIZE_IN_BYTES = %d\n", size, sizeof(valueb), MAX_KEY_SIZE_IN_BYTES);
	//fflush(stderr);
	//sleep(1);

	//the following code is kind of awkard because 99% of the time the first case will be taken,
	//and i want it to go fast.
	if (size >= 4)
	{
		valueb[size - 1] = in_ & 0xff;
		valueb[size - 2] = (in_ >> 8) & 0xff;
		valueb[size - 3] = (in_ >> 16) & 0xff;
		valueb[size - 4] = (in_ >> 24) & 0xff;
	}
	else if (size == 3)
	{
		valueb[size - 1] = in_ & 0xff;
		valueb[size - 2] = (in_ >> 8) & 0xff;
		valueb[size - 3] = (in_ >> 16) & 0xff;
	}
	else if (size == 2)
	{
		valueb[size - 1] = in_ & 0xff;
		valueb[size - 2] = (in_ >> 8) & 0xff;
	}
	else if (size == 1)
	{
		valueb[size - 1] = in_ & 0xff;
	}

	//fprintf(stderr, "Key::+= operator all done setting up valueb\n");
	//fflush(stderr);

/*
	printf("value=");
	for(i = 0; i < size; i++)
		printf("%02x", value[i]);
	printf("+valueb=");
	for(i = 0; i < size; i++)
		printf("%02x", valueb[i]);
	printf("\n");
*/

	carry = 0;
	for(i = size - 1; i >= 0; i--) {
		carry += valueb[i] + value[i];
		value[i] = carry & 0xff;
		carry >>= 8;
	}
/*
	printf("=");
	for(i = 0; i < size; i++)
		printf("%02x", value[i]);
	printf("\n");
//	sleep(100);
*/
}
			

Key::~Key()
{
		int i;
#ifdef JC_DEBUG
	cout << "Key::" << *this << "~Key\n";
#endif
	i++;
}



serialized_object Key :: serialize ()
{

	u_int32_t mysize = 0;
	u_int32_t n_size; // network_size;
	serialized_object ser_key;

	mysize = sizeof(size) + sizeof(MAXCHAR) + size;
	u_char_t *p = new u_char_t[mysize];
	
	if(p == NULL)
	{
		cout << "Could not allocate memory in Key::serialize. exiting\n";
		exit(0);
	}
	//printf("t ix %x\n, allocated memory at %x\n", t, *t);
	ser_key.data = p;

	n_size = htonl(size);
	memcpy(p, (void *) &n_size, sizeof(n_size));
	p += sizeof(size);
	memcpy(p, (void *) &MAXCHAR, sizeof(MAXCHAR));
	p += sizeof(MAXCHAR);
	memcpy(p, value, size); // this is different
	
	ser_key.size = mysize;
	
	return ser_key;

}

Key unserializeKey(serialized_object object)
{
	u_char_t *p;

	u_int32_t k_size;
	u_char_t k_maxchar;
	
	Key default_key;
	p = object.data;
	memcpy( (void *) &k_size, p, sizeof(k_size));
	k_size = ntohl(k_size);
	
	if (k_size > MAX_KEY_SIZE_IN_BYTES)
	{
		fprintf(stderr, "Key::unserialziiedKey, k_size > MAX_KEY_SIZE_IN_BYTES\n");
		printf("k_size = 0x%08X\n", k_size);
		exit(0);
		//return default_key; //if i did this right id throw something
	}

	p += sizeof(k_size);
	memcpy( (void *) &k_maxchar, p, sizeof(k_maxchar));
	p += sizeof(k_maxchar);

	Key k(k_size, k_maxchar);
	memcpy(k.value, p, k_size); // different
	return k;
}

u_int32_t Key :: serialize (FILE *fp)
{

	u_int32_t mysize = 0;
	int rc;
	mysize = sizeof(size) + sizeof(MAXCHAR) + size;

	fwrite( (void *) &size, sizeof(size), 1, fp);
	fwrite( (void *) &MAXCHAR, sizeof(MAXCHAR), 1, fp);
	fwrite( (void *) value, size, 1, fp); // diffferent
	if (ferror(fp))
		{
			cout << "fatal error in Key::serialize. fwrite failed.\n";
			perror("fwrite:");
			exit(0);
		}
#ifdef JC_DEBUG
	printf("wrote key in %d bytes\n", mysize);
#endif
	return mysize;
}
// nor this

Key unserializeKey(FILE *fp)
{
	u_int32_t k_size;
	u_char_t k_maxchar;
	int rc; 
	u_int32_t read_size = 0;

	fread( (void *) &k_size, sizeof(k_size), 1, fp);
	fread( (void *) &k_maxchar, sizeof(k_maxchar), 1, fp);
	if (ferror(fp))
		{
			cout << "fatal error in unserialize key. fread failed.\n";
			perror("fread:");
			exit(0);
		}
	read_size += sizeof(k_size) + sizeof(k_maxchar);
#ifdef JC_DEBUG
	printf("unserializing a key with size %d and maxchar %x\n", k_size, k_maxchar);
#endif
	Key k(k_size, k_maxchar);

	rc = fread( (void *) k.value, k_size, 1, fp); // diffferent
	if (ferror(fp))
		{
			cout << "fatal error in unserialize key. fread failed.\n";
			perror("fread:");
			exit(0);
		}
	if (rc != 1)
		printf("Error reading key. didnt finish\n");
	else
	read_size += k_size;
#ifdef JC_DEBUG
	printf("Read in %d bytes to unserialize key \n", read_size);
#endif
	return k;
}
string Key::getString()
{
	string s;
	string t;
    char tmp[256]; 
    int i;
    for ( i = 0; i < size; i++)
       {
		snprintf(tmp,sizeof(tmp), "%2.2x:", value[i]);
		t = tmp;
		s = s + t;
	   }
	return s;

}

// This is not part of the class, it is a friend
//ostream &operator<<(ostream &o, const Key &k)
ostream &operator<<(ostream &o, const Key &k)
{
	//o << k.getString();
	string s;
	string t;
    char tmp[256]; 
    int i;
    for ( i = 0; i < k.size; i++)
       {
		snprintf(tmp,sizeof(tmp), "%2.2x:", k.value[i]);
		t = tmp;
		s = s + t;
	   }
	o << s;
	return o;

}

