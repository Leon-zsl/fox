#include "fox.h"
#include "list.h"
#include "hmap.h"
#include "symbol.h"

static void clear_handler(size_t key, void *value) {
	symbol_release((struct symbol *)value);
}

struct symbol *symbol_create(const char *name) {
	struct symbol *s = malloc(sizeof(struct symbol));
	s->name = strdup(name);
	return s;
}

void symbol_release(struct symbol *s) {
	free(s->name);
	free(s);
}

struct symbol_table *symbol_table_create() {
	struct symbol_table *t = malloc(sizeof(struct symbol_table));
	t->m = malloc(sizeof(struct hmap));
	hmap_init(t->m, 256);
	return t;
}

void symbol_table_release(struct symbol_table *t) {
	hmap_clear(t->m, clear_handler);
	free(t->m);
	free(t);
}

void symbol_table_insert(struct symbol_table *t, struct symbol *s) {
	if(!s || !s->name) return;
	hmap_insert(t->m, HKEY_STR(s->name), s);
}

void symbol_table_remove(struct symbol_table *t, struct symbol *s) {
	if(!s || !s->name) return;	
	hmap_remove(t->m, HKEY_STR(s->name), NULL);
}

struct symbol *symbol_table_get(struct symbol_table *t, const char *name) {
	if(!name) return NULL;
	struct symbol *s = NULL;
	hmap_get(t->m, HKEY_STR(name), HVALUE_PTR(s));
	return s;
}

struct symbol *symbol_table_set(struct symbol_table *t, struct symbol *s) {
	if(!s || !s->name) return NULL;
	struct symbol *ps = NULL;
	int key = HKEY_STR(s->name);
	hmap_remove(t->m, key, HVALUE_PTR(ps));
	hmap_insert(t->m, key, s);
	return ps;
}
