#include "fox.h"
#include "symbol.h"

struct symbol *symbol_create(const char *name) {
	struct symbol *s = (struct symbol *)malloc(sizeof(struct symbol));
	strcpy(s->name, name);
	return s;
}

void symbol_release(struct symbol *s) {
	free(s);
}

struct symbol_table *symbol_table_create() {
	struct symbol_table *t = (struct symbol_table *)malloc(sizeof(struct symbol_table));
	list_init(&t->l);
	return t;
}

void symbol_table_release(struct symbol_table *t) {
	struct node *n = list_head(&t->l);
	while(n != list_end(&t->l)) {
		struct symbol *s = (struct symbol *)n;
		n = list_next(n);
		symbol_table_remove(t, s);
		symbol_release(s);
	}
	free(t);
}

void symbol_table_add(struct symbol_table *t, struct symbol *s) {
	struct symbol *c = symbol_table_find(t, s->name);
	if(c != NULL) {
		log_error("duplicate symbol:%s", s->name);
		return;
	}
	list_push_head(&t->l, (struct node *)s);
}

void symbol_table_remove(struct symbol_table *t, struct symbol *s) {
	list_remove((struct node *)s);
}

struct symbol *symbol_table_find(struct symbol_table *t, const char *name) {
	for(struct node *n = list_begin(&t->l); n != list_end(&t->l); n = list_next(n)) {
		struct symbol *s = (struct symbol *)n;
		if(!strcmp(s->name, name)) {
			return s;
		}
	}
	return NULL;
}
