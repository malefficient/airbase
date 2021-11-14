#include "wep-crypt.h"

bool WepEncrypt(Packet_80211_data &PlaintextPacket, Packet_80211_data &EncryptedPacket,
		        struct WEP_IV_struct IV, u_int8_t *key, u_int8_t keysize)
{

	if (keysize  + 3 > MAX_WEPKEYSIZE)
	{
		printf("Error.  wep key to big. change include/wep_key.h:#MAX_WEPKEYSIZE\n");
		exit(0);
	}
	u_int8_t real_key[MAX_WEPKEYSIZE];
	memcpy(real_key, (void *) &IV, 3); //skip key #
	memcpy(real_key + 3, key, keysize);

	return WepEncrypt(PlaintextPacket, EncryptedPacket, real_key, keysize + 3);
}


bool WepEncrypt(Packet_80211_data &PlaintextPacket, Packet_80211_data &EncryptedPacket,
		        u_int8_t *key, u_int8_t keysize, u_int8_t keynum)
{
		
	bool ret;

	if (PlaintextPacket.length > MAX_PCAP_DATA_SIZE - 8)
	{
		printf("Error. WepEncrypt Cannot encrypt packet. it is to big.\n");
		return false;
	}
	
	u_int32_t plaintext_size = PlaintextPacket.getPDUDataLength();
	if (plaintext_size + 4 > MAX_PCAP_DATA_SIZE)
	{
		printf("Error in WepEncrypt. Increase MAX_PCAP_DATA_SIZE so i can append an icv.\n");
		exit(0);
	}

	u_char_t plaintext_data[MAX_PCAP_DATA_SIZE];
	u_char_t encrypted_data[MAX_PCAP_DATA_SIZE];

	PlaintextPacket.getPDUData(plaintext_data);
	ret = WepEncryptData(plaintext_data, encrypted_data, plaintext_size,  key , keysize);

	if (ret == false)
	{
		printf("Error. WepEncryptData().. Failed, called from WepEncryptPacket..\n");
		return false;
	}
	EncryptedPacket.Init(PlaintextPacket);
	
	//enable wep.
	EncryptedPacket.set_wep();
	//add the wep info to packet.
	memcpy( (void *) EncryptedPacket.payload_ptr, key, 3);
	memcpy( (void *) (EncryptedPacket.payload_ptr + 3), &keynum, 1);
	//scoot payload ptr up past wep info.
	EncryptedPacket.payload_ptr += 4;	
	memcpy(EncryptedPacket.payload_ptr, encrypted_data, plaintext_size + 4); // keep appended icv attached.
	EncryptedPacket.length += 8; // 4 for the wep flags, 4 for icv.
	EncryptedPacket.tot_length += 8; // 4 for the wep flags, 4 for icv.
		
	return true;
}

//
bool WepEncryptData(u_int8_t *plaintext_data, u_int8_t *encrypted_data, u_int32_t plaintext_size, u_int8_t *iv, u_int8_t *key, u_int32_t key_size)
{
	u_int8_t real_key[MAX_WEPKEYSIZE];

	if (key_size + 3 > MAX_WEPKEYSIZE)
	{
		printf("WepEncryptData(with iv): Keysize to big. (%d + 3), > MAX_WEPKEYSIZE(%d)\n", key_size, MAX_WEPKEYSIZE);
		exit(0);
	}
	memcpy(real_key, iv, 3);
	memcpy(real_key + 3, key, key_size);
	return WepEncryptData(plaintext_data, encrypted_data, plaintext_size, real_key, key_size + 3);
}

//The outputted encrypted data will be 4 bytes larger with the encrypted icv appended.
bool WepEncryptData(u_int8_t *plaintext_data, u_int8_t *encrypted_data, u_int32_t plaintext_size,  u_int8_t *key, u_int32_t key_size)
{
	RC4_KEY rc4key;
	u_int32_t icv; //checksum

	RC4_set_key(&rc4key, key_size, key); 
	icv = bufcrc(plaintext_data, plaintext_size);

	RC4(&rc4key, plaintext_size , plaintext_data, encrypted_data); 
	RC4(&rc4key, sizeof(icv), (u_int8_t*) &icv, encrypted_data + plaintext_size); 

	return true;

}

//Decrypt straight into Decrypted packets memory with a minimum of asnity checking.
bool UNSAFE_WepDecrypt(Packet_80211_data &EncryptedPacket, Packet_80211_data &DecryptedPacket,
				                u_int8_t *key, u_int8_t keysize)
{
	printf("--UNSAFE_WepDecrypt--start\n");
	if (keysize  + 3 > MAX_WEPKEYSIZE)
	{
		printf("Error.  wep key to big. change include/wep_key.h:#MAX_WEPKEYSIZE\n");
		exit(0);
	}

	u_int8_t *encrypted_data = EncryptedPacket.data + sizeof(struct ieee80211) + 4;
	u_int8_t *plaintext_data =  DecryptedPacket.data + sizeof(struct ieee80211);
	if (EncryptedPacket.qos())
	{
		printf("UNSAFE detected QoS..\n");
		encrypted_data += sizeof (struct ieee80211QoS);
		plaintext_data += sizeof(struct ieee80211QoS);
	}

	u_int32_t ciphertext_size = EncryptedPacket.getPDUDataLength();
	u_int32_t plaintext_size = ciphertext_size - 4;


	u_int8_t IV[3];
	EncryptedPacket.getWepIV(IV);
	printf("IV:");
	print_hex(IV, 3);
	printf("\n");
	bool success_decr;
	success_decr = WepDecryptData(encrypted_data, plaintext_data, ciphertext_size, IV, key, keysize);
	if (!success_decr)
	{
		printf("Failed.\n"); 
		DecryptedPacket.initialized = false;
		return false;
	}
	else
	{
		//printf("Sucess!\n");
		//--XXX: This code will BREAK if pcap_packets init routines changed.
		DecryptedPacket.dlt = EncryptedPacket.dlt;  //these three fields take care of the pcap header
		DecryptedPacket.ts =  EncryptedPacket.ts;
		DecryptedPacket.tot_length = EncryptedPacket.tot_length;

		DecryptedPacket.length = EncryptedPacket.length;
		memcpy(DecryptedPacket.data, EncryptedPacket.data, sizeof(struct ieee80211));
		DecryptedPacket.wlan_header = (struct ieee80211 *) DecryptedPacket.data;

		if (EncryptedPacket.qos()) //watch this code mimc Packet_80211::init's handling of QoS
		{

			//printf("Unsafe WEP decrypt detected QoS on the output.\n");
			DecryptedPacket.qos_header=  (struct ieee80211QoS *) DecryptedPacket.data + sizeof(struct ieee80211);
			DecryptedPacket.payload_ptr = (u_int8_t*)  DecryptedPacket.qos_header + sizeof(struct ieee80211QoS);
		}
		else
		{
			//printf("Unsafe WEP didnt detect QoS\n");
			DecryptedPacket.qos_header = NULL;
			DecryptedPacket.payload_ptr = DecryptedPacket.data + sizeof(struct ieee80211);
		}
			
		DecryptedPacket.length -= 8;
		DecryptedPacket.tot_length -= 8;
		DecryptedPacket.clear_wep();
		DecryptedPacket.initialized = true;
		return true;
	}
}


bool WepDecrypt(Packet_80211_data &EncryptedPacket, Packet_80211_data &DecryptedPacket,
		                u_int8_t *key, u_int8_t keysize)
{
	if (keysize  + 3 > MAX_WEPKEYSIZE)
	{
		printf("Error.  wep key to big. change include/wep_key.h:#MAX_WEPKEYSIZE\n");
		exit(0);
	}
	//printf("--WepDecrypt Top--\n");
	u_char_t encrypted_data[MAX_PCAP_DATA_SIZE];
	u_char_t plaintext_data[MAX_PCAP_DATA_SIZE];

	u_int32_t ciphertext_size = EncryptedPacket.getPDUDataLength();
	u_int32_t plaintext_size = ciphertext_size - 4;

	if (ciphertext_size  > MAX_PCAP_DATA_SIZE )
	{
		printf("WepDecrypt::Error. Passed in packet.getPduDataLenth(%d) > MAX_PCAP_DATA_SIZE (%d)\n", 
				ciphertext_size, MAX_PCAP_DATA_SIZE);
		exit(0);
	}
	
	EncryptedPacket.getPDUData(encrypted_data);


	u_int8_t IV[3];
	EncryptedPacket.getWepIV(IV);
	bool success_decr;
	success_decr = WepDecryptData(encrypted_data, plaintext_data, ciphertext_size, IV, key, keysize);
			  
	if (!success_decr)
	{
		DecryptedPacket.initialized = false;
		return false;
	}
	else
	{
		//printf("--WepDecrypt--\n");
		DecryptedPacket.Init(EncryptedPacket);	
		//printf("Stage one--:\n%s\n", DecryptedPacket.toString().c_str());
		DecryptedPacket.clear_wep();
		DecryptedPacket.length -= 8;
		DecryptedPacket.tot_length -= 8;
		DecryptedPacket.payload_ptr -= 4;
		memcpy(DecryptedPacket.payload_ptr, plaintext_data, plaintext_size);
		//printf("Stage two--:\n%s\n", DecryptedPacket.toString().c_str());
		return true;
	}
}

bool WepDecryptData(u_int8_t *encrypted_data, u_int8_t *decrypted_data, u_int32_t encr_size, u_int8_t *iv, u_int8_t *key, u_int32_t key_size)
{
	u_int8_t real_key[MAX_WEPKEYSIZE];
	if (key_size + 3 > MAX_WEPKEYSIZE)
	{
		printf("WepDecryptData(with iv)::Error, key_size passed to big.\n");
		printf(" key_size (%d) + 3 > MAX_WEPKEYSIZE\n", key_size, MAX_WEPKEYSIZE);
		exit(0);
	}
	memcpy(real_key, iv, 3);
	memcpy(real_key + 3, key, key_size);
	return WepDecryptData(encrypted_data, decrypted_data, encr_size, real_key, key_size + 3);
}




bool WepDecryptData(u_int8_t *encrypted_data, u_int8_t *decrypted_data, u_int32_t encr_size, u_int8_t *key, u_int32_t key_size)
{
    RC4_KEY rc4key;
    u_int8_t *decrypted_icv_ptr; // decrpyted checksum
    u_int32_t decrypted_icv;
    u_int32_t computed_crc;
	if (key_size > MAX_WEPKEYSIZE)
	{
		printf("DecryptWepData::Error, key_size passed to big.\n");
		exit(0);
	}


	//printf("WepDecryptData::key\n");
	//print_hex(key, key_size);
	//printf("\n");

    RC4_set_key(&rc4key, key_size , key);
    RC4(&rc4key, encr_size, encrypted_data, decrypted_data);

    decrypted_icv_ptr = decrypted_data +encr_size - 4; //last 4 bytes.

  	memcpy(&decrypted_icv, decrypted_icv_ptr, 4);
   computed_crc = bufcrc(decrypted_data, encr_size - 4); // dont forget to not crc the ivc at the end

/*
	printf("decrypted icv:\n");
	print_hex((u_int8_t *)&decrypted_icv, 4);
	printf("\n");
	printf("computed crc\n");
	print_hex((u_int8_t *) &computed_crc, 4);
	printf("\n");
*/
		
    if (memcmp(&computed_crc, &decrypted_icv, 4) == 0)
            return true;
	else
    	return  false;
}

bool OldWepDecryptData(u_int8_t *encrypted_data, u_int8_t *decrypted_data, u_int32_t encr_size, u_int8_t *key, u_int32_t key_size)
{
    RC4_KEY rc4key;
    u_int8_t *decrypted_icv_ptr; // decrpyted checksum
    u_int32_t decrypted_icv;
    u_int32_t computed_crc;
	if (key_size > MAX_WEPKEYSIZE)
	{
		printf("DecryptWepData::Error, key_size passed to big.\n");
		exit(0);
	}


    RC4_set_key(&rc4key, key_size , key);
    RC4(&rc4key, encr_size, encrypted_data, decrypted_data);

    decrypted_icv_ptr = decrypted_data +encr_size - 4; //last 4 bytes.

#if BYTE_ORDER == LITTLE_ENDIAN
    decrypted_icv = decrypted_icv_ptr[0] | (decrypted_icv_ptr[1] << 8) 
                       | (decrypted_icv_ptr[2] << 16) | (decrypted_icv_ptr[3] << 24);
#elif BYTE_ORDER == BIG_ENDIAN
  memcpy(&decrypted_icv, decrypted_icv_ptr, 4);
#else
#error "Fix endian/bits.h"
#endif
    computed_crc = bufcrc(decrypted_data, encr_size - 4); // dont forget to not crc the ivc at the end

    if (computed_crc == decrypted_icv)
            return true;
	else
    	return  false;
}

//Returns the checksum in the *correct* byte order.
//This MAY NOT BE YOUR NATIVE ENDIANESS. Do not treat
//the 4 bytes conveintly returned as a int as a int!
//use memcpy, not =
u_int32_t bufcrc(u_int8_t *buf, int size)
{
        //XXX: used to be u_int, how did that work?
        register u_int32_t val;
        val = ~0;
        while(size--)
        {
                val = crc_table[(val ^ *buf) & 0xff] ^ (val >> 8);
                buf++;
        }
#if BYTE_ORDER == LITTLE_ENDIAN
        return ~val;
#elif BYTE_ORDER == BIG_ENDIAN
		return SWAP4(~val);
#else
#error "Fix bit/endian.h"
#endif


}

