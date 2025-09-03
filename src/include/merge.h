#ifndef MERGE
#define MERGE

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdlib.h>
#include <sys/time.h>
#include "file_info.h"
#include "progress_bar.h"

#define BUFFER_SIZE 4 * 1024

int merge_files(const char *first_file, const char *second_file) {
	int fd_first_file = open(first_file, O_WRONLY);
	if (fd_first_file == -1) {
		print_err("\"%s\" couldn't be opened", first_file);
		return EXIT_FAILURE;
	}

	int fd_second_file = open(second_file, O_RDONLY);
	if (fd_second_file == -1) {
		print_err("\"%s\" couldn't be opened", second_file);
		close(fd_first_file);
		return EXIT_FAILURE;
	}

	file_info first_file_info = get_file_info(first_file, 1);
	file_info second_file_info = get_file_info(second_file, 1);

	char buffer[BUFFER_SIZE];
	ssize_t bytes_read, bytes_written;
	off_t first_file_offset = first_file_info.file_size;
	off_t second_file_offset = 0;
	off_t total_written = 0;
	off_t total_size = second_file_info.file_size;

	if (first_file_offset == -1 || total_size == -1) {
		print_err("offset error");
		close(fd_first_file);
		close(fd_second_file);
		return EXIT_FAILURE;
	}

	struct timeval start_time, current_time;
	gettimeofday(&start_time, NULL);

	print_info("%s <- %s", first_file, second_file);
	while ((bytes_read = pread(fd_second_file, buffer, BUFFER_SIZE, second_file_offset)) > 0) {
		bytes_written = pwrite(fd_first_file, buffer, bytes_read, first_file_offset);
		if (bytes_written != bytes_read) {
			print_err("error writing to %s file", first_file);
			close(fd_first_file);
			close(fd_second_file);
			return EXIT_FAILURE;
		}

		first_file_offset += bytes_read;
		second_file_offset += bytes_written;
		total_written += bytes_read;

		gettimeofday(&current_time, NULL);
		float elapsed = time_diff(start_time, current_time);
		float speed_MBps = (elapsed > 0) ? (total_written / (1024.0f * 1024.0f)) / elapsed : 0;
		int eta_seconds = (speed_MBps > 0) ? (int)((total_size - total_written) / (speed_MBps * 1024 * 1024)) : 0;

		print_progress(total_size, total_written, speed_MBps, eta_seconds, elapsed);
	}

	printf("\n");

	close(fd_first_file);
	close(fd_second_file);
	return EXIT_SUCCESS;
}

#endif