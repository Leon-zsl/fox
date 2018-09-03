#include <stdio.h>
#include "list.h"

void handle(struct node *n) {
	printf("handle node:%p,%p,%p\n", n, n->next, n->prev);
}

int main(int argc, char **argv) {
	struct list l;
	list_init(&l);
	
	struct node n0,n1,n2;
	list_push_head(&l, &n0);
	list_push_head(&l, &n1);
	list_push_head(&l, &n2);

	list_foreach(&l, handle);
	list_foreach_reverse(&l, handle);
	
	list_pop_tail(&l);
	list_pop_tail(&l);
	list_pop_tail(&l);

	list_foreach(&l, handle);
	list_foreach_reverse(&l, handle);
	return 0;
}
