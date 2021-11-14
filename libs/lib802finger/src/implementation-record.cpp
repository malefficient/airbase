#include "fingerlib-datatypes.h"

ImplementationRecord::ImplementationRecord()
{
	initialized=false;	
	version_num = 0;

}
bool ImplementationRecord::ParseCSV(char *line)
{
  	char * pch;
 	pch = strtok (line,",");
	if (pch == NULL)
		return false;
	if (atoi(pch) != 0)
	{
		printf("ImplementationRecord::ParseCSV:Error, invalid version number.\n");
		exit(0);
		return false;
	}
	//From here on out we assume versoin number 0
	
   	pch = strtok (NULL, ",");
	int cnt = 0;
 	while (pch != NULL)
  	{

		switch (cnt)
		{
			case 0: id = atoi(pch); break;
			case 1: card_vendor = pch; break;
			case 2: card_model = pch; break;
			case 3: card_version = pch; break;
			case 4: OS = pch;  break;
			case 5: OS_version = pch; break;
			case 6: chipset_vendor = pch; break;
			case 7: chipset = pch; break;
			case 8: driver_name = pch; break;
			case 9: driver_version = pch; break;
			case 10: md5 = pch; break;
			case 11: notes = pch; break;
		};

	cnt++;
   	pch = strtok (NULL, ",");
  	}
	if (cnt != 12)
	{
		printf("ImplementationRecord::ParseCSV:Error Trailing junk following notes field.\n");
		exit(0);
	}

	initialized=true;
	return true;

}

string ImplementationRecord::ToString()
{
	char tmp[2048];
	string ret;

	snprintf(tmp, sizeof(tmp),  "Vendor:\t%s\n", card_vendor.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "Model:\t%s\n", card_model.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "Version:\t%s\n", card_version.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "OS:\t\t%s\n", OS.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "OS-Version:\t%s\n", OS_version.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "chipset-vendor:\t%s\n", chipset_vendor.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "chipset:\t%s\n", chipset.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "driver-name:\t%s\n", driver_name.c_str());
	ret += tmp;

	snprintf(tmp, sizeof(tmp),  "driver-version:\t%s\n", driver_version.c_str());
	ret += tmp;


	return ret;
}

string ImplementationRecord::ToCSV()
{
	
	if (!initialized)
	{
		printf("ImplemetnationREcord::ToCSV::Error. Initalized = false.\n");
		exit(0);
	}
	
}

