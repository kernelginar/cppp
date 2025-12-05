#ifndef SPLIT
#define SPLIT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "file_info.h"
#include "print_warns.h"
#include "buffer_size.h"
#include "error_codes.h"

ErrorCode split(const char* file_name, off_t num_parts);

#endif