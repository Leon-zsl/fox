#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "fox.h"
#include "parser.h"
#include "generator.h"

int ensure_path(const char *srcpath, const char *destpath) {
	int val = access(srcpath, R_OK);
	if(val) {
		log_error("can not access srcpath:%s", srcpath);
		return val;
	}

	char *dest = malloc(strlen(destpath)+1);
	strcpy(dest, destpath);
	char *destdir = NULL;
	
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

int translate(const char *src, const char *dest) {
	int val = ensure_path(src, dest);
	if(val) return val;

	struct stat st;
	stat(src, &st);
	if(S_ISREG(st.st_mode)) {
		if(strcmp(strrchr(src, '.'), ".lua")) {
			log_warn("src is not lua file: %s", src);
			return 0;
		}

		struct syntax_tree tree;
		tree_init(&tree);
		int val = parse(src, &tree);
		if(val) return val;

		val = generate(dest, &tree);
		tree_release(&tree);
		if(val) return val;
	} else if(S_ISDIR(st.st_mode)) {
		DIR *dir = opendir(src);
		if(!dir) {
			log_error("opendir failed:%s", src);
			return -1;
		}

		struct dirent *ent;
		while((ent = readdir(dir)) != NULL) {
			if(!strcmp(ent->d_name, ".")) continue;
			if(!strcmp(ent->d_name, "..")) continue;
			char *extname = strrchr(ent->d_name, '.');
			if(!extname || strcmp(extname, ".lua")) continue;

			char cursrc[1024];
			strcpy(cursrc, src);
			strcat(cursrc, ent->d_name);

			char curdest[1024];
			strcpy(curdest, dest);
			strcat(curdest, "/");
			strcat(curdest, ent->d_name);
			char * extlua = strrchr(curdest, '.');
			strcpy(extlua, ".js");

			translate(cursrc, curdest);
		}
		closedir(dir);
	} else {
		log_warn("illeagal file: %s", src);
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

	log_info("translating start... src: %s, dest: %s\n", argv[1], argv[2]);
	
	char *srcpath = malloc(strlen(argv[1])+1);
	strcpy(srcpath, argv[1]);
	char *destpath = malloc(strlen(argv[2])+1);
	strcpy(destpath, argv[2]);

	int val = translate(srcpath, destpath);
	if(val) {
		log_error("translating error! error code:%d\n", val);
	} else {
		log_info("translating succeed!\n");
	}

	free(srcpath);
	free(destpath);
	return val;
}
