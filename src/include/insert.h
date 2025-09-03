#ifndef INSERT
#define INSERT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "file_info.h"
#include "merge.h"

#define BUFFER_SIZE 4 * 1024

int insert_data(const char *file_name, off_t num_parts, off_t part_location, const char *new_data_file_name) {
	int fd_file = open(file_name, O_RDWR);
	if (fd_file == -1) {
		print_err("File couldn't be opened (insert.h)");
		return EXIT_FAILURE;
	}

	int fd_new_data_file = open(new_data_file_name, O_RDONLY);
	if (fd_new_data_file == -1) {
		print_err("Failed to open the data file to be inserted (insert.h)");
		close(fd_file);
		return EXIT_FAILURE;
	}

	char temp_file_name[256];
	snprintf(temp_file_name, sizeof(temp_file_name), "%s_part_XXXXXX", file_name);
	int fd_temp_part_file = mkstemp(temp_file_name);
	if (fd_temp_part_file == -1) {
		print_err("mkstemp failed (insert.h)");
		close(fd_file);
		close(fd_new_data_file);
		return EXIT_FAILURE;
	}

	print_success("mkstemp succeded, temp file created at %s (insert.h)", temp_file_name);

	file_info main_file_info = get_file_info(file_name, num_parts);

	if (part_location > num_parts) {
		print_err("Part_location can't be greater than num_parts (insert.h)");
		close(fd_file);
		close(fd_new_data_file);
		close(fd_temp_part_file);
		return EXIT_FAILURE;
	} else if (part_location == num_parts) {
		merge_files(file_name, new_data_file_name);
	} else if (part_location < num_parts) {
		off_t src_offset = main_file_info.part_size * part_location;

		char buffer[BUFFER_SIZE];
		off_t temp_offset = 0;

		ssize_t bytes_read, bytes_written;
		while ((bytes_read = pread(fd_file, buffer, BUFFER_SIZE, src_offset)) > 0) {
			bytes_written = pwrite(fd_temp_part_file, buffer, bytes_read, temp_offset);
			if (bytes_written != bytes_read) {
				print_err("pwrite error (insert.h)");
				close(fd_file);
				close(fd_new_data_file);
				close(fd_temp_part_file);
				return EXIT_FAILURE;
			}

			src_offset += bytes_read;
			temp_offset += bytes_written;
		}

		if (bytes_read == -1) {
			print_err("pread error (insert.h)");
			close(fd_file);
			close(fd_new_data_file);
			close(fd_temp_part_file);
			return EXIT_FAILURE;
		}

		if (ftruncate(fd_file, src_offset) == -1) {
			print_err("ftruncate error (insert.h)");
			close(fd_file);
			close(fd_new_data_file);
			close(fd_temp_part_file);
			return EXIT_FAILURE;
		}

		merge_files(file_name, new_data_file_name);
		merge_files(file_name, temp_file_name);

		close(fd_file);
		close(fd_new_data_file);
		unlink(temp_file_name);
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}

#endif