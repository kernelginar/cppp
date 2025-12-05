#ifndef REPLACE
#define REPLACE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "file_info.h"
#include "buffer_size.h"
#include "print_warns.h"
#include "error_codes.h"
#include "cli_parser.h"

ErrorCode replace(const char *file_name, off_t num_parts, off_t position, const char *new_data_file_name, parser_options cli_options);

#endif