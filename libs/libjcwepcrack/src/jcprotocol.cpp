#include "jcprotocol.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "keyspacechunk.h"

static int connect_to_server( char *name, u_int16_t port)
{
	int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr; // connector's address information 
	if (name == NULL)
		return -1;

	if (port == 0)
		return -1;

	he = gethostbyname(name);
    if (he == NULL)
	{ 
    	perror("gethostbyname");
		return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
    	perror("socket");
		return -1;
    }

	memset(&their_addr, '\0', sizeof(their_addr));
    their_addr.sin_family = AF_INET;    // host byte order 
    their_addr.sin_port = htons(port);  // short, network byte order 
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) 
	{
    	perror("connect");
		return -1;
    }

    return sockfd;

	
}

static struct ProtocolHeader generate_header(u_int32_t type)
{
	struct ProtocolHeader header;
	header.flags = generate_flags();
	header.major = MAJOR_PROTOCOL_VERSION;
	header.minor = MINOR_PROTOCOL_VERSION;
	header.RequestType = type;
	header.size = 0; // not for me to set
//	printf("Set RequestType to %x\n", header.RequestType);

	return header;
}
//indirectly called by client. expects the protocol header to still be waiting on the wire.
//
static bool ReadSessionData(int fd,   struct SessionData &session_data)
{
	
	bool ret;
	int rc;
	struct ProtocolHeader h;
	u_int8_t *buf;

	//rc = recv(fd, (char *) &h, sizeof(h), 0);
	//if (rc != sizeof(h))
	ret = read_packet(fd, h, &buf);
	if (!ret)
	{
		fprintf(stderr, "couldnt read the reply header entirely. exiting\n");
		cout << "read " << rc << " bytes\n";
		cout << "was supposed to read " << sizeof(h) << "bytes\n";
		return false;
	}
#ifdef JC_DEBUG
		cout << "Successfuly read in header at " << sizeof(h) << " bytes\n";
#endif

	
	if (h.RequestType != DATA_REPLY)
	{
		cout << "invalid reply. Exiting\n";
		delete buf;
		exit(0);
	}
	
#ifdef JC_DEBUG
	print_header(h);
	printf("Packet has %x bytes of data:\n", h.size);	
#endif
	if (h.size == 0)
	{
		printf("Wrong damn packet size transmitted\n");
		return false;
	}
	//
	// Okay, start parsing that data.
    // network byte order: packet1size 4 bytes
    // neutral byte order: packet1IV    3 bytes
    // neutral byte order: packet1data
    // ------------------
    // network byte order: packet2size 4 bytes
    // neutral byte order: packet2IV   3 bytes
    // neutral byte order: packet2data
    // ------------------
    // network byte order: serializedKeySpaceChunk size 4 bytes
    // neutral byte order: serializedKeySpaceChunk
	
	u_int8_t *p;
	u_int8_t *buf_end; // for sanity checking.
	p = buf;
	buf_end = buf + h.size;
	if (buf_end < buf)
		{
			delete buf;
			return false;
		}

	if (p + sizeof(session_data.packet1size) > buf_end) // we got a nastygram, illegal bounds
	{
		delete buf;
		return false;
	}
	memcpy(& session_data.packet1size, p, sizeof(session_data.packet1size));
	session_data.packet1size = ntohl(session_data.packet1size);
	p += sizeof(session_data.packet1size);
	//printf("packet1size = %d\n", session_data.packet1size);
	

	if (p + sizeof(session_data.packet1_iv) > buf_end) // we got a nastygram, illegal bounds
	{
		delete buf;
		return false;
	}
	memcpy(session_data.packet1_iv, p, sizeof(session_data.packet1_iv));
	p += sizeof(session_data.packet1_iv);

	
	if ((p + session_data.packet1size >= buf_end) || session_data.packet1size > MAX_MALLOC_SIZE) // we got a nastygram, illegal bounds
	{
		delete buf;
		cout << "Got a nastygram from server 1 . Invalid bounds\n";
		return false;
	}

	session_data.packet1_data = new u_int8_t[session_data.packet1size];
	if (!session_data.packet1_data)
	{
	   perror("new:");
	   exit(0);
	}

	memcpy(session_data.packet1_data, p, session_data.packet1size);
	p += session_data.packet1size;

	// ---one packet down------
	
	if (p + sizeof(session_data.packet2size) > buf_end) // we got a nastygram, illegal bounds
	{
		delete buf;
		delete session_data.packet1_data;
		return false;
	}
	memcpy(&session_data.packet2size, p, sizeof(session_data.packet2size));
	session_data.packet2size = ntohl(session_data.packet2size);
	p += sizeof(session_data.packet2size);

	
	if (p + sizeof(session_data.packet2_iv) > buf_end) // we got a nastygram, illegal bounds
	{
		delete buf;
		delete session_data.packet1_data;
		return false;
	}
	
	memcpy(session_data.packet2_iv, p, sizeof(session_data.packet2_iv));
	p += sizeof(session_data.packet2_iv);


	
	if (p + session_data.packet2size >= buf_end || (session_data.packet2size > MAX_MALLOC_SIZE)) // we got a nastygram, illegal bounds
	{
		delete buf;
		delete session_data.packet1_data;
		cout << "Got a nastygram from server 2 .  Invalid bounds\n";
		return false;
	}


	session_data.packet2_data = new u_int8_t[session_data.packet2size];


	if (!session_data.packet2_data)
	{
	   perror("new:");
	   exit(0);
	}
	memcpy(session_data.packet2_data, p, session_data.packet2size);
	p += session_data.packet2size;
	// Now for the KeySpaceChunk

	serialized_object serKeySpaceChunk;

	if (p + sizeof(serKeySpaceChunk.size) >= buf_end ) // we got a nastygram, illegal bounds
	{
		delete buf;
		delete session_data.packet1_data;
		delete session_data.packet2_data;
		cout << "Got a nastygram from server 2 .  Invalid bounds\n";
		return false;
	}
	memcpy(&serKeySpaceChunk.size, p, sizeof(serKeySpaceChunk.size));
	serKeySpaceChunk.size = ntohl(serKeySpaceChunk.size);
	p += sizeof(serKeySpaceChunk.size);
	//printf("SerializedKeySpaceChunk.size is %u\n", serKeySpaceChunk.size);
	
	if ( (p + serKeySpaceChunk.size != buf_end) || (serKeySpaceChunk.size > MAX_MALLOC_SIZE) ) // we got a nastygram, illegal bounds
	{
		delete buf;
		delete session_data.packet1_data;
		delete session_data.packet2_data;
		cout << "Got a nastygram from server dealing with serKeySpaceChunk\n";
		printf("p + serKeySpaceChunk.size = %x\n", p + serKeySpaceChunk.size);
		printf("buf_end = %x\n", buf_end);
		printf("serKeySpaceChunk.size = %d\n", serKeySpaceChunk.size);
		printf("MAX_MALLOC_SIZE = %d\n", MAX_MALLOC_SIZE);
		return false;
	}

	serKeySpaceChunk.data = new u_char_t[serKeySpaceChunk.size];
	if (!serKeySpaceChunk.data)
	{
		perror("new:");
		exit(0);
	}
	memcpy(serKeySpaceChunk.data, p, serKeySpaceChunk.size);

	//cout << "All done de-marshalling data!\n";
	delete buf;

	KeySpaceChunk myChunk = unserializeKeySpaceChunk(serKeySpaceChunk);
	delete serKeySpaceChunk.data;
	//cout << "heres our key space chunk\n";
	//cout << myChunk.iKey; 

	session_data.myKeySpaceChunk= new KeySpaceChunk;
	if (!session_data.myKeySpaceChunk)
	{
		cout << "cant allocate memory for KeySpaceChunk. exiting\n";
		perror("new");
		exit(0);
	}	
	*session_data.myKeySpaceChunk = myChunk;

	return true;
}


//Directly called by server. does not need a ProtocolHeader because it generates its own.
bool SendSessionData(int fd,  struct SessionData session_data)
{
	int rc;
	bool ret;
	int bytes_written;
	u_int8_t *data;
	serialized_object serializedKeyChunk;
	u_int32_t h_size; // this is to be used a proxy for h.size because h.size will be in netowrk order
	u_int32_t packet1size; //similliarly for session_data.packet1size
	u_int32_t packet2size;
	u_int32_t ser_key_size; // same idea as above
	
	struct ProtocolHeader h;

	serializedKeyChunk =  session_data.myKeySpaceChunk->serialize();
	ser_key_size = serializedKeyChunk.size;

    h_size =  sizeof(session_data.packet1size) + sizeof(session_data.packet1_iv) + session_data.packet1size +
              sizeof(session_data.packet2size) + sizeof(session_data.packet2_iv) + session_data.packet2size +
              sizeof(serializedKeyChunk.size) + serializedKeyChunk.size;
	h = generate_header(DATA_REPLY);
	h.size = h_size;


	data = new u_int8_t[h_size];
	if (!data)
	{
		perror("new:");
		delete []serializedKeyChunk.data;
		return false;
	}
#ifdef JC_DEBUG
    cout << "Writing a total of " << h_size << " bytes\n";
#endif

	// start flipping bits
	packet1size = session_data.packet1size;
	session_data.packet1size = htonl(session_data.packet1size);
	packet2size = session_data.packet2size;
	session_data.packet2size = htonl(session_data.packet2size );

	serializedKeyChunk.size = htonl(serializedKeyChunk.size);

	u_int8_t *p = data;
	
	 // so the layout for the data is
	 // ---[ProtocolHeader]---
    // network byte order: packet1size 4 bytes
    // neutral byte order: packet1IV    3 bytes
    // neutral byte order: packet1data
    // ------------------
    // network byte order: packet2size 4 bytes
    // neutral byte order: packet2IV   3 bytes
    // neutral byte order: packet2data
    // ------------------
    // network byte order: serializedKeySpaceChunk size
    // neutral byte order: serializedKeySpaceChunk


    memcpy(p, &session_data.packet1size, sizeof(session_data.packet1size));
    p += sizeof(session_data.packet1size);

    memcpy(p, session_data.packet1_iv, sizeof(session_data.packet1_iv));
    p += sizeof(session_data.packet1_iv);

    memcpy(p, session_data.packet1_data, packet1size); //packet1size in correct byte ordering
    p += packet1size;
	
    // thats the first packet down
	
    memcpy(p, &session_data.packet2size, sizeof(session_data.packet2size));
    p += sizeof(session_data.packet2size);

    memcpy(p, session_data.packet2_iv, sizeof(session_data.packet2_iv));
    p += sizeof(session_data.packet2_iv);

    memcpy(p, session_data.packet2_data, packet2size); //packet2size in correct byte ordering
    p += packet2size;
	// 
  // and the second packet
	//finally the serialized key
	//
    memcpy(p, &serializedKeyChunk.size, sizeof(serializedKeyChunk.size));
    p += sizeof(serializedKeyChunk.size);

    memcpy(p, serializedKeyChunk.data, ser_key_size);// ser_key_size in correct byte ordering
	
	ret = write_packet(fd, h, data);
	delete []serializedKeyChunk.data;
	delete []data;
	if (!ret)
	{
		cout << "YIKES. error in libjcwepcrackSendSessionData, only wrote << " << bytes_written << "out of ";
		cout << h.size << " byes\n";
		return false;
	}
	else
		return true;
	
}




//client public interface
bool StartNewSession(char *server, u_int16_t port, struct SessionData &session_data)
{
	int fd = 0;
	int rc = 0;
	bool ret = false;
	int bytes_written = 0;
	struct ProtocolHeader request_data_header;
	

	fd = connect_to_server(server, port);
	if (fd < 0)
		return false;
	
	request_data_header = generate_header(REQUEST_DATA);
	request_data_header.size = 0;

	
	ret = write_packet(fd, request_data_header, NULL);

	if (!ret)
	{
		perror("jcprotocol.cpp::StartNewSession:: couldnt send request for data\n");
		return false;
	}

	// this function handles all the nastyness of bit ordering etc
	ret = ReadSessionData(fd,  session_data);

	
	if ( close(fd) < 0)
	{
		perror("close");
		exit(0);
	}
	return ret;

			
}


bool SubmitFinishedKeySpaceChunk(char *server, u_int16_t port, u_int32_t session_id, Key iKey)
{
	return SubmitFinishedKeyOrKeyChunk(server, port, session_id, iKey, SUBMIT_COMPLETED_CHUNK);		
}
bool SubmitVerifiedKey(char *server, u_int16_t port, u_int32_t session_id, Key iKey)
{
	return SubmitFinishedKeyOrKeyChunk(server, port, session_id, iKey, SUBMIT_VERIFIED_KEY);
}


static bool SubmitFinishedKeyOrKeyChunk(char *server, u_int16_t port, u_int32_t session_id, Key iKey, u_int32_t mode)
{
	int fd;
	int rc;
	bool ret;
	int bytes_written;
	serialized_object serialized_key;
	u_int8_t * data;
	u_int32_t data_size;

	ProtocolHeader submit_keychunk_header;
	submit_keychunk_header = generate_header(mode);
	
	serialized_key = iKey.serialize();

	data_size= + sizeof(session_id) + sizeof(serialized_key.size) + serialized_key.size;

	data = new u_char_t[data_size];
	if (!data)
	{
		cout << "cant allocate memory. exiting. \n";
		perror("new");
		exit(0);
	}
	
	submit_keychunk_header.size = data_size;


	unsigned char *p = data;
	
	//the format of this is
	//network byte order  u_int32_t session_id 4 bytes 
	//network byte order  u_int32_Tserialized_key size 4 bytes 
	//neutral byte order  serialized key data 
	
	session_id = htonl(session_id);
	memcpy(p, &session_id, sizeof(session_id));
	p += sizeof(session_id);

	u_int32_t ser_key_size;
	ser_key_size = htonl(serialized_key.size);
	memcpy(p, &ser_key_size, sizeof(ser_key_size));
	p += sizeof(ser_key_size);

	memcpy(p, serialized_key.data, serialized_key.size);
	

	
	fd = connect_to_server(server, port);
	if (fd < 0)
		return false;

	ret = write_packet(fd, submit_keychunk_header, data);
	close(fd);
	delete data;
	if (!ret)
	{
		perror("couldnt write all the bytes\n");
		exit(0);	 // handle this more gracefully later
	}
	return true;
	
}

static bool write_packet(int fd, struct ProtocolHeader h, u_int8_t *data)
{

	int rc;
	int bytes_sent;
	u_int8_t *buf;
	int buf_size = sizeof(h) + h.size;
	u_int32_t h_size;
	buf = new u_int8_t[buf_size];

	if (!buf)
		return false;
	
	// start flipping bits
	h.flags = htonl(h.flags);
	h.major = htons(h.major);	
	h.minor = htons(h.minor);	
	h.RequestType = htonl(h.RequestType);
	h_size = h.size; // we need this in local byte form.
	h.size = htonl(h_size);
	// encrypting a 0 sized packet may be tricky, dont forget about it

		
	// This is where application level encryption stuff would go
	u_int8_t *p;
	p = buf;

	memcpy(p, (void *) &h, sizeof(h));	
	p += sizeof(h);

	if (h_size != 0)
		memcpy(p, data, h_size);

	rc = send_all(fd, (char *) buf, buf_size, &bytes_sent);
	if (rc <  0)
	{
		cout << "Write Packet::Couldnt send_all bytes\n";
		cout << "Bytes sent: " << bytes_sent << endl;
		cout << "buf_size : " << buf_size << endl;
		delete buf;
		return false;
	}
	
	delete buf;
	return true;
	
}

//if this returns true callee must delete data
//if it returns false it handles it itself
//if the header contains 0 bytes callee should not free the data

bool read_packet(int fd, struct ProtocolHeader &h, u_int8_t ** to_data)
{
	int rc;
	bool ret;

	rc = recv(fd, (char *) &h, sizeof(h), 0);
	if (rc != sizeof(h))
	{
		cout << "couldnt read the reply header entirely. exiting\n";
		cout << "read " << rc << " bytes\n";
		cout << "was supposed to read " << sizeof(h) << "bytes\n";
		return false;
	}
	else
	{
#ifdef JC_DEBUG
		cout << "Successfuly read in header at " << sizeof(h) << " bytes\n";
#endif
	}

	h.flags = ntohl(h.flags);
	h.major = ntohs(h.major);	
	h.minor = ntohs(h.minor);	
	h.RequestType = ntohl(h.RequestType);
	h.size = ntohl(h.size);
	

	if (h.size == 0)
	{
		return true;
	}


	char tempstr[2048];
	if (h.size > MAX_MALLOC_SIZE)
	{
		snprintf(tempstr, sizeof(tempstr), "Fishy packet, data = %u, MAX_MALLOC_SIZE = %u\nDropping Connection\n", h.size, MAX_MALLOC_SIZE);
		cout << tempstr;
		cout << "Edit libjcwepcrack/config.h to change MAX_MALLOC_SIZE if you think this is legit traffic\n";
		return false;
	}

	*to_data = new u_int8_t[h.size];
	if (!to_data)
	{
		perror("read_packet::new\n");
		return false;
	}

	rc = recv(fd, *to_data, h.size, 0);
	if (rc != h.size)
	{
		perror("read_packet::recv");
		cout << "Only read " << rc << "bytes\n";
		delete *to_data;
		return false;
	}

	return true;
	
}
		

//public server interface
bool ParseFinishedKeyOrKeyChunk(struct ProtocolHeader h, u_int8_t *data,  u_int32_t &session_id, Key &iKey)
{
    int rc;
    //bool ret;
    serialized_object ser_key;

    u_int8_t *p = data;
	u_int8_t *buf_end = data + h.size;
	if (buf_end < data)
		return false;
    // the format of this data is
    //network byte order u_int32_t session_id   4 bytes
    //network byte order u_int32_t ser_key_size 4 bytes
    //neutral byte order serialized_key
    p = data;


	if (p + sizeof(session_id) > buf_end)
	{
		printf("Malicious key submission attempt. invalid packet\n");
		return false;
	}
    memcpy(&session_id, p, sizeof(session_id));
    session_id = ntohl(session_id);
    p += sizeof(session_id);

	if ( p + sizeof(ser_key.size) > buf_end)
	{
		printf("Malicious key submission attempt. invalid packet\n");
		return false;
	}
    memcpy(&ser_key.size, p, sizeof(ser_key.size));
    ser_key.size = ntohl(ser_key.size);
    p += sizeof(ser_key.size);

    if (ser_key.size > MAX_MALLOC_SIZE)
    {
        //!
        cout << "Fishy Key submission request. Says serialized key is " << ser_key.size << "bytes\n";
        cout << "Dropping it. Could be attemped dos.\n";
        return false;
    }
    ser_key.data = new u_int8_t[ser_key.size];
    if (!ser_key.data)
    {
        perror("new:");
        return false;
    }
	memset(ser_key.data, 0, ser_key.size);


	if ( p + ser_key.size != buf_end)
	{
		printf("Malicious key submission attempt. invalid packet\n");
		delete ser_key.data;
		return false;
	}
    memcpy(ser_key.data, p, ser_key.size);
    iKey = unserializeKey(ser_key);
#ifdef JC_DEBUG
	cout << "ReadFinishedsomething:: iKey " << iKey << endl;
#endif

    delete ser_key.data;

	return true;
}


static int send_all(int s, char *buf, int len, int *length_written)
{
	int total = 0;        // how many bytes we've sent
	int bytesleft = len; // how many we have left to send
	int n = 0;

#ifdef JC_DEBUG
	printf("attempting to send %d bytes\n", len);
#endif
	while(total < len) 
	{
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1) 
		{break;} 
//		printf("wrote %d bytes\n", n);
		
		total += n;
		bytesleft -= n;
	}

*length_written = total; // return number actually sent here

return n==-1?-1:0; // return -1 on failure, 0 on success
} 

void print_header(struct ProtocolHeader &h)
{
	printf(" flags is %x\n", h.flags);
	printf("Major version is %x\n", h.major);
	printf("minor version is %x\n", h.minor);
	printf("requestype is is %x\n", h.RequestType);
	printf("size is  %x\n", h.size);
}

void print_SessionData(SessionData &data)
{
	printf("-----------------------\n");
	printf("packet1size: %d\n", data.packet1size);
	printf("packet1_iv: ");
	print_hex(data.packet1_iv, 3);
	printf("\npacket1_data:\n");
	print_hex(data.packet1_data, data.packet1size);
	printf("\n----------------------\n");
	printf("KeyspaceChunk.size: %d\n", data.myKeySpaceChunk->iKey.getSize());
	printf("KeyspaceChunk.iKey: %s\n", data.myKeySpaceChunk->iKey.getString().c_str());
	printf("----------------------\n");

	

}

u_int32_t generate_flags()
{
	return 0; //more here later
}

