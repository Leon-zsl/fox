#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

int translate(const char *filename, struct syntax_tree *tree, struct symbol_table *table);
int translate_syntax_node(FILE *file, struct syntax_node *n, struct symbol_table *t);

#endif
