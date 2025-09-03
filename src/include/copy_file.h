#ifndef COPY_FILE
#define COPY_FILE

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dirent.h>
#include "file_info.h"
#include "progress_bar.h"

#define BUFFER_SIZE 4 * 1024

int copy_file(const char *src_file, const char *dest_file, int num_parts) {
	int fd_src = open(src_file, O_RDONLY);
	if (fd_src == -1) {
		print_err("File couldn't be opened (copy_file.h)");
		return EXIT_FAILURE;
	}
	file_info src_info = get_file_info(src_file, num_parts);

	int fd_dest = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, src_info.permissions);
	if (fd_dest == -1) {
		print_err("File couldn't be created (copy_file.h)");
		close(fd_src);
		return EXIT_FAILURE;
	}

	char buffer[BUFFER_SIZE];
	ssize_t bytes_read, bytes_written;
	off_t file_size = src_info.file_size;
	off_t part_size = src_info.part_size;
	off_t last_part_size = src_info.last_part_size;

	struct timeval start_time, current_time;

	print_info("%s -> %s", src_file, dest_file);
	if (num_parts != 1) {
		off_t src_offset = 0;
		off_t dest_offset = 0;
		for (int i = 1; i <= num_parts; i++) {
			off_t total_size = 0;
			if (i == num_parts) {
				off_t part_written = 0;
				src_offset = (i - 1) * part_size;
				total_size = last_part_size;
				printf("> %s (%d/%d)\n", dest_file, i, num_parts);

				gettimeofday(&start_time, NULL);
				while (src_offset < file_size && (bytes_read = pread(fd_src, buffer, (file_size - src_offset) < BUFFER_SIZE ? (file_size - src_offset) : BUFFER_SIZE, src_offset)) > 0) {
					bytes_written = pwrite(fd_dest, buffer, bytes_read, dest_offset);
					if (bytes_written != bytes_read) {
						print_err("Error copying \"%s\" to \"%s\": an error encountered while copying \"%d\" part (copy_file.h)", src_file, dest_file, i);
						close(fd_src);
						close(fd_dest);
						return EXIT_FAILURE;
					}

					src_offset += bytes_read;
					dest_offset += bytes_written;
					part_written += bytes_read;

					gettimeofday(&current_time, NULL);
					float elapsed = time_diff(start_time, current_time);
					float speed_MBps = (elapsed > 0) ? (part_written / (1024.0f * 1024.0f)) / elapsed : 0;
					int eta_seconds = (speed_MBps > 0) ? (int)(total_size - part_written) / (speed_MBps * 1024 * 1024) : 0;
					print_progress(total_size, part_written, speed_MBps, eta_seconds, elapsed);
				}
			} else {
				off_t part_written = 0;
				src_offset = (i - 1) * part_size;
				total_size = part_size;
				printf("> %s (%d/%d)\n", dest_file, i, num_parts);

				gettimeofday(&start_time, NULL);
				while (src_offset < i * part_size && (bytes_read = pread(fd_src, buffer, ((i * part_size) - src_offset) < BUFFER_SIZE ? ((i * part_size) - src_offset) : BUFFER_SIZE, src_offset)) > 0) {
					bytes_written = pwrite(fd_dest, buffer, bytes_read, dest_offset);
					if (bytes_written != bytes_read) {
						print_err("Error copying \"%s\" to \"%s\": an error encountered while copying \"%d\" part (copy_file.h)", src_file, dest_file, i);
						close(fd_src);
						close(fd_dest);
						return EXIT_FAILURE;
					}

					src_offset += bytes_read;
					dest_offset += bytes_written;
					part_written += bytes_read;

					gettimeofday(&current_time, NULL);
					float elapsed = time_diff(start_time, current_time);
					float speed_MBps = (elapsed > 0) ? (part_written / (1024.0f * 1024.0f)) / elapsed : 0;
					int eta_seconds = (speed_MBps > 0) ? (int)(total_size - part_written) / (speed_MBps * 1024 * 1024) : 0;
					print_progress(total_size, part_written, speed_MBps, eta_seconds, elapsed);
				}
			}
			printf("\n");
		}
	} else if (num_parts == 1) {
		off_t total_written = 0;
		off_t src_offset = 0;
		off_t dest_offset = 0;
		gettimeofday(&start_time, NULL);
		while ((bytes_read = pread(fd_src, buffer, BUFFER_SIZE, src_offset)) > 0) {
			bytes_written = pwrite(fd_dest, buffer, bytes_read, dest_offset);
			if (bytes_written != bytes_read) {
				print_err("Error copying \"%s\" to \"%s\" (copy_file.h)", src_file, dest_file);
				close(fd_src);
				close(fd_dest);
				return EXIT_FAILURE;
			}

			src_offset += bytes_read;
			dest_offset += bytes_written;
			total_written += bytes_read;

			gettimeofday(&current_time, NULL);
			float elapsed = time_diff(start_time, current_time);
			float speed_MBps = (elapsed > 0) ? (total_written / (1024.0f * 1024.0f)) / elapsed : 0;
			int eta_seconds = (speed_MBps > 0) ? (int)(file_size - total_written) / (speed_MBps * 1024 * 1024) : 0;
			print_progress(file_size, total_written, speed_MBps, eta_seconds, elapsed);
		}
		printf("\n");
	}

	print_warn("fsync() executed... (copy_file.h)");
	fsync(fd_dest);
	print_warn("fsync() executed... (copy_file.h)");
	fsync(fd_dest);
	print_warn("fsync() executed... (copy_file.h)");
	fsync(fd_dest);

	close(fd_src);
	close(fd_dest);
	return EXIT_SUCCESS;
}

int mkdir_p(const char *path, mode_t mode) {
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", path);
	len = strlen(tmp);
	if (len == 0)
		return -1;

	if (tmp[len - 1] == '/')
		tmp[len - 1] = '\0';

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			if (mkdir(tmp, mode) != 0 && errno != EEXIST)
				return -1;
			*p = '/';
		}
	}

	if (mkdir(tmp, mode) != 0 && errno != EEXIST)
		return -1;

	return 0;
}

int copy_directory(const char *src_dir, const char *dest_dir, int num_parts) {
	DIR *dir = opendir(src_dir);
	file_info src_dir_info = get_file_info(src_dir, 1);
	if (!dir) {
		print_err("Cannot open directory: %s", src_dir);
		closedir(dir);
		return EXIT_FAILURE;
	}
	mkdir_p(dest_dir, src_dir_info.permissions);

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		char src_path[PATH_MAX];
		char dest_path[PATH_MAX];
		snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
		snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

		struct stat st;
		if (stat(src_path, &st) == -1) {
			print_err("stat failed for %s", src_path);
			closedir(dir);
			continue;
		}

		if (S_ISDIR(st.st_mode)) {
			if (copy_directory(src_path, dest_path, num_parts) != 0) {
				print_err("Cannot copy directory");
				closedir(dir);
				return EXIT_FAILURE;
			}
		} else if (S_ISREG(st.st_mode)) {
			char src_hash[HASH_STR_LEN];
			char dest_hash[HASH_STR_LEN];
			if (calculate_sha256(src_path, src_hash) != 0) {
				print_err("Source file's SHA256 hash value couldn't be calculated (copy_file.h)");
				closedir(dir);
				return EXIT_FAILURE;
			}
			if (copy_file(src_path, dest_path, num_parts) != 0) {
				print_err("Cannot copy file");
				closedir(dir);
				return EXIT_FAILURE;
			}
			if (calculate_sha256(dest_path, dest_hash) != 0) {
				print_err("Destination file's SHA256 hash value couldn't be calculated (copy_file.h)");
				closedir(dir);
				return EXIT_FAILURE;
			}
			print_info("\033[92m%s\033[0m %s", src_hash, get_only_file_name(src_path));
			print_info("\033[92m%s\033[0m %s", dest_hash, get_only_file_name(dest_path));
			(strcmp(src_hash, dest_hash) == 0 ? print_success("SHA256 Hash values are matched!") : print_failure("SHA256 Hash mismatch detected!"));
		}
	}

    closedir(dir);
    return EXIT_SUCCESS;
}


int start_copy(parser_options cli_options) {
	struct stat st_input;
	struct stat st_output;
	if (cli_options.mode == 0) {
		if (cli_options.input_count == 0) {
			print_err("No file was given. At least give one file with -i flag (copy_file.h)");
			free(cli_options.input_files);
			return EXIT_FAILURE;
		} else if (cli_options.output == NULL) {
			print_err("No output path was given (copy_file.h)");
			free(cli_options.input_files);
			return EXIT_FAILURE;
		}

		for (int i = 0; i < cli_options.input_count; i++) {
			int status_input = stat(cli_options.input_files[i], &st_input);
			int status_output = stat(cli_options.output, &st_output);

			if (status_input == -1) {
				print_err("Input file couldn't be opened");
				free(cli_options.input_files);
				return EXIT_FAILURE;
			}

			file_info src_info = get_file_info(cli_options.input_files[i], 1);
			file_info dest_info = get_file_info(cli_options.output, 1);
			char dest_path[PATH_MAX];

			if (S_ISDIR(st_input.st_mode)) {
				if (status_output == 0) {
					snprintf(dest_path, sizeof(dest_path), "%s/%s", cli_options.output, get_only_file_name(cli_options.input_files[i]));
					mkdir_p(dest_path, src_info.permissions);
				} else if (status_output == -1) {
					snprintf(dest_path, sizeof(dest_path), "%s", cli_options.output, get_only_file_name(cli_options.input_files[i]));
					mkdir_p(dest_path, src_info.permissions);
				}
				if (copy_directory(cli_options.input_files[i], dest_path, cli_options.num_parts) != 0) {
					print_err("Cannot copy directory (copy_file.h)");
					free(cli_options.input_files);
					return EXIT_FAILURE;
				}
			} else if (S_ISREG(st_input.st_mode)) {
				char src_hash[HASH_STR_LEN];
				char dest_hash[HASH_STR_LEN];

				if (calculate_sha256(cli_options.input_files[i], src_hash) != 0) {
					print_err("Source file's SHA256 hash value couldn't be calculated (copy_file.h)");
					free(cli_options.input_files);
					return EXIT_FAILURE;
				}

				if (strcmp(cli_options.output, "./") == 0) {
					print_err("\'%s\' and \'./%s\' are the same file (copy_file.h)", cli_options.input_files[i], cli_options.input_files[i]);
					free(cli_options.input_files);
					return EXIT_FAILURE;
				} else if (strcmp(cli_options.output, cli_options.input_files[i]) == 0) {
					print_err("\'%s\' and \'./%s\' are the same file (copy_file.h)", cli_options.output, cli_options.input_files[i]);
					free(cli_options.input_files);
					return EXIT_FAILURE;
				}

				char dest_file[PATH_MAX];
				if (S_ISREG(st_input.st_mode) && S_ISDIR(st_output.st_mode)) {
					snprintf(dest_file, sizeof(dest_file), "%s/%s", cli_options.output, cli_options.input_files[i]);
				}

				if (copy_file(cli_options.input_files[i], dest_file, cli_options.num_parts) != 0) {
					print_err("\'%s\' can not be copied to \'%s\' (copy_file.h)", cli_options.input_files[i], cli_options.output);
					free(cli_options.input_files);
					return EXIT_FAILURE;
				}

				if (calculate_sha256(dest_file, dest_hash) != 0) {
					print_err("Destination file's SHA256 hash value couldn't be calculated (copy_file.h)");
					free(cli_options.input_files);
					return EXIT_FAILURE;
				}

				print_info("\033[92m%s\033[0m %s", src_hash, get_only_file_name(cli_options.input_files[i]));
				print_info("\033[92m%s\033[0m %s", dest_hash, get_only_file_name(dest_file));
				(strcmp(src_hash, dest_hash) == 0 ? print_success("SHA256 Hash values are matched!") : print_failure("SHA256 Hash mismatch detected!"));
			}

			/*
			if (S_ISDIR(st.st_mode)) {
				print_warn("Folder detected, skipping SHA256 calculation...");
			} else if (calculate_sha256(cli_options.input_files[i], src_hash) != 0) {
				print_err("Source file's SHA256 hash value couldn't be calculated (copy_file.h)");
				return EXIT_FAILURE;
			}

			if (strcmp(cli_options.output, "./") == 0) {
				print_err("\'%s\' and \'./%s\' are the same file (copy_file.h)", cli_options.input_files[i], cli_options.input_files[i]);
				return EXIT_FAILURE;
			} else if (strcmp(cli_options.output, cli_options.input_files[i]) == 0) {
				print_err("\'%s\' and \'./%s\' are the same file (copy_file.h)", cli_options.output, cli_options.input_files[i]);
				return EXIT_FAILURE;
			}

			char dest_file[255];
			if (S_ISDIR(st.st_mode)) {
				snprintf(dest_file, sizeof(dest_file), "%s/%s", cli_options.output, cli_options.input_files[i]);
			} else if (S_ISREG(st.st_mode)) {
				snprintf(dest_file, sizeof(dest_file), "%s", cli_options.output);
			}

			if (copy_file(cli_options.input_files[i], dest_file, cli_options.num_parts) != 0) {
				print_err("\'%s\' can not be copied to \'%s\' (copy_file.h)", cli_options.input_files[i], cli_options.output);
				return EXIT_FAILURE;
			}

			if (S_ISDIR(st.st_mode)) {
				print_warn("Folder detected, skipping SHA256 calculation...");
			} else if (calculate_sha256(dest_file, dest_hash) != 0) {
				print_err("Destination file's SHA256 hash value couldn't be calculated (copy_file.h)");
				return EXIT_FAILURE;
			}

			print_info("\033[92m%s\033[0m %s", src_hash, get_only_file_name(cli_options.input_files[i]));
			print_info("\033[92m%s\033[0m %s", dest_hash, get_only_file_name(dest_file));
			(strcmp(src_hash, dest_hash) == 0 ? print_success("SHA256 Hash values are matched!") : print_failure("SHA256 Hash mismatch detected!"));
			*/
		}
	}
	free(cli_options.input_files);
}

#endif
