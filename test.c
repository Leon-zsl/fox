#include <stdio.h>
#include "list.h"
#include "hmap.h"

void handle_list(struct lnode *n) {
	printf("handle list node:%p,%p,%p\n", n, n->next, n->prev);
}

void handle_hmap(size_t key, void *value) {
	printf("handle hmap node: %zu,%p\n", key, value);
}

int main(int argc, char **argv) {
	struct list l;
	list_init(&l);
	
	struct lnode n0,n1,n2;
	list_push_head(&l, &n0);
	list_push_head(&l, &n1);
	list_push_head(&l, &n2);

	list_foreach(&l, handle_list);
	list_foreach_reverse(&l, handle_list);
	
	list_pop_tail(&l);
	list_pop_tail(&l);
	list_pop_tail(&l);

	list_foreach(&l, handle_list);
	list_foreach_reverse(&l, handle_list);

	struct hmap m;
	hmap_init(&m, 16);
	printf("%d\n", hmap_insert(&m, HKEY_INT(0), HVALUE(16)));
	printf("%d\n", hmap_insert(&m, HKEY_STR("asd"), HVALUE(18)));
	printf("%d\n", hmap_insert(&m, HKEY_STR("12131"), HVALUE(24)));

	int a,b,c;
	printf("%d\n", hmap_get(&m, HKEY_INT(0), HVALUE_PTR(a)));
	printf("%d\n", hmap_get(&m, HKEY_STR("asd"), HVALUE_PTR(b)));
	printf("%d\n", hmap_get(&m, HKEY_STR("12131"), HVALUE_PTR(c)));

	printf("a=%d,b=%d,c=%d\n", a,b,c);

	hmap_foreach(&m, handle_hmap);

	printf("%d\n", hmap_remove(&m, HKEY_INT(0), NULL));
	printf("%d\n", hmap_remove(&m, HKEY_STR("asd"), NULL));
	printf("%d\n", hmap_remove(&m, HKEY_STR("12131"), NULL));
	
	printf("%d\n", hmap_get(&m, HKEY_INT(0), HVALUE_PTR(a)));
	printf("%d\n", hmap_get(&m, HKEY_STR("asd"), HVALUE_PTR(b)));
	printf("%d\n", hmap_get(&m, HKEY_STR("12131"), HVALUE_PTR(c)));

	printf("a=%d,b=%d,c=%d\n", a,b,c);

	hmap_foreach(&m, handle_hmap);	
	
	return 0;
}
