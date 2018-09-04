#include "fox.h"
#include "symbol.h"
#include "syntax.h"
#include "parser.h"

#define YY_NULL 0

int yylex(void);
void yyset_in(FILE *in);
void yyset_out(FILE *out);
void yyset_lineno(int lineno);
void yyset_filename(const char *name);

int parse(const char *filename, struct syntax_tree *tree) {
	if(!tree) {
		log_error("syntax tree is invalid");
		return -1;
	}
	
	FILE *fp = fopen(filename, "rb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return -1;
	}

	yyset_in(fp);
	yyset_out(stdout);
	yyset_filename(filename);
	yyset_lineno(1);

	/* yylex function test */
	while(yylex() != YY_NULL);

	/* int val = yyparse(); */
	/* if(val) { */
	/* 	fclose(fp); */
	/* 	return val; */
	/* } */

	fclose(fp);
	return 0;
}
