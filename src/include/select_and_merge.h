#ifndef SELECT_AND_MERGE
#define SELECT_AND_MERGE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <errno.h>
#include "file_info.h"
#include "print_warns.h"
#include "error_codes.h"
#include "buffer_size.h"

int compare_asc(const void *a, const void *b);
int get_max(int array[], size_t count);
ErrorCode select_and_merge(const char *file_name, off_t num_parts, int parts[], int count);

#endif