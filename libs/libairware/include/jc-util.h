#ifndef JC_UTIL_H
#define JC_UTIL_H
#include "typedefs.h"
#include <string>
using std::string;

void print_hex(u_int8_t * start, u_int32_t len ); 
//yes, I do know how to overload
//but for some reason using a default FILE* param causes build issues
void print_hex(u_int8_t * start, u_int32_t len, FILE* );

string get_hex_dump(u_int8_t * start, u_int32_t len);
string hex2string(u_int8_t * start, u_int32_t len);

string mac_to_string(u_int8_t *mac);

#endif


