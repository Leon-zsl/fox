#include "fox.h"
#include "syntax.h"

struct syntax_node *syntax_node_create(int ty, int lineno, const char *name) {
	struct syntax_node *n = (struct syntax_node *)malloc(sizeof(struct syntax_node));
	n->parent = NULL;
	list_init(&n->children);
	
	n->type = ty;
	n->lineno = lineno;
	strcpy(n->name, name);
	return n;
}

void syntax_node_release(struct syntax_node *n) {
	struct node *c = list_head(&n->children);
	while(c != list_end(&n->children)) {
		struct node *sc = c;
		c = list_next(c);
		
		list_remove(sc);
		syntax_node_release((struct syntax_node *)sc);
	}
	free(n);
}

struct syntax_tree *syntax_tree_create() {
	struct syntax_tree *t = (struct syntax_tree *)malloc(sizeof(struct syntax_tree));
	list_init(&t->l);
	return t;
}

void syntax_tree_release(struct syntax_tree *t) {
	struct node *n = list_head(&t->l);
	while(n != list_end(&t->l)) {
		struct node *sn = n;
		n = list_next(n);

		list_remove(sn);
		syntax_node_release((struct syntax_node *)sn);
	}
}
