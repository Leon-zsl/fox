#ifndef __SYMBOL_H__
#define __SYMBOL_H__

struct symbol {
	char *name;	/* name must be unique for unique symbol */
	struct syntax_node *node;
};

struct symbol *symbol_create(const char *name, struct syntax_node *node);
void symbol_release(struct symbol *s);

struct symbol_table {
	struct hmap *m;
};

struct symbol_table *symbol_table_create();
void symbol_table_release(struct symbol_table *t);
void symbol_table_insert(struct symbol_table *t, struct symbol *s);
void symbol_table_remove(struct symbol_table *t, struct symbol *s);
struct symbol *symbol_table_get(struct symbol_table *t, const char *name);
struct symbol *symbol_table_set(struct symbol_table *t, struct symbol *s);

#endif
