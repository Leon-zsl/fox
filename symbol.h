#ifndef __SYMBOL_H__
#define __SYMBOL_H__

enum symbol_type {
	SYM_VAR,
	SYM_FUNC,
	SYM_VAR_LOCAL,
	SYM_FUNC_LOCAL,
};

struct symbol {
	char *name;	/* name must be unique for unique symbol */
	enum symbol_type type;
};

struct symbol *symbol_create(const char *name, enum symbol_type type);
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

typedef void (*symbol_handler)(const char *name, struct symbol *s);
void symbol_table_walk(struct symbol_table *t, symbol_handler h);

#endif
