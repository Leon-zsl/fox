#ifndef __PARSER_H__
#define __PARSER_H__

struct parser {
	char *filename;
	struct syntax_tree *tree;
	struct symbol_table *table;
};

struct parser *parse(const char *filename);
void parser_release(struct parser *p);

#endif
