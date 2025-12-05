#ifndef MERGE
#define MERGE

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdlib.h>
#include <sys/time.h>
#include "cli_parser.h"
#include "file_info.h"
#include "progress_bar.h"
#include "buffer_size.h"
#include "error_codes.h"
#include "file_info.h"

ErrorCode merge(const char *first_file_name, const char *second_file_name, parser_options cli_options);

#endif