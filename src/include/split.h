#ifndef SPLIT
#define SPLIT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "file_info.h"
#include "print_warns.h"

#define BUFFER_SIZE 4 * 1024

int split_file(const char* file_name, off_t num_parts) {
	int fd_file = open(file_name, O_RDWR);
	if (fd_file == -1) {
		print_err("File couldn't be opened (split.h)");
		return EXIT_FAILURE;
	}

	file_info file_info = get_file_info(file_name, num_parts);

	char buffer[BUFFER_SIZE];
	off_t part_size = file_info.part_size;
	off_t last_part_size = file_info.last_part_size;

	for (int i = 0; i < num_parts; i++) {
		char part_name[256];
		// part_name (256) -> $(file_name).part$(i + 1) -> example.part1, example.part2
		snprintf(part_name, sizeof(part_name), "%s.part%d", file_name, i + 1);

		int fd_part = open(part_name, O_WRONLY | O_CREAT | O_TRUNC, file_info.permissions);
		if (fd_part == -1) {
			print_err("Part file couldn't be created (split.h)");
			close(fd_file);
			return EXIT_FAILURE;
		}

		off_t src_offset = i * part_size;
		off_t dest_offset = 0;
		off_t bytes_to_copy = part_size + (i == num_parts - 1 ? last_part_size : 0);

		while (bytes_to_copy > 0) {
			ssize_t to_read = bytes_to_copy < BUFFER_SIZE ? bytes_to_copy : BUFFER_SIZE;
			ssize_t bytes_read = pread(fd_file, buffer, to_read, src_offset);
			if (bytes_read < 0) {
				print_err("pread error (split.h)");
				close(fd_file);
				close(fd_part);
				return EXIT_FAILURE;
			} else if (bytes_read == 0) {
				print_warn("EOF reached before expected data was fully read (split.h)");
			}

			ssize_t bytes_written = pwrite(fd_part, buffer, bytes_read, dest_offset);
			if (bytes_written != bytes_read) {
				print_err("pwrite error (split.h)");
				close(fd_part);
				close(fd_file);
				return EXIT_FAILURE;
			}

			src_offset += bytes_read;
			dest_offset += bytes_written;
			bytes_to_copy -= bytes_read;
		}

		close(fd_part);
	}
	close(fd_file);
	return EXIT_SUCCESS;
}

#endif