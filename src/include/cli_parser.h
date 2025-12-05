#ifndef CLI_PARSER
#define CLI_PARSER
#define __GNU_SOURCE

#include "file_info.h"
#include "print_warns.h"
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CPPP_VERSION "cppp 1.0.0"

typedef struct {
		int mode;
		char **input_files;
		int input_count;
		unsigned int num_parts;
		char output[PATH_MAX];
		bool verbose_mode;
		bool check_sha256;
		bool overwrite;
} parser_options;

void print_help_message();
int compare(const void *a, const void *b);
parser_options parse_cli(int argc, char *argv[]);

#endif