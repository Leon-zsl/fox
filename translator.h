#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

int translate(const char *filename, struct syntax_tree *tree);
int translate_syntax_node(FILE *file, struct syntax_node *n);

#endif
