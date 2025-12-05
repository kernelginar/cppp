#include "file_info.h"

file_info get_file_info(const char *file_name, off_t num_parts) {
	file_info file_info;
	struct stat st;
	file_info.status = stat(file_name, &st);
	off_t file_size = st.st_size;
	off_t part_size = file_size / num_parts;
	off_t last_part_size = file_size - part_size * (num_parts - 1);
	mode_t permissions = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
	mode_t st_mode = st.st_mode;
	ino_t st_ino = st.st_ino;
	dev_t st_dev = st.st_dev;

	realpath(file_name, file_info.file_name);
	file_info.file_size = file_size;
	file_info.part_size = part_size;
	file_info.last_part_size = last_part_size;
	file_info.num_parts = num_parts;
	file_info.permissions = permissions;
	file_info.st_mode = st_mode;
	file_info.st_ino = st_ino;
	file_info.st_dev = st_dev;
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