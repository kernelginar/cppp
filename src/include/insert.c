#include "insert.h"

ErrorCode insert(const char *file_name, off_t num_parts, off_t part_location, const char *new_data_file_name, parser_options cli_options) {
    file_info main_file = get_file_info(file_name, num_parts);
    file_info data_file = get_file_info(new_data_file_name, 1);

	int fd_file = open(main_file.file_name, O_RDWR);
	if (fd_file == -1) {
		print_err("ERR_INSERT_FILE_OPEN: '%s' couldn't be opened", main_file.file_name);
		return ERR_INSERT_FILE_OPEN;
	}

	int fd_data_file = open(data_file.file_name, O_RDONLY);
	if (fd_data_file == -1) {
		print_err("ERR_INSERT_DATA_FILE_OPEN: '%s' data file couldn't be opened", data_file.file_name);
		close(fd_file);
		return ERR_INSERT_DATA_FILE_OPEN;
	}

	char temp_file_name[PATH_MAX];
	snprintf(temp_file_name, sizeof(temp_file_name), "%s_part_XXXXXX", file_name);

	int fd_temp_part_file = mkstemp(temp_file_name);
	file_info temp_part_file = get_file_info(temp_file_name, 1);
	if (fd_temp_part_file == -1) {
		print_err("ERR_INSERT_MKSTEMP: mkstemp failed");
		close(fd_file);
		close(fd_data_file);
		return ERR_INSERT_MKSTEMP;
	}

	if (part_location > num_parts) {
		print_err("ERR_INSERT_INVALID_PART_LOCATION: part_location can not be greater than num_parts");
		close(fd_file);
		close(fd_data_file);
		close(fd_temp_part_file);
		unlink(temp_part_file.file_name);
		return ERR_INSERT_INVALID_PART_LOCATION;
	} else if (part_location == num_parts) {
		merge(main_file.file_name, data_file.file_name, cli_options);
	} else if (part_location < num_parts) {
		off_t src_offset = main_file.part_size * part_location;
		char buffer[BUFFER_SIZE];
		off_t temp_offset = 0;

		ssize_t bytes_read, bytes_written;
		while ((bytes_read = pread(fd_file, buffer, BUFFER_SIZE, src_offset)) > 0) {
			bytes_written = pwrite(fd_temp_part_file, buffer, bytes_read, temp_offset);
			if (bytes_written != bytes_read) {
				print_err("ERR_INSERT_PWRITE: pwrite error");
				close(fd_file);
				close(fd_data_file);
				close(fd_temp_part_file);
				return ERR_INSERT_PWRITE;
			}

			src_offset += bytes_read;
			temp_offset += bytes_written;
		}

		if (bytes_read == -1) {
			print_err("ERR_INSERT_PREAD: pread error");
			close(fd_file);
			close(fd_data_file);
			close(fd_temp_part_file);
			return ERR_INSERT_PREAD;
		}

		if (ftruncate(fd_file, src_offset) == -1) {
			print_err("ERR_INSERT_FTRUNCATE: ftruncate error");
			close(fd_file);
			close(fd_data_file);
			close(fd_temp_part_file);
			return ERR_INSERT_FTRUNCATE;
		}

		merge(main_file.file_name, data_file.file_name, cli_options);
		merge(main_file.file_name, temp_part_file.file_name, cli_options);

		close(fd_file);
		close(fd_data_file);
		close(fd_temp_part_file);
		unlink(temp_part_file.file_name);
	}

	return ERR_OK;
}