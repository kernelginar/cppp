#include "copy.h"
#include "cli_parser.h"
#include "error_codes.h"
#include "file_info.h"

ErrorCode copy(const char *src, const char *dst, off_t num_parts, parser_options cli_options) {
	file_info src_info = get_file_info(src, num_parts);
	file_info dst_info = get_file_info(dst, num_parts);

	int fd_src = open(src_info.file_name, O_RDONLY);
	if (fd_src == -1) {
		print_err("ERR_COPY_FILE_OPEN: '%s' couldn't be opened",
				  src_info.file_name);
		return ERR_COPY_FILE_OPEN;
	}

	int fd_dst = open(dst_info.file_name, O_WRONLY | O_CREAT | O_TRUNC, src_info.permissions);
	if (fd_dst == -1) {
		print_err("ERR_COPY_FILE_CREATE: '%s' couldn't be created",
				  dst_info.file_name);
		return ERR_COPY_FILE_CREATE;
	}

	src_info = get_file_info(src, num_parts);
	dst_info = get_file_info(dst, num_parts);

	if (cli_options.verbose_mode) {
		print_info("'%s' -> '%s'", src_info.file_name, dst_info.file_name);
	}

	if (num_parts == 1) {
		ErrorCode status = copy_full(fd_src, fd_dst, src_info, dst_info, cli_options);
		if (status == ERR_COPY_FILE_FULL_FAIL) {
			return ERR_COPY_FILE_FULL_FAIL;
		}
	} else if (num_parts > 1) {
		ErrorCode status = copy_part(fd_src, fd_dst, src_info, dst_info, cli_options);
		if (status == ERR_COPY_FILE_PART_COPY) {
			return ERR_COPY_FILE_PART_COPY;
		}
	} else {
		print_err("ERR_COPY_FILE_NOT_ALLOWED: num_parts cannot be less than 1");
		return ERR_COPY_FILE_NOT_ALLOWED;
	}

	if (cli_options.verbose_mode) {
		print_warn("fsync() executing...");
	}
	fsync(fd_dst);

	close(fd_src);
	close(fd_dst);

	if (cli_options.check_sha256) {
		char src_hash[HASH_STR_LEN];
		char dst_hash[HASH_STR_LEN];

		ErrorCode status_sha1 = calculate_sha256(src_info.file_name, src_hash, cli_options);
		ErrorCode status_sha2 = calculate_sha256(dst_info.file_name, dst_hash, cli_options);

		if (status_sha1 == ERR_OK && status_sha2 == ERR_OK) {
			print_info("\033[92m%s\033[0m %s", src_hash, src_info.file_name);
			print_info("\033[92m%s\033[0m %s", dst_hash, dst_info.file_name);
			(strcmp(src_hash, dst_hash) == 0
				 ? print_success("SHA256 Hash values are matched!")
				 : print_failure("SHA256 Hash mismatch detected!"));
		}
	}

	return ERR_OK;
}

ErrorCode copy_part(int fd_src, int fd_dst, file_info src_info, file_info dst_info, parser_options cli_options) {
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read, bytes_written;

	off_t file_size = src_info.file_size;
	off_t part_size = src_info.part_size;
	off_t last_part_size = src_info.last_part_size;

	off_t src_offset = 0;
	off_t dst_offset = 0;

	struct timeval start_time, current_time, last_update;
	for (int i = 1; i <= src_info.num_parts; i++) {
		off_t total_size = 0;
		if (i == src_info.num_parts) {
			off_t part_written = 0;
			src_offset = (i - 1) * part_size;
			total_size = last_part_size;
			if (cli_options.verbose_mode) {
				printf("> '%s'\n   (%d/%d)\n", dst_info.file_name, i, src_info.num_parts);
			}

			gettimeofday(&start_time, NULL);
			last_update = start_time;

			while (src_offset < file_size && (bytes_read = pread(fd_src, buffer, (file_size - src_offset) < BUFFER_SIZE ? (file_size - src_offset) : BUFFER_SIZE, src_offset)) > 0) {
				bytes_written = pwrite(fd_dst, buffer, bytes_read, dst_offset);
				if (bytes_written != bytes_read) {
					print_err("ERR_COPY_FILE_PART_COPY: '%s' to '%s': an error encountered while copying '%d' part", get_only_file_name(src_info.file_name), get_only_file_name(dst_info.file_name), i);
					close(fd_src);
					close(fd_dst);
					return ERR_COPY_FILE_PART_COPY;
				}

				src_offset += bytes_read;
				dst_offset += bytes_written;
				part_written += bytes_read;

				gettimeofday(&current_time, NULL);
				float since_last = time_diff(last_update, current_time);

				if (cli_options.verbose_mode) {
					if (since_last >= UPDATE_INTERVAL || part_written == total_size) {
						float elapsed = time_diff(start_time, current_time);
						float speed_MBps = (elapsed > 0) ? (part_written / (1024.0f * 1024.0f)) / elapsed : 0;
						int eta_seconds = (speed_MBps > 0) ? (int)(total_size - part_written) / (speed_MBps * 1024 * 1024) : 0;
						print_progress(total_size, part_written, speed_MBps, eta_seconds, elapsed);
						last_update = current_time;
					}
				}
			}
		} else {
			off_t part_written = 0;
			src_offset = (i - 1) * part_size;
			total_size = part_size;
			if (cli_options.verbose_mode) {
				printf("> '%s'\n   (%d/%d)\n", dst_info.file_name, i, src_info.num_parts);
			}

			gettimeofday(&start_time, NULL);
			last_update = start_time;
			while (src_offset < i * part_size && (bytes_read = pread(fd_src, buffer, ((i * part_size) - src_offset) < BUFFER_SIZE ? ((i * part_size) - src_offset) : BUFFER_SIZE, src_offset)) > 0) {
				bytes_written = pwrite(fd_dst, buffer, bytes_read, dst_offset);
				if (bytes_written != bytes_read) {
					print_err("ERR_COPY_FILE_PART_COPY: '%s' to '%s': an error encountered while copying '%d' part",
							  get_only_file_name(src_info.file_name),
							  get_only_file_name(dst_info.file_name), i);
					close(fd_src);
					close(fd_dst);
					return ERR_COPY_FILE_PART_COPY;
				}

				src_offset += bytes_read;
				dst_offset += bytes_written;
				part_written += bytes_read;

				gettimeofday(&current_time, NULL);
				float since_last = time_diff(last_update, current_time);

				if (cli_options.verbose_mode) {
					if (since_last >= UPDATE_INTERVAL || part_written == total_size) {
						float elapsed = time_diff(start_time, current_time);
						float speed_MBps = (elapsed > 0) ? (part_written / (1024.0f * 1024.0f)) / elapsed : 0;
						int eta_seconds = (speed_MBps > 0) ? (int)(total_size - part_written) / (speed_MBps * 1024 * 1024) : 0;
						print_progress(total_size, part_written, speed_MBps, eta_seconds, elapsed);
						last_update = current_time;
					}
				}
			}
		}

		if (cli_options.verbose_mode) {
			printf("\n");
		}
	}

	return ERR_OK;
}

ErrorCode copy_full(int fd_src, int fd_dst, file_info src_info, file_info dst_info, parser_options cli_options) {
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read, bytes_written;
	off_t total_written = 0;
	off_t src_offset = 0;
	off_t dst_offset = 0;
	off_t total_size = src_info.file_size;

	struct timeval start_time, current_time, last_update;

	gettimeofday(&start_time, NULL);
	last_update = start_time;
	while ((bytes_read = pread(fd_src, buffer, BUFFER_SIZE, src_offset)) > 0) {
		bytes_written = pwrite(fd_dst, buffer, bytes_read, dst_offset);
		if (bytes_written != bytes_read) {
			print_err("ERR_COPY_FILE_FULL_FAIL: error encountered while '%s'copying to '%s'", src_info.file_name, dst_info.file_name);
			close(fd_src);
			close(fd_dst);
			return ERR_COPY_FILE_FULL_FAIL;
		}

		src_offset += bytes_read;
		dst_offset += bytes_written;
		total_written += bytes_read;

		gettimeofday(&current_time, NULL);
		float since_last = time_diff(last_update, current_time);
		if (cli_options.verbose_mode) {
			if (since_last >= UPDATE_INTERVAL || total_written == total_size) {
				float elapsed = time_diff(start_time, current_time);
				float speed_MBps = (elapsed > 0) ? (total_written / (1024.0f * 1024.0f)) / elapsed : 0;
				int eta_seconds = (speed_MBps > 0) ? (int)(total_size - total_written) / (speed_MBps * 1024 * 1024) : 0;
				print_progress(total_size, total_written, speed_MBps, eta_seconds, elapsed);
				last_update = current_time;
			}
		}
	}

	if (cli_options.verbose_mode) {
		printf("\n");
	}

	return ERR_OK;
}

ErrorCode copy_directory(const char *src, const char *dst, parser_options cli_options) {
	file_info src_info = get_file_info(src, 1);

	if (!S_ISDIR(src_info.st_mode)) {
		print_err("ERR_COPY_DIR_SRC_NOT_DIR: Source is not a directory");
		return ERR_COPY_DIR_SRC_PATH_INFO;
	}

	if (mkdir_p(dst, src_info.permissions) != EXIT_SUCCESS) {
		if (errno != EEXIST) {
			print_err("ERR_COPY_DIR_MKDIR_FAIL: mkdir_p error");
			return ERR_COPY_DIR_MKDIR_FAIL;
		}
	}

	DIR *dir;
	struct dirent *entry;
	char src_path[PATH_MAX];
	char dst_path[PATH_MAX];

	dir = opendir(src);
	if (!dir) {
		print_err("ERR_COPY_DIR_OPEN: opendir error");
		return ERR_COPY_DIR_OPEN;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		snprintf(src_path, PATH_MAX, "%s/%s", src, entry->d_name);
		snprintf(dst_path, PATH_MAX, "%s/%s", dst, entry->d_name);

		file_info src_info = get_file_info(src_path, cli_options.num_parts);
		file_info dst_info = get_file_info(dst_path, cli_options.num_parts);

		if (src_info.status == 0 && dst_info.status == 0) {
			if (src_info.st_ino == dst_info.st_ino && src_info.st_dev == dst_info.st_dev) {
				continue;
			}
		}

		file_info current_src_info = get_file_info(src_path, 1);

		if (current_src_info.status == -1) {
			print_warn("Skipping unreadable file: %s", src_path);
			continue;
		}

		if (S_ISDIR(current_src_info.st_mode)) {
			if (copy_directory(src_path, dst_path, cli_options) != ERR_OK) {
				print_err("ERR_COPY_DIR_RECURSIVE_FAIL: error in %s", src_path);
			}
		} else if (S_ISREG(current_src_info.st_mode)) {
			ErrorCode status = copy(src_path, dst_path, cli_options.num_parts, cli_options);
			if (status != ERR_OK) {
				print_warn("Failed to copy file: %s", src_path);
			}
		} else {
			print_warn("Skipped: '%s' is not supported type", src_path);
		}
	}

	closedir(dir);
	return ERR_OK;
}

ErrorCode mkdir_p(const char *path, mode_t mode) {
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;
	struct stat st;

	if (!path || *path == '\0') {
		errno = EINVAL;
		return ERR_UNKNOWN;
	}

	snprintf(tmp, sizeof(tmp), "%s", path);
	len = strlen(tmp);
	if (len == 0 || len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return ERR_UNKNOWN;
	}

	if (tmp[len - 1] == '/')
		tmp[len - 1] = '\0';

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			if (mkdir(tmp, mode) != 0) {
				if (errno == EEXIST) {
					if (stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)) {
						errno = ENOTDIR;
						return ERR_UNKNOWN;
					}
				} else {
					return ERR_UNKNOWN;
				}
			}
			*p = '/';
		}
	}

	if (mkdir(tmp, mode) != 0) {
		if (errno == EEXIST) {
			if (stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)) {
				errno = ENOTDIR;
				return ERR_UNKNOWN;
			}
		} else {
			return ERR_UNKNOWN;
		}
	}

	return ERR_OK;
}