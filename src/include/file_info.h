#ifndef FILE_INFO
#define FILE_INFO

#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
		char file_name[PATH_MAX];
		off_t file_size;
		off_t part_size;
		off_t last_part_size;
		unsigned int num_parts;
		mode_t permissions;
		mode_t dir_permissions;
		int status;
		mode_t st_mode;
		ino_t st_ino;
		dev_t st_dev;
} file_info;

file_info get_file_info(const char *file_name, off_t num_parts);
char *get_only_file_name(const char *path);

#endif