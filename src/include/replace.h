#ifndef REPLACE
#define REPLACE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "file_info.h"

#define BUFFER_SIZE 4 * 1024

int replace(const char *file_name, off_t num_parts, off_t position, const char *new_data_file_name) {
	if (position > num_parts || position <= 0) {
		print_err("Position number cannot be greater than num_parts or cannot be less than 1 (replace.h)");
		exit(EXIT_FAILURE);
	}

	int fd_file = open(file_name, O_WRONLY);
	if (fd_file == -1) {
		print_err("File couldn't be opened (replace.h)");
		exit(EXIT_FAILURE);
	}

	int fd_data_file = open(new_data_file_name, O_RDONLY);
	if (fd_data_file == -1) {
		print_err("Data file couldn't be opened (replace.h)");
		close(fd_file);
		exit(EXIT_FAILURE);
	}

	file_info main_file_info = get_file_info(file_name, num_parts);
	file_info data_file_info = get_file_info(new_data_file_name, 1);

	off_t main_offset = main_file_info.part_size * (position - 1);
	off_t write_size;

	if ((main_file_info.file_size - main_offset) == main_file_info.last_part_size) {
		write_size = main_file_info.last_part_size;
	} else {
		write_size = main_file_info.part_size;
	}

	char buffer[BUFFER_SIZE];
	off_t data_offset = 0;

	ssize_t bytes_read, bytes_written;
	while ((bytes_read = pread(fd_data_file, buffer, BUFFER_SIZE, data_offset)) > 0 && bytes_read <= write_size) {
		if (main_offset == main_file_info.part_size * (position)) {
			break;
		}

		bytes_written = pwrite(fd_file, buffer, bytes_read, main_offset);
		if (bytes_written != bytes_read) {
			print_err("pwrite error (replace.h)");
			close(fd_file);
			close(fd_data_file);
			return EXIT_FAILURE;
		}

		main_offset += bytes_read;
		data_offset += bytes_written;
	}

	if (bytes_read == -1) {
		print_err("pread error (replace.h)");
		close(fd_file);
		close(fd_data_file);
		return EXIT_FAILURE;
	}

	close(fd_file);
	close(fd_data_file);
	return EXIT_SUCCESS;
}

#endif