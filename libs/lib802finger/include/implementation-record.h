#include "fingerlib-datatypes.h"
class ImplementationRecord
{
    private:
	bool initialized;
	int version_num;
    public:
    string record_format;

    u_int32_t id;

    string card_vendor;
    string card_model;
    string card_version;

    string OS;
    string OS_version;

    string chipset_vendor;
    string chipset;

    string driver_name;
    string driver_version;

    string md5;

    string notes;


    public:
    ImplementationRecord();
    bool ParseCSV(char *s);
    string ToCSV();
	string ToString();
}; 
