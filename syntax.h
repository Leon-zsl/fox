#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include "list.h"

struct syntax_node {
	struct node n;
	struct syntax_node *parent;
	struct list children;

	int type;
	char name[128];
	int lineno;
};

struct syntax_node *syntax_node_create(int ty, int lineno, const char *name);
void syntax_node_release(struct syntax_node *n);

struct syntax_tree {
	struct list l;
};

struct syntax_tree *syntax_tree_create();
void syntax_tree_release(struct syntax_tree *t);

#endif
