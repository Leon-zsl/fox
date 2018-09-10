#ifndef __PARSER_H__
#define __PARSER_H__

extern struct syntax_tree *parse_tree;
extern struct symbol_table *parse_table;

int parse(const char *filename, struct syntax_tree **tree, struct symbol_table **table);

#endif
