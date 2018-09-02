#include "fox.h"
#include "parser.h"

int parse(const char *filename, struct syntax_tree *tree) {
	if(tree == NULL) {
		log_error("syntax tree is invalid");
		return -1;
	}
	
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL) {
		log_error("open file failed %s", filename);
		return -1;
	}

	*tree = NULL;
	//todo: parse file and create syntex tree

	fclose(fp);
	return 0;
}
