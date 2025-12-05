#include "select_and_merge.h"
#include "error_codes.h"
#include "file_info.h"

int compare_asc(const void *a, const void *b) {
    off_t val_a = *(const off_t *)a;
    off_t val_b = *(const off_t *)b;

    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

int get_max(int array[], size_t count) {
    if (count == 0) return INT_MIN;

    int max = array[0];
    for (size_t i = 1; i < count; ++i) {
        if (array[i] > max)
            max = array[i];
    }
    return max;
}

ErrorCode select_and_merge(const char *file_name, off_t num_parts, int parts[], int count) {
	if (count > num_parts) {
		print_err("ERR_SELECT_TOO_MANY_PARTS: selected parts (%d) exceed total available parts (%d)", count, num_parts);
		return ERR_SELECT_TOO_MANY_PARTS;
	}

	if (get_max(parts, count) > num_parts) {
		print_err("ERR_SELECT_INVALID_PART_VALUE: selected part index's exceed valid range [1..%d]", num_parts);
		return ERR_SELECT_INVALID_PART_VALUE;
	}

	file_info main_file = get_file_info(file_name, num_parts);
	int fd_file = open(main_file.file_name, O_RDONLY);
	if (fd_file == -1) {
		print_err("ERR_SELECT_FILE_OPEN: '%s' couldn't be opened", main_file.file_name);
		return ERR_SELECT_FILE_OPEN;
	}

	char new_file_name[PATH_MAX];
	snprintf(new_file_name, sizeof(new_file_name), "%s_new", file_name);

	file_info new_file = get_file_info(new_file_name, count);
	int fd_new_file = open(new_file.file_name, O_WRONLY | O_CREAT | O_TRUNC, main_file.permissions);
	if (fd_new_file == -1) {
		print_err("ERR_SELECT_FILE_CREATE: '%s' couldn't be created", new_file.file_name);
		close(fd_file);
		return ERR_SELECT_FILE_CREATE;
	}

	char buffer[BUFFER_SIZE];
	off_t dest_offset = 0;
	for (size_t i = 0; i < count; i++) {
		off_t src_offset = main_file.part_size * (parts[i] - 1);
		off_t bytes_to_copy = main_file.part_size + (parts[i] == num_parts - 1 ? main_file.last_part_size : 0);

		while (bytes_to_copy > 0) {
			ssize_t to_read = bytes_to_copy < BUFFER_SIZE ? bytes_to_copy : BUFFER_SIZE;
			ssize_t bytes_read = pread(fd_file, buffer, to_read, src_offset);
			if (bytes_read < 0) {
				print_err("ERR_SELECT_PREAD: pread error");
				close(fd_new_file);
				close(fd_file);
				return ERR_SELECT_PREAD;
			} else if (bytes_read == 0) {
				print_warn("EOF reached before expected data was fully read (can be ignored)");
			}

			ssize_t bytes_written = pwrite(fd_new_file, buffer, bytes_read, dest_offset);
			if (bytes_written != bytes_read) {
				print_err("ERR_SELECT_PWRITE: pwrite error");
				close(fd_new_file);
				close(fd_file);
				return ERR_SELECT_PWRITE;
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