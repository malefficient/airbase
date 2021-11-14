#include "airware-radiotap.h"
bool radiotap_header::init(u_int8_t *rtap_hdr)
{
		initialized = false;
		//printf("radiotap_header::init\n");
		
		struct ieee80211_radiotap_header *local_rtap_ptr;
		local_rtap_ptr = (struct ieee80211_radiotap_header *) rtap_hdr;
		//printf("local_rtap_ptr->it_version = %d\n", local_rtap_ptr->it_version);
		//printf("local_rtap_ptr->it_len = %d\n", local_rtap_ptr->it_len);
		//printf("local_rtap_ptr->it_present = 0x%4X\n", local_rtap_ptr->it_present);
		if (local_rtap_ptr->it_len < MAX_RTAP_SIZE)
		{
			memcpy(radiotap_data, rtap_hdr, local_rtap_ptr->it_len);
			rtap_ptr = (struct ieee80211_radiotap_header*) radiotap_data;
		}
		else
		{
			printf("radiotap_header::init::error. it-len  (%d) > MAX_RTAP_SIZE (%d)\n", rtap_ptr->it_len, MAX_RTAP_SIZE);
			exit(0); //XXX: take me out later
			return false;
		}
		//its now safe to quit using local_rtap_ptr
		if (rtap_ptr->it_version != 0)
		{
			printf("Error. invliad radiotap it_version: %d\n", rtap_ptr->it_version);
			return false;
		}
		if (rtap_ptr->it_present & (1 << IEEE80211_RADIOTAP_EXT))
		{
			printf("Error. current parser doesnt support extended bitmaps.\n");
			return false;
		}
		//iterate over all the fields, saving the offsets to each.
		int offset_to_curr_field = sizeof(struct ieee80211_radiotap_header);
		for (int i = 0; i <  MAX_RTAP_DEFINED_FIELDS; i++)
		{
			if (is_present(i))
        	{
				 
				offset_to_field[i] = offset_to_curr_field;
           		 //printf("###%d %s \t size=%d offset=%d\n", i, nameof_field(i),sizeof_field(i), offset_to_field[i]);
				offset_to_curr_field += sizeof_field(i);
        	}
			else
				offset_to_field[i] = -1;
		}

		initialized = true;
		return true;
}
bool radiotap_header :: is_present(u_int32_t field_number)
{
	if (rtap_ptr->it_present & (1 << field_number))
		{
			return  true;
		}
	return false;
}
u_int32_t radiotap_header :: retrieve_value(u_int32_t field_number, u_int8_t *dst)
{
	if (! is_present(field_number))
		return 0; 
	if (sizeof_field(field_number) <= 0)
	{
		printf("radiotap_header::retrieve_value sanity check failed!\n");
		exit(0);
	}
	memcpy(dst, &radiotap_data[ offset_to_field[field_number]], sizeof_field(field_number));
	return sizeof_field(field_number);
}






