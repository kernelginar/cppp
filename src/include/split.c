#include "split.h"

ErrorCode split(const char* file_name, off_t num_parts) {
    file_info file_info = get_file_info(file_name, num_parts);

	int fd_file = open(file_info.file_name, O_RDWR);
	if (fd_file == -1) {
		print_err("ERR_SPLIT_FILE_OPEN: '%s' couldn't be opened", file_info.file_name);
		return ERR_SPLIT_FILE_OPEN;
	}

	char buffer[BUFFER_SIZE];
	off_t part_size = file_info.part_size;
	off_t last_part_size = file_info.last_part_size;

	for (int i = 0; i < num_parts; i++) {
		char part_name[PATH_MAX];
		// part_name (PATH_MAX) -> $(file_name).part$(i + 1) -> example.part1, example.part2
		snprintf(part_name, sizeof(part_name), "%s.part%d", file_info.file_name, i + 1);

		int fd_part = open(part_name, O_WRONLY | O_CREAT | O_TRUNC, file_info.permissions);
		if (fd_part == -1) {
			print_err("ERR_SPLIT_PART_CREATE: '%s' couldn't be created", get_only_file_name(part_name));
			close(fd_file);
			return ERR_SPLIT_PART_CREATE;
		}

		off_t src_offset = i * part_size;
		off_t dest_offset = 0;
		off_t bytes_to_copy = part_size + (i == num_parts - 1 ? last_part_size : 0);

		while (bytes_to_copy > 0) {
			ssize_t to_read = bytes_to_copy < BUFFER_SIZE ? bytes_to_copy : BUFFER_SIZE;
			ssize_t bytes_read = pread(fd_file, buffer, to_read, src_offset);
			if (bytes_read < 0) {
				print_err("ERR_SPLIT_PREAD: pread error");
				close(fd_file);
				close(fd_part);
				return ERR_SPLIT_PREAD;
			} else if (bytes_read == 0) {
				print_warn("EOF reached before expected data was fully read (can be ignored)");
			}

			ssize_t bytes_written = pwrite(fd_part, buffer, bytes_read, dest_offset);
			if (bytes_written != bytes_read) {
				print_err("ERR_SPLIT_PWRITE: pwrite error");
				close(fd_part);
				close(fd_file);
				return ERR_SPLIT_PWRITE;
			}

			src_offset += bytes_read;
			dest_offset += bytes_written;
			bytes_to_copy -= bytes_read;
		}

		close(fd_part);
	}
	close(fd_file);
	return ERR_OK;
}