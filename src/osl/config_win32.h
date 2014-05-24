/* OSL_CONFIG_SYSTEM_H: configuration file for platform without autoconfig.
 * Lists various machine properties-- type sizes, endianness, etc.
 */
#ifndef __OSL_CONFIG_SYSTEM_H
#define __OSL_CONFIG_SYSTEM_H
#define OSL_bitsPerChar 8

/*2-byte type*/
typedef short osl_int16;
typedef unsigned short osl_uint16;
/*4-byte type*/
typedef int osl_int32;
typedef unsigned int osl_uint32;

/*8-byte type*/
typedef __int64 osl_int64;

//No way to do this in stupid, stupid win32 compiler?
typedef unsigned __int64 osl_uint64;

#define OSL_LIL_ENDIAN 1
#define OSL_LIL_IEEE 1 /*Floating Point is IEEE, little-endian*/
#define OSL_USE_MMX_GNU 0 /*Intel x86 MMX extension*/

#define OSL_NEED_INT64  1
#define OSL_HAS_LONG_DOUBLE 0

#endif /*defined(__OSL_CONFIG_SYSTEM_H)*/
