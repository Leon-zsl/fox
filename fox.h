#ifndef __FOX_H__
#define __FOX_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define FOX_VERSION "0.0.1"

#define log_error(fmt, ...) {			                        \
	char errmsg[1024];											\
	sprintf(errmsg, fmt, ##__VA_ARGS__);						\
	fprintf(stdout, "[ERROR]FILE:%s LINE:%d %s\n", __FILE__, __LINE__, errmsg); \
}

#define log_warn(fmt, ...) {            			            \
	char errmsg[1024];											\
	sprintf(errmsg, fmt, ##__VA_ARGS__);						\
	fprintf(stdout, "[WARN]FILE:%s LINE:%d %s\n", __FILE__, __LINE__, errmsg); \
}

#define log_info(fmt, ...) {                        			\
	char errmsg[1024];											\
	sprintf(errmsg, fmt, ##__VA_ARGS__);						\
	fprintf(stdout, "[INFO]FILE:%s LINE:%d %s\n", __FILE__, __LINE__, errmsg); \
}

#ifndef NULL
#define NULL 0
#endif

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

#endif
