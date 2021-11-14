#include "jc-util.h"
#include <netinet/in.h> // for ntohs()
void print_hex(u_int8_t *start, u_int32_t len)
{
	print_hex(start, len, stdout);
}

void print_hex(u_int8_t *start, u_int32_t len, FILE*fp)
{
	fprintf(fp, "%s", get_hex_dump(start, len).c_str());
}

string  mac_to_string(u_int8_t *mac)
{
	char tmp[256];
	snprintf(tmp, sizeof(tmp), "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	string s = tmp;
	return s;
}
string hex2string(u_int8_t *start, u_int32_t len)
{
		return get_hex_dump(start, len);
}
//We had josh's improved hex dumper as default for awhile, but it
//messed upthe display of to many apps. The new hex dumper
//is available below.
string get_hex_dump(u_int8_t *start, u_int32_t len)
{
	string out;
	char tmp[8];
 
	u_int32_t cnt = 0;
	int char_cnt = 0;
	int grp_cnt = 0;

   while (cnt < len)
   {
		snprintf(tmp, sizeof(tmp), "%2.2x ", *(start + cnt));
   		out += tmp;
		grp_cnt++;
		char_cnt++;
		cnt++;
		if (grp_cnt == 8)
		{
			out += " : ";
			//printf(" : ");
			grp_cnt = 0;
		}
		if (char_cnt == 32)
		{
			out += "\n";
			//printf("\n");
			char_cnt = 0;
		}
	}
		//out +="\n
	return out;
}
string get_hex_dump_with_ascii(u_int8_t *start, u_int32_t len)
{
	string out;

	/* stolen from tcpdump, then kludged extensively */

	static const char asciify[] = "................................ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~.................................................................................................................................";

	const unsigned short *sp;
	const unsigned char *ap;
	unsigned int i, j;
	int nshorts, nshorts2;
	int padding;
	char tmp[8];

	out = "\n\t";
	padding = 0;
	sp = (unsigned short *)start;
	ap = (unsigned char *)start;
	nshorts = (unsigned int) len / sizeof(unsigned short);
	nshorts2 = (unsigned int) len / sizeof(unsigned short);
	i = 0;
	j = 0;
	while(1) {
		while (--nshorts >= 0) {
			snprintf(tmp, sizeof(tmp), " %04x", ntohs(*sp));
			out += tmp;
			sp++;
			if ((++i % 8) == 0)
				break;
		}
		if (nshorts < 0) {
			if ((len & 1) && (((i-1) % 8) != 0)) {
				snprintf(tmp, sizeof(tmp), " %02x  ", 
						*(unsigned char *)sp);
				out += tmp;
				padding++;
			}
			nshorts = (8 - (nshorts2 - nshorts));
			while(--nshorts >= 0) {
				out += "     ";
			}
			if (!padding) out += "     ";
		}
		out += "  ";

		while (--nshorts2 >= 0) {
			snprintf(tmp, sizeof(tmp), "%c%c", 
					asciify[*ap], asciify[*(ap+1)]);
			out += tmp;
			ap += 2;
			if ((++j % 8) == 0) {
				out += "\n\t";
				break;
			}
		}
		if (nshorts2 < 0) {
			if ((len & 1) && (((j-1) % 8) != 0)) {
				snprintf(tmp, sizeof(tmp), "%c", asciify[*ap]);
				out += tmp;
			}
			break;
		}
	}
	if ((len & 1) && (((i-1) % 8) == 0)) {
		snprintf(tmp, sizeof(tmp), " %02x", *(unsigned char *)sp);
		out += tmp;
		snprintf(tmp, sizeof(tmp),
				"                                       %c",
		       		asciify[*ap]);
		out += tmp;
	}
	out += "\n";

	return out;
}



