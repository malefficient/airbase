/*
 *  Misc.cpp
 *  jc-wepcrack
 *
 *  Created by johnny cache on Sun Jul 11 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "datatypes.h"
#include "print.h"
#include <stdlib.h>
void print_hex(u_char_t *start, u_int32_t len)
{
	u_int32_t cnt = 0;
	int char_cnt = 0;
	int grp_cnt = 0;
	while (cnt < len)
	{
		printf("%2.2x ", *(start + cnt));
		grp_cnt++;
		char_cnt++;
		cnt++;
		if (grp_cnt == 8) 
		{
			printf(" : ");
			grp_cnt = 0;
		}
			if (char_cnt == 32)
		{
			printf("\n");
			char_cnt = 0;
		}
	}
	printf("\n");
}



