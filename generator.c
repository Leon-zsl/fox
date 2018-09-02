#include "fox.h"
#include "parser.h"
#include "generator.h"

int generate(const char *filename, struct syntax_tree *tree) {
	if(!tree) {
		log_error("syntax tree is invalid");
		return -1;
	}

	FILE *fp = fopen(filename, "wb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return -1;
	}

	log_info("generate file:%s", filename);
	//todo: traverse syntax tree and generate file
	
	fclose(fp);
	return 0;
}
