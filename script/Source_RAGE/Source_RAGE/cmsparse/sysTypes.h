#ifndef SYSTYPES_H_
#define SYSTYPES_H_

typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
#ifdef _WIN32
 typedef long int32;
 // typedef unsigned long uint32;
 typedef unsigned int uint32;
 typedef __int64 int64;
 typedef unsigned __int64 uint64;
#else
 typedef int int32;
 typedef unsigned int uint32;
 typedef long long int64;
 typedef unsigned long long uint64;
 //using std::size_t;
 //using std::ptrdiff_t;
#endif

#endif /*SYSTYPES_H_*/
