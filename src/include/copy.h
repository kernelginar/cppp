#ifndef COPY_FILE
#define COPY_FILE

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/time.h>
#include <dirent.h>
#include "file_info.h"
#include "error_codes.h"
#include "progress_bar.h"
#include "buffer_size.h"
#include "cli_parser.h"
#include "sha256.h"

ErrorCode copy(const char *src, const char *dst, off_t num_parts, parser_options cli_options);
ErrorCode copy_part(int fd_src, int fd_dst, file_info src_info, file_info dst_info, parser_options cli_options);
ErrorCode copy_full(int fd_src, int fd_dst, file_info src_info, file_info dst_info, parser_options cli_options);
ErrorCode copy_directory(const char *src, const char *dst, parser_options cli_options);
ErrorCode mkdir_p(const char *dst, mode_t permissions);
ErrorCode init_process(parser_options cli_options);

#endif