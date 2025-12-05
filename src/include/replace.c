#include "replace.h"

ErrorCode replace(const char *file_name, off_t num_parts, off_t position, const char *new_data_file_name, parser_options cli_options) {
	if (position > num_parts || position <= 0) {
		print_err("ERR_REPLACE_INVALID_POSITION: 1 >= position <= num_parts");
		return ERR_REPLACE_INVALID_POSITION;
	}

	file_info main_file = get_file_info(file_name, num_parts);
	file_info data_file = get_file_info(new_data_file_name, 1);

	int fd_file = open(main_file.file_name, O_WRONLY);
	if (fd_file == -1) {
		print_err("ERR_REPLACE_FILE_OPEN: '%s' couldn't be opened", main_file.file_name);
		return ERR_REPLACE_FILE_OPEN;
	}

	int fd_data_file = open(data_file.file_name, O_RDONLY);
	if (fd_data_file == -1) {
		print_err("ERR_REPLACE_DATA_FILE_OPEN: '%s' couldn't be opened", data_file.file_name);
		close(fd_file);
		return ERR_REPLACE_DATA_FILE_OPEN;
	}

	off_t main_offset = main_file.part_size * (position - 1);
	off_t write_size;

	if ((main_file.file_size - main_offset) == main_file.last_part_size) {
		write_size = main_file.last_part_size;
	} else {
		write_size = main_file.part_size;
	}

	char buffer[BUFFER_SIZE];
	off_t data_offset = 0;

	ssize_t bytes_read, bytes_written;
	while ((bytes_read = pread(fd_data_file, buffer, BUFFER_SIZE, data_offset)) > 0 && bytes_read <= write_size) {
		if (main_offset == main_file.part_size * (position)) {
			break;
		}

		bytes_written = pwrite(fd_file, buffer, bytes_read, main_offset);
		if (bytes_written != bytes_read) {
			print_err("ERR_REPLACE_PWRITE: pwrite error");
			close(fd_file);
			close(fd_data_file);
			return ERR_REPLACE_PWRITE;
		}

		main_offset += bytes_read;
		data_offset += bytes_written;
	}

	if (bytes_read == -1) {
		print_err("ERR_REPLACE_PREAD: pread error");
		close(fd_file);
		close(fd_data_file);
		return ERR_REPLACE_PREAD;
	}

	close(fd_file);
	close(fd_data_file);
	return ERR_OK;
}