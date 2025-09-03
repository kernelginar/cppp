#ifndef SELECT_AND_MERGE
#define SELECT_AND_MERGE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <errno.h>
#include "file_info.h"

#define BUFFER_SIZE 4 * 1024

int compare_asc(const void *a, const void *b) {
    off_t val_a = *(const off_t *)a;
    off_t val_b = *(const off_t *)b;

    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

int get_max(int array[], size_t count) {
    if (count == 0) {
        return INT_MIN;
    }

    int max = array[0];
    for (size_t i = 1; i < count; ++i) {
        if (array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

int select_and_merge(const char *file_name, off_t num_parts, int parts[], int count) {
	if (count > num_parts) {
		print_err("The total number of selected parts cannot be more than the total number of parts (select_and_merge.h)");
		return EXIT_FAILURE;
	}

	if (get_max(parts, count) > num_parts) {
		print_err("The value of the selected part cannot be greater than the total number of parts (select_and_merge.h)");
		return EXIT_FAILURE;
	}

	int fd_file = open(file_name, O_RDONLY);
	if (fd_file == -1) {
		print_err("File couldn't be opened (select_and_merge.h)");
		return EXIT_FAILURE;
	}

	file_info main_file_info = get_file_info(file_name, num_parts);

	char new_file_name[256];
	snprintf(new_file_name, sizeof(new_file_name), "%s_new", file_name);
	int fd_new_file = open(new_file_name, O_WRONLY | O_CREAT | O_TRUNC, main_file_info.permissions);
	if (fd_new_file == -1) {
		print_err("New file couldn't be created (select_and_merge.h)");
		close(fd_file);
		return EXIT_FAILURE;
	}

	char buffer[BUFFER_SIZE];
	off_t dest_offset = 0;
	for (size_t i = 0; i < count; i++) {
		off_t src_offset = main_file_info.part_size * (parts[i] - 1);
		off_t bytes_to_copy = main_file_info.part_size + (parts[i] == num_parts - 1 ? main_file_info.last_part_size : 0);

		while (bytes_to_copy > 0) {
			ssize_t to_read = bytes_to_copy < BUFFER_SIZE ? bytes_to_copy : BUFFER_SIZE;
			ssize_t bytes_read = pread(fd_file, buffer, to_read, src_offset);
			if (bytes_read < 0) {
				print_err("pread error (select_and_merge.h)");
				close(fd_new_file);
				close(fd_file);
				return EXIT_FAILURE;
			} else if (bytes_read == 0) {
				print_warn("EOF reached before expected data was fully read (select_and_merge.h)");
			}

			ssize_t bytes_written = pwrite(fd_new_file, buffer, bytes_read, dest_offset);
			if (bytes_written != bytes_read) {
				print_err("pwrite error (select_and_merge.h)");
				close(fd_new_file);
				close(fd_file);
				return EXIT_FAILURE;
			}

			src_offset += bytes_read;
			dest_offset += bytes_written;
			bytes_to_copy -= bytes_read;
		}
	}
	close(fd_file);
	close(fd_new_file);
	return EXIT_SUCCESS;
}

#endif