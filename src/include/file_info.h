#ifndef FILE_INFO
#define FILE_INFO

#include <sys/stat.h>
#include <string.h>

typedef struct {
	off_t file_size;
	off_t part_size;
	off_t last_part_size;
	off_t num_parts;
	mode_t permissions;
} file_info;

file_info get_file_info(const char* file_name, off_t num_parts) {
	file_info file_info;
	struct stat st;
	stat(file_name, &st);
	off_t file_size = st.st_size;
	off_t part_size = file_size / num_parts;
	off_t last_part_size = file_size - part_size * (num_parts - 1);
	mode_t permissions = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

	file_info.file_size = file_size;
	file_info.part_size = part_size;
	file_info.last_part_size = last_part_size;
	file_info.num_parts = num_parts;
	file_info.permissions = permissions;
	return file_info;
}

char *get_only_file_name(const char *path) {
	const char *file_name = strrchr(path, '/');
	if (file_name != NULL) {
		return (char *)(file_name + 1);
	} else {
		return (char *)path;
	}
}

#endif