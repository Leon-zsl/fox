#ifndef __PARSER_H__
#define __PARSER_H__

#include "lua_y.h"

int parse(const char *filename, struct syntax_tree *tree);

#endif
