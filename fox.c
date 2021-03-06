#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "fox.h"
#include "symbol.h"
#include "syntax.h"
#include "translator.h"

#ifdef DEBUG
int log_level = LOG_DEBUG;
#else
int log_level = LOG_MSG;
#endif

int ensure_path(const char *srcpath, const char *destpath) {
	int val = access(srcpath, R_OK);
	if(val) {
		log_error("can not access srcpath:%s", srcpath);
		return val;
	}

	char *destdir = NULL;
	char *dest = malloc(strlen(destpath)+1);
	strcpy(dest, destpath);
	struct stat stsrc;
	stat(srcpath, &stsrc);
	if(S_ISREG(stsrc.st_mode)) {
		destdir = dirname(dest);
	} else if(S_ISDIR(stsrc.st_mode)) {
		destdir = dest;
	}

	val = access(destdir, W_OK);
	if(!val) {
		free(dest);
		return 0;
	}

	//create dest folder
	char cmd[1024];
	sprintf(cmd, "mkdir -p %s", destdir);
	system(cmd);
	val = access(destdir, W_OK);
	if(val) {
		log_error("can not access destpath:%s", destpath);
		free(dest);
		return val;
	}

	free(dest);
	return 0;
}

int process(const char *srcpath, const char *destpath) {
	int val = ensure_path(srcpath, destpath);
	if(val) return val;

	struct stat st;
	stat(srcpath, &st);
	if(S_ISREG(st.st_mode)) {
		char *extname = strrchr(srcpath, '.');
		if(!extname || strcmp(extname, ".lua")) {
			log_info("skip non-lua file: %s", srcpath);
			return 0;
		}

		struct syntax_tree *tree = NULL;
		struct symbol_table *table = NULL;
		int val = parse(srcpath, &tree, &table);
		if(!val) {
			log_error("parse file failed:%s", srcpath);
			return -1;
		}

		val = translate(destpath, tree, table);
		syntax_tree_release(tree);
		symbol_table_release(table);
		if(!val) {
			log_error("translate file failed:%s", destpath);
			return -1;
		}
	} else if(S_ISDIR(st.st_mode)) {
		DIR *dir = opendir(srcpath);
		if(!dir) {
			log_error("opendir failed:%s", srcpath);
			return -1;
		}

		struct dirent *ent;
		while((ent = readdir(dir)) != NULL) {
			if(!strcmp(ent->d_name, ".")) continue;
			if(!strcmp(ent->d_name, "..")) continue;

			char cursrc[1024];
			strcpy(cursrc, srcpath);
			strcat(cursrc, "/");
			strcat(cursrc, ent->d_name);

			char curdest[1024];
			strcpy(curdest, destpath);
			strcat(curdest, "/");
			strcat(curdest, ent->d_name);
			char * extname = strrchr(curdest, '.');
			if(extname && !strcmp(extname, ".lua")) {
				strcpy(extname, ".js");
			}

			int val = process(cursrc, curdest);
			if(val) return val;
		}
		closedir(dir);
	} else {
		log_warn("illeagal file: %s", srcpath);
		return 0;
	}

	return 0;
}

const char *usage = "usage: fox src dest (support file or folder)\n";

int main(int argc, char **argv) {
	if(argc < 3) {
		log_error("too few params!\n%s", usage);
		return 1;
	}
	if(argc > 3) {
		log_error("too many params!\n%s", usage);
		return 1;
	}

	log_info("processing start... src: %s, dest: %s", argv[1], argv[2]);

	int srclen = strlen(argv[1]);
	char *srcpath = malloc(srclen+1);
	strcpy(srcpath, argv[1]);
	if(srcpath[srclen-1] == '/') {
		srcpath[srclen-1] = '\0';
	}

	int destlen = strlen(argv[2]);
	char *destpath = malloc(srclen+1);
	strcpy(destpath, argv[2]);
	if(destpath[destlen-1] == '/') {
		destpath[destlen-1] = '\0';
	}

	int val = process(srcpath, destpath);
	if(val) {
		log_error("processing error! error code:%d\n", val);
	} else {
		log_info("processing succeed!\n");
	}

	free(srcpath);
	free(destpath);
	return val;
}
