#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "list.h"

struct symbol {
	struct node n;
	char name[128];
};

struct symbol *symbol_create(const char *name);
void symbol_release(struct symbol *s);

struct symbol_table {
	struct list l;
};

struct symbol_table *symbol_table_create();
void symbol_table_release(struct symbol_table *t);
void symbol_talbe_add(struct symbol_table *t, struct symbol *s);
void symbol_table_remove(struct symbol_table *t, struct symbol *s);
struct symbol *symbol_table_find(struct symbol_table *t, const char *name);
	
#endif
