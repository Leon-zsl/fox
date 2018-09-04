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

	/* todo: parse file and create syntex tree */	
	log_info("parse file:%s", filename);

	yyset_in(fp);
	yyset_out(stdout);
	yyset_filename(filename);
	yyset_lineno(1);

	/* int val = yylex(); */
	/* while(val != YY_NULL) { */
	/* 	log_info("yylex result:%d", val); */
	/* 	val = yylex(); */
	/* } */

	int val = yyparse();
	if(val) {
		fclose(fp);
		return val;
	}

	log_info("finish parse:%s", filename);
	
	fclose(fp);
	return 0;
}
