#ifndef CLI_PARSER
#define CLI_PARSER

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// It defines maximum selectable part number.
// For this value, you can only select 128 part on file, nothing more.
// You can change it if you want to.
// #define DEFAULT_SELECT_ARRAY_LENGTH 128
#define CPPP_VERSION "cppp 1.0.0"

void print_help_message() {
    printf("Usage: cppp [OPTIONS]\n\n");
    printf("Options:\n");
    printf("  -m, --mode           Specify the operation mode. (required)\n");
    printf("  -i, --input          Source file or directory path. (required)\n");
    printf("  -o, --output         Destination file or directory path. (required)\n");
    printf("  -p, --parts          Specify the number of parts for copying.\n");
    printf("      --insert-after   Insert data after the specified position.\n");
    printf("      --select         Select specific parts for \"select-merge\" mode.\n");
    printf("  -c, --disable-sha256 Disable verifying SHA-256 checksum after copying.\n");
    printf("  -h, --help           Display this help message.\n");
    printf("  -V, --version        Show program version information.\n\n");
    printf("Notes:\n");
    printf("- 'mode', 'input', and 'output' parameters are required.\n");
    printf("- '--input', and '--output' parameters are required.\n");
}

typedef struct {
	int mode;
	char **input_files;
	int input_count;
	int num_parts;
	const char *output;
	bool verbose_mode;
	bool check_sha256;
} parser_options;

int compare(const void *a, const void *b) {
	return (*(int *)a - *(int *)b);
}

parser_options parse_cli(int argc, char *argv[]) {
	int option;
	int option_index = 0;

	char *endptr;
	int errno;

	parser_options parser_options = {
		.mode = 0,
		.input_files = malloc(argc * sizeof(char *)),
		.input_count = 0,
		.num_parts = 1,
		.output = NULL,
		.verbose_mode = true,
		.check_sha256 = true
	};

	const char *short_options = "m:i:o:p:hcV";

	static struct option long_options[] = {
		/*
			default = 0
			split = 1
			merge = 2
			insert = 3
			select-merge = 4
		*/
		{"mode", 			required_argument,		 0, 'm'},
		{"input", 			required_argument,		 0, 'i'},
		{"output", 			required_argument,		 0, 'o'},
		{"parts",			required_argument,		 0, 'p'},
		{"help", 			no_argument, 			 0, 'h'},
		{"disable-sha256", 	no_argument, 			 0, 'c'},
		{"version",			no_argument, 			 0, 'V'},
		{0,					0,						 0,  0 }
	};

	while ((option = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (option) {
			case 'm':
				if (strcmp(optarg, "split") == 0) {
					parser_options.mode = 1;
				} else if (strcmp(optarg, "merge") == 0) {
					parser_options.mode = 2;
				} else if (strcmp(optarg, "insert") == 0) {
					parser_options.mode = 3;
				} else if (strcmp(optarg, "select-merge") == 0) {
					parser_options.mode = 4;
				}
				break;
			case 'i':
				while (optind < argc && argv[optind][0] != '-') {
					parser_options.input_files[parser_options.input_count++] = argv[optind++ -1];
				}
				parser_options.input_files[parser_options.input_count++] = argv[optind-1];
				break;
			case 'o':
				if (strcmp(optarg, ".") == 0) {
					parser_options.output = "./";
				} else if (strcmp(optarg, "..") == 0) {
					parser_options.output = "../";
				} else {
					parser_options.output = optarg;
				}
				break;
			case 'p':
				parser_options.num_parts = strtol(optarg, &endptr, 10);
				if (errno == ERANGE) {
					print_err("Number is out of range. Please provide valid number (cli_parser.h)");
					exit(EXIT_FAILURE);
				} else if (endptr == optarg) {
					print_err("Not a valid number. Please provide valid number (cli_parser.h)");
					exit(EXIT_FAILURE);
				} else {
					if (parser_options.num_parts <= 0) {
						print_err("You can not split a file to 0 or less pieces. Please provide valid number (cli_parser.h)");
						exit(EXIT_FAILURE);
					}
				}
				break;
			case 'c':
				parser_options.check_sha256 = false;
				break;
			case 'h':
				print_help_message();
				exit(EXIT_FAILURE);
			case '?':
				print_err("Invalid argument.");
				exit(EXIT_FAILURE);
			default:
				print_help_message();
				exit(EXIT_FAILURE);
		}
	}
	return parser_options;
}

#endif
