#ifndef INSERT
#define INSERT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "cli_parser.h"
#include "file_info.h"
#include "merge.h"
#include "buffer_size.h"
#include "error_codes.h"

ErrorCode insert(const char *file_name, off_t num_parts, off_t part_location, const char *new_data_file_name, parser_options cli_options);

#endif