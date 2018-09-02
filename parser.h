#ifndef __PARSER_H__
#define __PARSER_H__

#include "lua_y.h"

struct syntax_tree {
};

static inline void tree_init(struct syntax_tree *tree) {}
static inline void tree_release(struct syntax_tree *tree) {}

int parse(const char *filename, struct syntax_tree *tree);

#endif
