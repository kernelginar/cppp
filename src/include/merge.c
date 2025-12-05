#include "merge.h"

ErrorCode merge(const char *first_file_name, const char *second_file_name, parser_options cli_options) {
    file_info first_file = get_file_info(first_file_name, 1);
    file_info second_file = get_file_info(second_file_name, 1);

	int fd_first_file = open(first_file.file_name, O_WRONLY);
	if (fd_first_file == -1) {
		print_err("ERR_MERGE_FIRST_FILE_OPEN: '%s\' couldn't be opened", first_file.file_name);
		return ERR_MERGE_FIRST_FILE_OPEN;
	}

	int fd_second_file = open(second_file.file_name, O_RDONLY);
	if (fd_second_file == -1) {
		print_err("ERR_MERGE_SECOND_FILE_OPEN: '%s' couldn't be opened", second_file.file_name);
		close(fd_first_file);
		return ERR_MERGE_SECOND_FILE_OPEN;
	}

	char buffer[BUFFER_SIZE];
	ssize_t bytes_read, bytes_written;
	off_t first_file_offset = first_file.file_size;
	off_t second_file_offset = 0;
	off_t total_written = 0;
	off_t total_size = second_file.file_size;

	if (first_file_offset == -1 || total_size == -1) {
		print_err("ERR_MERGE_OFFSET: offset error");
		close(fd_first_file);
		close(fd_second_file);
		return ERR_MERGE_OFFSET;
	}

	struct timeval start_time, current_time;
	gettimeofday(&start_time, NULL);

	print_info("%s <- %s", first_file.file_name, second_file.file_name);
	while ((bytes_read = pread(fd_second_file, buffer, BUFFER_SIZE, second_file_offset)) > 0) {
		bytes_written = pwrite(fd_first_file, buffer, bytes_read, first_file_offset);
		if (bytes_written != bytes_read) {
			print_err("ERR_MERGE_WRITE: error writing to '%s'", first_file.file_name);
			close(fd_first_file);
			close(fd_second_file);
			return ERR_MERGE_WRITE;
		}

		first_file_offset += bytes_read;
		second_file_offset += bytes_written;
		total_written += bytes_read;

		gettimeofday(&current_time, NULL);
		if (cli_options.verbose_mode) {
		    float elapsed = time_diff(start_time, current_time);
			float speed_MBps = (elapsed > 0) ? (total_written / (1024.0f * 1024.0f)) / elapsed : 0;
			int eta_seconds = (speed_MBps > 0) ? (int)((total_size - total_written) / (speed_MBps * 1024 * 1024)) : 0;
			print_progress(total_size, total_written, speed_MBps, eta_seconds, elapsed);
		}
	}

	if (cli_options.verbose_mode) {
	    printf("\n");
	}

	close(fd_first_file);
	close(fd_second_file);
	return ERR_OK;
}