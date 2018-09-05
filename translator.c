#include "fox.h"
#include "symbol.h"
#include "parser.h"
#include "translator.h"

int translate(const char *filename, struct syntax_tree *tree) {
	if(!tree) {
		log_error("syntax tree is invalid");
		return -1;
	}

	FILE *fp = fopen(filename, "wb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return -1;
	}

	//todo: traverse syntax tree and translate it to file
	
	fclose(fp);
	return 0;
}
