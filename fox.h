#ifndef __FOX_H__
#define __FOX_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define FOX_VERSION "0.0.1"

#define log_error(fmt, ...) {			                       \
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

#endif
