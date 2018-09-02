#include "fox.h"
#include "parser.h"
#include "generator.h"

int generate(struct syntax_tree *tree, const char *filename) {
	if(tree == NULL) {
log_error("syntax tree is invalid");
		return -1;
	}

	FILE *fp = fopen(filename, "wb");
	if(fp == NULL) {
		log_error("open file failed %s", filename);
		return -1;
	}

	//todo: traverse syntax tree and generate file
	
	fclose(fp);
	return 0;
}
