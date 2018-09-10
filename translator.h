#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

int parse(const char *filename, struct syntax_tree **tree, struct symbol_table **table);
int translate(struct syntax_tree *tree, struct symbol_table *table, const char *filename);

#endif
