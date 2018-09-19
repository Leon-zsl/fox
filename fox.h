#ifndef __FOX_H__
#define __FOX_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#define FOX_VERSION "0.0.1"

extern int log_level;
#define LOG_NO   0
#define LOG_ERR  1
#define LOG_WARN 2
#define LOG_MSG  3

#define log_error(fmt, ...) 			        \
if(log_level >= LOG_ERR) {						\
	char errmsg[1024];							\
	sprintf(errmsg, fmt, ##__VA_ARGS__);		\
	fprintf(stdout, "[ERROR]FILE:%s LINE:%d %s\n", __FILE__, __LINE__, errmsg); \
}

#define log_warn(fmt, ...)             			\
if(log_level >= LOG_WARN) {						\
	char errmsg[1024];							\
	sprintf(errmsg, fmt, ##__VA_ARGS__);		\
	fprintf(stdout, "[WARN]FILE:%s LINE:%d %s\n", __FILE__, __LINE__, errmsg); \
}

#define log_info(fmt, ...)                      \
if(log_level >= LOG_MSG) {						\
	char errmsg[1024];							\
	sprintf(errmsg, fmt, ##__VA_ARGS__);		\
	fprintf(stdout, "[INFO]FILE:%s LINE:%d %s\n", __FILE__, __LINE__, errmsg); \
}

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef size_t l;
typedef int bool;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

/*
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
*/

#ifndef ct_assert
#define ct_assert(e) { enum { compile_time_assert_value = 1/ !!(e) } }
#endif

static inline char *fox_strdup(const char *s) {
	if(s == NULL) return NULL;
	size_t l = strlen(s);
	char *ds = malloc(l+1);
	if(l == 0) ds[0] = '\0';
	else strncpy(ds, s, l);
	return ds;
}

static inline int fox_strempty(const char *s) {
	return s == NULL || strlen(s) == 0;
}

static inline char *fox_strcat(const char *s0, const char * s1) {
	if(!s0) {
		return fox_strdup(s1);
	}
	if(!s1) {
		return fox_strdup(s0);
	}

	int l0 = strlen(s0);
	int l1 = strlen(s1);
	char *d = malloc(l0 + l1 + 1);
	if(l0 + l1 == 0) {
		d[0] = '\0';
	} else {
		strncpy(d, s0, l0);
		strncpy(d + l0, s1, l1);
	}
	return d;
}
#endif
