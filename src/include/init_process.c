#include "init_process.h"

ErrorCode init_process(parser_options cli_options) {
	if (cli_options.mode == 0) {
		if (cli_options.input_count == 0) {
			print_err("ERR_COPY_NO_INPUT_FILE: Input file wasn't specified");
			return ERR_COPY_NO_INPUT_FILE;
		} else if (strcmp(cli_options.output, "") == 0) {
			print_err("ERR_COPY_NO_OUTPUT_PATH: Output path wasn't specified");
			return ERR_COPY_NO_OUTPUT_PATH;
		}

		int fileCount = 0;
		int directoryCount = 0;

		for (int i = 0; i < cli_options.input_count; i++) {
			file_info src_info = get_file_info(cli_options.input_files[i], 1);
			if (S_ISDIR(src_info.st_mode)) {
				directoryCount++;
			} else if (S_ISREG(src_info.st_mode)) {
				fileCount++;
			}
		}

		for (int i = 0; i < cli_options.input_count; i++) {
			file_info src_info = get_file_info(cli_options.input_files[i], 1);
			file_info dst_info = get_file_info(cli_options.output, 1);

			if (src_info.status == -1) {
				print_err("ERR_COPY_INPUT_FILE_OPEN: '%s' cannot be opened", src_info.file_name);
				return ERR_COPY_INPUT_FILE_OPEN;
			}

			if (S_ISDIR(src_info.st_mode)) {
				char output[PATH_MAX];

				if (dst_info.status == 0) {
					snprintf(output, sizeof(output), "%s/%s", dst_info.file_name, get_only_file_name(src_info.file_name));
					mkdir_p(output, src_info.permissions);
				} else if (dst_info.status == -1) {
					if (fileCount != 0 && directoryCount != 0) {
						snprintf(output, sizeof(output), "%s/%s", dst_info.file_name, get_only_file_name(src_info.file_name));
						mkdir_p(output, src_info.permissions);
					} else if (fileCount == 0 && directoryCount != 0) {
						snprintf(output, sizeof(output), "%s", dst_info.file_name);
						mkdir_p(output, src_info.permissions);
					}
				}

				ErrorCode status = copy_directory(src_info.file_name, output, cli_options);
				if (status == ERR_COPY_DIR_OPEN) {
					print_failure("ERR_COPY_DIR_OPEN: '%s' couldn't be copied", src_info.file_name);
				} else if (status == ERR_COPY_DIR_MKDIR_FAIL) {
					print_failure("ERR_COPY_DIR_MKDIR_FAIL: '%s' couldn't be copied", src_info.file_name);
				} else if (status == ERR_OK) {
					print_success("'%s' copied to '%s'", src_info.file_name, dst_info.file_name);
				}

			} else if (S_ISREG(src_info.st_mode)) {
				if (strcmp(src_info.file_name, dst_info.file_name) == 0) {
					print_err("ERR_COPY_SAME_FILE_INPUT: '%s' and '%s' are the same file", src_info.file_name, dst_info.file_name);
					return ERR_COPY_SAME_FILE_INPUT;
				}

				if (dst_info.status == 0 && S_ISDIR(dst_info.st_mode)) {
					char output[PATH_MAX];
					snprintf(output, sizeof(output), "%s/%s", dst_info.file_name, get_only_file_name(src_info.file_name));
					ErrorCode status = copy(src_info.file_name, output, cli_options.num_parts, cli_options);
					if (status == ERR_COPY_FILE_FULL_FAIL) {
						print_failure("ERR_COPY_FILE_FULL_FAIL: '%s' couldn't be copied", src_info.file_name);
					} else if (status == ERR_COPY_FILE_PART_COPY) {
						print_failure("ERR_COPY_FILE_PART_COPY: '%s' couldn't be copied", src_info.file_name);
					} else if (status == ERR_OK) {
						print_success("'%s' copied to '%s'", src_info.file_name, dst_info.file_name);
					}
				} else if (dst_info.status == -1) {
					ErrorCode status = copy(src_info.file_name, dst_info.file_name, cli_options.num_parts, cli_options);
					if (status == ERR_COPY_FILE_FULL_FAIL) {
						print_failure("ERR_COPY_FILE_FULL_FAIL: '%s' couldn't be copied", src_info.file_name);
					} else if (status == ERR_COPY_FILE_PART_COPY) {
						print_failure("ERR_COPY_FILE_PART_COPY: '%s' couldn't be copied", src_info.file_name);
					} else if (status == ERR_OK) {
						print_success("'%s' copied to '%s'", src_info.file_name, dst_info.file_name);
					}
				} else if (dst_info.status == 0) {
					if (cli_options.overwrite) {
						ErrorCode status = copy(src_info.file_name, dst_info.file_name, cli_options.num_parts, cli_options);
						if (status == ERR_COPY_FILE_FULL_FAIL) {
							print_failure("ERR_COPY_FILE_FULL_FAIL: '%s' couldn't be copied", src_info.file_name);
						} else if (status == ERR_COPY_FILE_PART_COPY) {
							print_failure("ERR_COPY_FILE_PART_COPY: '%s' couldn't be copied", src_info.file_name);
						} else if (status == ERR_OK) {
							print_success("'%s' copied to '%s'", src_info.file_name, dst_info.file_name);
						}
					} else {
						print_err("ERR_COPY_FILE_NOT_ALLOWED: no overwrite permission");
						continue;
					}
				}
			}
		}
	}
	return ERR_OK;
}