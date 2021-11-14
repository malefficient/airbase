#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H
#include <limits.h>
#include <sys/types.h>
//These are kind of kludgey because tehy glue me and joshwr1ghts (standard)
//typing system together.
//typedef unsigned char	u_int8_t;
typedef u_int8_t u8;
//typedef unsigned short 	u_int16_t;
typedef u_int16_t u16;
//typedef unsigned int	u_int32_t;
typedef u_int32_t u32;

typedef unsigned char	u_char_t;


#define U_INT8_T_MAX UCHAR_MAX
#define U_INT16_T_MAX USHRT_MAX
#define U_INT32_T_MAX UINT_MAX
#define U_CHAR_T_MAX UCHAR_MAX
#endif
