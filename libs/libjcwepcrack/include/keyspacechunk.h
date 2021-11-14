#ifndef _KEYCHUNK_H
#define _KEYCHUNK_H
#include "key.h"

//this class is usually handed back filled in by the KeySpaceManager.getSpaceChunk.
//the KeySpaceManager will automatically allocate/free the memory for iValue.
class KeySpaceChunk
{
	public:
	u_int32_t session_id; // needed to submit the completed chunk
	u_int32_t range;// how many increments we should try
	struct timeval in_time;
	// out_time and compute time are set on return by server. CLient should ignore them
	// byte ordering may not match up for client anyway.
	struct timeval out_time;
	u_int32_t       compute_time; //in_time - out_time

	Key iKey;
	KeySpaceChunk();
	KeySpaceChunk & operator = (const KeySpaceChunk &right);
	serialized_object serialize();
	u_int32_t serialize(FILE *fp);
	~KeySpaceChunk();
};

KeySpaceChunk unserializeKeySpaceChunk(serialized_object);
KeySpaceChunk unserializeKeySpaceChunk(FILE *fp);

#endif 

