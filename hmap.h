#ifndef __HMAP_H__
#define __HMAP_H__

#include <stdlib.h>
#include <string.h>

#define HKEY_INT(i) ((size_t)(i))
#define HKEY_STR(s) (hash_string(s))

#define HVALUE_INT(i) ((void*)(i))
#define HVALUE_STR(s) ((void*)(s))

static inline size_t hash_string(const char *s) {
	if(!s) return 0;
	
	size_t hash = 5381;
    int c = 0;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
    return hash;
}

struct hnode {
	struct hnode *next;
	size_t key;
	void* value;
};
	
struct hmap {
	size_t bsize;
	struct hnode *bucket;
	size_t count;
};

typedef void (*hmap_handler)(size_t key, void *value);

static inline void hmap_init(struct hmap *m, size_t bsize) {
	m->bsize = bsize;
	m->bucket = (struct hnode *)malloc(sizeof(struct hnode) * bsize);
	memset(m->bucket, 0, sizeof(struct hnode) * bsize);
	m->count = 0;	
}

static inline int hmap_empty(struct hmap *m) {
	return m->count == 0;
}

static inline int hmap_get(struct hmap *m, size_t key, void **value) {
	struct hnode *n = m->bucket[key % m->bsize].next;
	while(n) {
		if(n->key == key) {
			if(value) {
				*value = n->value;
			}
			return 1;
		}
		n = n->next;
	}
	return 0;
}

static inline int hmap_insert(struct hmap *m, size_t key, void *value) {
	struct hnode *n = &m->bucket[key % m->bsize];
	while(n->next) {
		if(n->next->key == key) {
			return 0;
		}
		n = n->next;
	}
	
	struct hnode *c = (struct hnode *)malloc(sizeof(struct hnode));
	c->next = NULL;
	c->key = key;
	c->value = value;
	n->next = c;
	m->count++;
	return 1;
}

static inline int hmap_remove(struct hmap *m, size_t key, void **value) {
	struct hnode *p = &m->bucket[key % m->bsize];
	struct hnode *n = p->next;
	while(n) {
		if(n->key == key) {
			if(value) {
				*value = n->value;
			}
			break;
		}
		p = n;
		n = p->next;
	}
	//not found
	if(!n) {
		return 0;
	}
	p->next = n->next;
	m->count--;
	free(n);
	return 1;
}

static inline void hmap_foreach(struct hmap *m, hmap_handler h) {
	for(size_t i = 0; i < m->bsize; i++) {
		struct hnode *n = m->bucket[i].next;
		while(n) {
			h(n->key, n->value);
			n = n->next;
		}
	}
}

static inline void hmap_clear(struct hmap *m, hmap_handler h) {
	for(size_t i = 0; i < m->bsize; i++) {
		struct hnode *n = m->bucket[i].next;
		while(n) {
			struct hnode *c = n;
			n = n->next;
			h(c->key, c->value);
			free(c);
		}
	}
	free(m->bucket);
	m->bsize = 0;
	m->count = 0;
}

#endif
