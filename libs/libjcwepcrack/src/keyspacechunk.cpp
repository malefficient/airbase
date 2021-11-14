#include "keyspacechunk.h"
#include <iostream>
KeySpaceChunk :: KeySpaceChunk()
{
	#ifdef JC_DEBUG
	cout << "Creating a new KeySpaceChunk\n";
	#endif
	session_id = 0;
	range = 0;
	//dont both initializing time fields. no sensible values.
}

KeySpaceChunk&  KeySpaceChunk :: operator = (const KeySpaceChunk &right)
{
#ifdef JC_DEBUG
	cout << "Chunk assign oper\n";
#endif
	this->session_id = right.session_id;
	this->range = right.range;
	this->in_time = right.in_time;
	this->out_time = right.out_time;
	this->compute_time = right.compute_time;
	this->iKey = right.iKey; //key has working assigment operator

	return *this;
}

KeySpaceChunk unserializeKeySpaceChunk(serialized_object s)
{
	KeySpaceChunk chunk;
	unsigned char *p = s.data;
	memcpy(&chunk.session_id, p, sizeof(chunk.session_id));
	chunk.session_id = ntohl(chunk.session_id);
	p += sizeof(chunk.session_id);
	s.size -= sizeof(chunk.session_id);

	memcpy(&chunk.range, p, sizeof(chunk.range));
	chunk.range = ntohl(chunk.range);
	p+= sizeof(chunk.range);
	s.size -= sizeof(chunk.range);

	memcpy(&chunk.in_time, p, sizeof(chunk.in_time));
	p+= sizeof(chunk.in_time);
	s.size -= sizeof(chunk.in_time);
	
	memcpy(&chunk.out_time, p, sizeof(chunk.out_time));
	p += sizeof(chunk.out_time);
	s.size -= sizeof(chunk.out_time);

	memcpy(&chunk.compute_time, p,  sizeof(chunk.compute_time));
	chunk.compute_time = ntohl(chunk.compute_time);
	p += sizeof(chunk.compute_time);
	s.size -= sizeof(chunk.compute_time);

	//this is kind of hackish but oh well. We've moved along the serialized object
	//to a key and we just modify the serialized_object struct a bit and pass it
	//to unserializeKey. It works.
	s.data = p;
	chunk.iKey = unserializeKey(s);
	
	return chunk;
}

serialized_object KeySpaceChunk :: serialize()
{
	serialized_object ser_obj;
	serialized_object serialized_key;
	u_int32_t tmp;

	//tricky
	serialized_key = iKey.serialize();
	u_int32_t mysize = sizeof(session_id) + sizeof(range) + sizeof(in_time) + sizeof(out_time) + sizeof(compute_time)
						+ serialized_key.size;

	unsigned char *p =  new unsigned char[mysize];
	if (!p)
	{
		cout << "KeySpaceChunk::serialize (u_chat_t **) could not allocate memory!\n";
		exit(0);
	}
	ser_obj.size = mysize;
	ser_obj.data = p;
	
	tmp = htonl(session_id);
	memcpy(p, &tmp, sizeof(session_id));
	p += sizeof(session_id);
	
	tmp = htonl(range);
	memcpy(p, &tmp, sizeof(range));
	p+= sizeof(range);

	
	memcpy(p, &in_time, sizeof(in_time));
	p+= sizeof(in_time);
	
	memcpy(p, &out_time, sizeof(out_time));
	p += sizeof(out_time);

	tmp = htonl(compute_time);
	memcpy(p, &compute_time, sizeof(compute_time));
	p += sizeof(compute_time);
	//tricky. Its occured to me that a much more robust serializing system would be nice but this
	//is all we need it for so im not going to build on it.
	memcpy(p, serialized_key.data, serialized_key.size);
	delete []serialized_key.data;	
	return ser_obj;
}

u_int32_t KeySpaceChunk :: serialize(FILE *fp)
{
	u_int32_t mysize = sizeof(session_id) + sizeof(range) + sizeof(in_time) + sizeof(out_time) + sizeof(compute_time);

	fwrite(&session_id, sizeof(session_id), 1, fp);
	fwrite(&range, sizeof(range), 1, fp);
	fwrite(&in_time, sizeof(in_time), 1, fp);
	fwrite(&out_time, sizeof(out_time), 1, fp);
	fwrite(&compute_time, sizeof(compute_time), 1, fp);
	if (ferror(fp))
	{
		cout << "fatal error in KeySpaceChunk::serialize\n";
		perror("fwrite:");
		exit(0);
	}
	mysize += iKey.serialize(fp);

	return mysize;
}

KeySpaceChunk :: ~KeySpaceChunk()
{
	#ifdef JC_DEBUG
	cout << "Destroying a KeySpaceChunk\n";
	#endif
}



KeySpaceChunk unserializeKeySpaceChunk(FILE *fp)
{
	KeySpaceChunk k;
	fread(&k.session_id, sizeof(k.session_id), 1, fp);
	fread(&k.range, sizeof(k.range), 1, fp);
	fread(&k.in_time, sizeof(k.in_time), 1, fp);
	fread(&k.out_time, sizeof(k.out_time), 1, fp);
	fread(&k.compute_time, sizeof(k.compute_time), 1, fp);
	if (ferror(fp))
	{
		cout << "fatal error in unserializeKeySpaceChunk\n";
		perror("fread:");
		exit(0);
	}
	k.iKey = unserializeKey(fp);

	return k;
}
