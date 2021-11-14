/*
 *  Misc.cpp
 *  jc-wepcrack
 *
 *  Created by johnny cache on Sun Jul 11 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "jcwepcrack.h"
#include "datatypes.h"
#include "crypt.h"
#include "pcap_packet.h"
#include "packet80211-data.h"
#include <pcap.h>
#include <stdlib.h>
#include <map.h>
//extern Packet_80211_data *GlobalPacketToCrack;


//This functions trusts that the iv/keystream match up. It is broken up like this to support
//dictionary lookups.
Packet_80211_data* EncryptPacketWithKeystream(u_int8_t iv[3], u_int8_t key_id, u_int8_t * key_stream,  u_int32_t stream_size, Packet_80211_data &p)
{

	// This is way easier. First. Compute the ICV over the data
	//thne make anew bigg buffer to put both in
	//[IV][KEYID][DATA][ICV]

	printf("#1!\n");

	struct pcap_pkthdr old_pcap_hdr;
	struct pcap_pkthdr new_pcap_hdr;

	struct DLT_IEEE802_11_frame old_wlan_header;
	struct DLT_IEEE802_11_frame new_wlan_header;

	//u_int8_t *data;

	u_int32_t ICV;
	u_int32_t data_size = p.getPDUDataLength();
	if (stream_size < data_size + 4)
	{
		perror("STREAM TO SMALL FOR PACKET\n");
		return NULL;
	}



	old_pcap_hdr = p.get_pcap_hdr();
	new_pcap_hdr = p.get_pcap_hdr();
	new_pcap_hdr.len += 8;
	new_pcap_hdr.caplen +=8;

	p.getWlanHeader(&old_wlan_header);
	new_wlan_header = old_wlan_header;

	u_int16_t frame_ctrl; //first 2 bytes of a 80211 packet

	memcpy(&frame_ctrl, &old_wlan_header, sizeof(frame_ctrl));
	u_int16_t wep_on, wep_off;
	wep_on = 0x0040;

	#ifdef LITTLE_ENDIAN
	wep_on = htons(wep_on);
	#endif
	
	if (frame_ctrl & wep_on)
	{
	   printf("Error! trying to Encrypt a WEP'd up packet.\n");
	   printf("misc.cpp::EncryptePacketWithKeystream\n");
	   exit(0);
	}
	else
		frame_ctrl = frame_ctrl | wep_on;

	memcpy(&new_wlan_header, &frame_ctrl, sizeof(frame_ctrl));
	
	//Ok. so far we have a new pcap header and a new wlan header. we now need a ICV to append and then 
	//get that data all snug in a buffer.
	//ICV = bufcrc(data, data_size);
	// Thats it! lets put it all together.

	printf("sizeof(new_wlan_header) is %d\n", sizeof(new_wlan_header));
	printf("sizeof iv is %d\n", sizeof(iv));
	printf("sizeof key_id is %d\n", sizeof(key_id));
	printf("sizeof icv is %d\n", sizeof(ICV));
	
	int mem_too_alloc = sizeof(new_wlan_header) +4 + data_size + 4;

	u_int8_t * buff = new u_int8_t[mem_too_alloc];
	u_int8_t *c;
	if (!buff)
	{
		perror("new");
		exit(0);
	}
	c = buff;
	memcpy(c, &new_wlan_header, sizeof(new_wlan_header));
	c+= sizeof(new_wlan_header);
	printf("0\n");

	memcpy(c, iv, 3);
	c += 3;
	printf("1\n");

	memcpy(c , &key_id, 1);
	c += 1;
	printf("2\n");

	p.getPDUData(c);
	printf("3\n");

	ICV = bufcrc(c, data_size);
	c += data_size;

	memcpy(c, (void *) &ICV, sizeof(ICV));
	c += sizeof(ICV);
	printf("4\n");
	
	printf("c - buff is %d\n", c - buff);
	printf("mem too alloc is %d\n", mem_too_alloc);
	c = buff + sizeof(new_wlan_header) + 4; // back at the data;

	for (int i = 0; i < data_size + 4; i++)
	{
		//c[i]
		c[i] = c[i] ^ key_stream[i];
		//*(c + i) = *(c + i) ^ *(key_stream + i);

	}

	Packet_80211_data *Encrypted_Packet;
	Encrypted_Packet = new Packet_80211_data(new_pcap_hdr, buff);
	printf("6\n");
	Encrypted_Packet->print();
	printf("7\n");
	delete buff;
	return Encrypted_Packet;
}

Packet_80211_data * BuildDecrypted80211_Packet(struct pcap_pkthdr old_pcap_hdr, struct DLT_IEEE802_11_frame old_wlan_hdr, u_int8_t *data, u_int32_t data_size)
{
	printf("-----------------\n");
	printf("old_pcap_hdr.len = %d\n", old_pcap_hdr.len);
	printf("sizeof wlan_header = %d\n", sizeof(struct DLT_IEEE802_11_frame));
	printf("data_size = %d\n\n", data_size);

	struct pcap_pkthdr new_pcap_hdr;
	struct DLT_IEEE802_11_frame new_wlan_hdr;

	new_pcap_hdr = old_pcap_hdr;
	new_pcap_hdr.len -= 8;
	new_pcap_hdr.caplen -=8; //XXX is the definetly the right thing to do?
	printf("new_pcap_jdr.len  = %d\n");
	new_wlan_hdr = old_wlan_hdr;

	u_int16_t frame_ctrl; //first two bytes off any 80211 packet

	memcpy(&frame_ctrl, &old_wlan_hdr, sizeof(frame_ctrl));
	printf("before: %x\n", frame_ctrl);
	u_int16_t flag, off_flag;
	flag = 0x0040;
	off_flag =0xFFBF;

	#ifdef LITTLE_ENDIAN
	printf("LITTLE ENDIAN\n");
	flag = htons(flag);
	off_flag =htons(off_flag);
	#endif

	if(frame_ctrl & flag)
	{
		printf("Turning the WEP bit off.\n");

		printf ("flag is %x\n", flag);
		frame_ctrl = frame_ctrl & (off_flag);

	}
	else
	{
		printf("Sanity check failed! this packets frame control doesnt ahve wep?\n");
		exit(0);
	}
	printf("after: %x\n", frame_ctrl);
	memcpy(&new_wlan_hdr, &frame_ctrl, sizeof(frame_ctrl));

	printf("sizeof wlan_hdr = %d\n", sizeof(new_wlan_hdr));
	printf ("data_size = %d\n", data_size);
	printf ("wlan_hdr + data_size  = %d\n", sizeof(new_wlan_hdr) +data_size);
	printf ("Does that eq new_pcap_hdr.len? %d\n", new_pcap_hdr.len);

	u_int8_t  *big_buff = new u_int8_t[sizeof(new_wlan_hdr) + data_size];
	if (! big_buff)
	{
		perror("new");
		exit(0);
	}
	memcpy(big_buff, &new_wlan_hdr, sizeof(new_wlan_hdr));
	memcpy(big_buff  + sizeof(new_wlan_hdr), data, data_size);

	
	Packet_80211_data *Test = new Packet_80211_data(new_pcap_hdr, big_buff);
	//Test->print();

	delete big_buff;
	return Test;

}



//Returns a new Packet80211 if it works, null otherwise
Packet_80211_data * CrackPacketWithKeystream(u_int8_t * keystream, u_int32_t stream_size, Packet_80211_data &p)
{
	printf("Attempting to decrypt packet:\n");
	p.print();
	
	u_int8_t *decrypted_icv; // decrypted checksum, points into buffer
	u_int32_t decrypted_icv_crc;
	u_int32_t computed_crc;
	
	u_int8_t * decrypted_data;
	u_int8_t *encrypted_data;
	
	u_int32_t size = stream_size;

	printf("stream_size is %d  p.pcap_hdr.len = %d p.getDataLength is %d\n", stream_size, 
			p.get_pcap_hdr().len, p.getPDUDataLength());
	if (p.getPDUDataLength() != stream_size)
	{
		printf("Size mismatch. if stream_size < p.getDataLength, we have issues\n");
		if (stream_size < p.getPDUDataLength())
		{
			printf("we have issues\n");
			exit(0);
		}
		printf("Its OK. we have a bigger keystream than packet.\n");
		size = p.getPDUDataLength();
		
	}
	printf("1\n");
	encrypted_data = new u_int8_t[size];
	if (!encrypted_data)
	{
		perror("new");
		exit(0);
	}
	if (!p.wep())
	{
	  printf("Attempting to decrypt plaintext packet!\n");
	  exit(0);
	}

	printf("2\n");
	p.getPDUData(encrypted_data);
	if (!encrypted_data)
	{
	   perror("Coudlnt get packets data..\n");
	}
	
	printf("3\n");

	//printf("Got encrypted data:\n");
	//print_hex(encrypted_data, size);
	
	//printf("XOR KEYSTEAM\n");
	//print_hex(keystream, size);

	//printf("trying to allocate %d bytes.\n", size);
	decrypted_data = new u_int8_t[size];

	if (!decrypted_data)
	{
		perror("new");
		exit(0);
	}
	//printf("allocated room for decrypted  data.\n");
	//printf("Decrypting..\n");

	// lets get to it!
	for (int i = 0; i < size; i++)
	{
		decrypted_data[i] = encrypted_data[i] ^ keystream[i];
	}
	print_hex(decrypted_data, size);

	//printf("Ganking decrypted icv\n");
	decrypted_icv = decrypted_data + size - 4;
	decrypted_icv_crc = decrypted_icv[0] | (decrypted_icv[1] << 8)
			| (decrypted_icv[2] << 16) | (decrypted_icv[3] << 24);


	//  printf("Decrypted icv: ");
  	//print_hex(decrypted_icv, 4);
	
	//printf("crc of Decrypted icv: %x\n", decrypted_icv_crc);
	//printf("\n");


	computed_crc = bufcrc(decrypted_data, size - 4);
	//printf("computed_crc %x\n", computed_crc);
	
	
	Packet_80211_data *Decrypted_Packet = NULL;
	if (computed_crc == decrypted_icv_crc)
	{
		printf("Match!\n");
		struct DLT_IEEE802_11_frame old_wlan_header;
		p.getWlanHeader(&old_wlan_header);
		Decrypted_Packet = BuildDecrypted80211_Packet(p.get_pcap_hdr(), old_wlan_header, 
				   decrypted_data, size - 4); //dont pass the ICV at the end.
		printf("Just got packet back from builder..\n");
		Decrypted_Packet->print();
	}

	delete decrypted_data;
	delete encrypted_data;

	return Decrypted_Packet;
}

