#ifndef __LIST_H__
#define __LIST_H__

struct node {
	struct node *next;
	struct node *prev;
};

struct list {
	struct node root;
};

typedef void (*list_handler)(struct node *n);

static inline void list_init(struct list *l) { l->root.next = l->root.prev = &l->root; }
static inline struct node *list_begin(struct list *l) { return l->root.next; }
static inline struct node *list_end(struct list *l) { return &l->root; }
static inline struct node *list_head(struct list *l) { return l->root.next; }
static inline struct node *list_tail(struct list *l) { return l->root.prev; }
static inline int list_empty(struct list *l) { return list_begin(l) == list_end(l); }

static inline struct node *list_next(struct node *n) { return n->next; }
static inline struct node *list_prev(struct node *n) { return n->prev; }

static inline void list_insert(struct node *p, struct node *n) {
	n->next = p->next;
	n->prev = p;
	p->next->prev = n;
	p->next = n;	
}

static inline struct node *list_remove(struct node *p) {
	p->prev->next = p->next;
	p->next->prev = p->prev;
	return p;
}

static inline void list_push_head(struct list *l, struct node *n) {
	list_insert(&l->root, n);
}

static inline struct node *list_pop_head(struct list *l) {
	return list_remove(l->root.next);
}

static inline void list_push_tail(struct list *l, struct node *n) {
	list_insert(l->root.prev, n);
}

static inline struct node *list_pop_tail(struct list *l) {
	return list_remove(l->root.prev);
}

static inline void list_foreach(struct list *l, list_handler h) {
	for(struct node *n = list_head(l); n != list_end(l); n = list_next(n)) h(n);
}

static inline void list_foreach_reverse(struct list *l, list_handler h) {
	for(struct node *n = list_tail(l); n != list_end(l); n = list_prev(n)) h(n);
}

#endif
