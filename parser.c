#include "fox.h"
#include "symbol.h"
#include "syntax.h"
#include "parser.h"

#define YY_NULL 0

int yylex(void);
int yyparse(void);

void yyset_in(FILE *in);
void yyset_out(FILE *out);
void yyset_lineno(int lineno);
void yyset_filename(const char *name);

extern struct syntax_tree *parse_tree;
extern struct symbol_table *parse_table;

int parse(const char *filename, struct syntax_tree **tree, struct symbol_table **table) {
	FILE *fp = fopen(filename, "rb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return 0;
	}

	yyset_in(fp);
	yyset_out(stdout);
	yyset_filename(filename);
	yyset_lineno(1);
	/* yylex function test */
	/* while(yylex() != YY_NULL); */

	parse_tree = syntax_tree_create();
	parse_table = symbol_table_create();	
	if(yyparse()) {
		log_error("yyparse failed:%s", filename);
		syntax_tree_release(parse_tree);
		symbol_table_release(parse_table);
		fclose(fp);
		return 0;
	}

	fclose(fp);
	*tree = parse_tree;
	*table = parse_table;
	return 1;
}
