#ifndef _PROTOCOL_H
#define _PROTOCOL_H
#include "airware.h"
#include "wepcrack_datatypes.h"
#include "keyspacechunk.h"
#define MAJOR_PROTOCOL_VERSION 2
#define MINOR_PROTOCOL_VERSION 0
#define MAXDATASIZE 8192 // largest amount of data client or server will ever send
struct ProtocolHeader
{
	u_int32_t flags;
	u_int16_t major;
	u_int16_t minor;
	//u_int8_t  IV[3];
	u_int32_t RequestType;
	u_int32_t size; //not total size, size following this point
};
void print_header(struct ProtocolHeader &h);
#define REQUEST_DATA 0x00000001 
#define DATA_REPLY   0X00010000 //reply from server

#define SUBMIT_COMPLETED_CHUNK   0x00000002 
#define GOT_SUBMITED_CHUNK 	     0X00020000

#define SUBMIT_VERIFIED_KEY      0x0000003
#define GOT_VERIFIED_KEY         0X0003000




	
// This is what the client gets out
struct SessionData
{
	u_int32_t packet1size;
	u_int8_t  packet1_iv[3];
	u_int8_t *packet1_data;

	u_int32_t packet2size;
	u_int8_t  packet2_iv[3];
	u_int8_t *packet2_data;

	KeySpaceChunk *myKeySpaceChunk;

};

//TODO: remove read/send header. just confuses things.
//bool read_header(int fd, struct ProtocolHeader &h);
//bool send_header(int fd, struct ProtocolHeader h);
//-------------------Public Client interface:------------------
bool StartNewSession(char *server, u_int16_t port, SessionData &data);
bool SubmitFinishedKeySpaceChunk(char *server, u_int16_t port, u_int32_t session_id, Key iKey);
bool SubmitVerifiedKey(char *server, u_int16_t port, u_int32_t session_id, Key iKey);
//-------------------------------------------------------------
//private, called by above
//if this returns false, no data must be freed by the caller
//if this returns true, all the dynamically allocated memory must be freed by caller
static bool ReadSessionData(int fd,  struct SessionData &data);
static bool SubmitFinishedKeyOrKeyChunk(char *server, u_int16_t port, u_int32_t session_id, Key iKey, u_int32_t mode);

//-----------------Public Server Interface-------------
//unfortunately read_packet needs to be exposed so that the server can read a whole packet at once.
//the server however should consider the data to be opaque and use the provided functions to manipulate
//it.
bool read_packet(int fd, struct ProtocolHeader &h, u_int8_t **data);
bool ParseFinishedKeyOrKeyChunk(struct ProtocolHeader h, u_int8_t *data,  u_int32_t &session_id, Key &iKey);
bool SendSessionData(int fd,  struct SessionData data);
//-----------------------------------------------------


void print_SessionData(SessionData &);

//private, called internally by everything
static bool write_packet(int fd, struct ProtocolHeader h, u_int8_t *data);

//internal use only:
static  u_int32_t generate_flags();
static int connect_to_server(char *name, u_int16_t port);
static struct ProtocolHeader generate_header(u_int32_t type); 
static int send_all(int s, char *buf, int len, int *length_written);


#endif

