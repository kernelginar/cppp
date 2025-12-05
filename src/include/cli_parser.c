#include "cli_parser.h"

void print_help_message() {
	printf("Usage: cppp [OPTIONS]\n\n");
	printf("Options:\n");
	printf("  -m, --mode           Specify the operation mode. (required)\n");
	printf("  -i, --input <path>   Source file or directory path. (required)\n");
	printf("  -o, --output <path>  Destination file or directory path. (required)\n");
	printf("  -p, --parts          Specify the number of parts for copying.\n");
	printf("  -c, --check-sha256   Enable verifying SHA-256 checksum after copying.\n");
	printf("  -h, --help           Display this help message.\n");
	printf("  -v, --verbose        Enable verbose logging.\n");
	printf("  -f --force           Enable overwriting on existing file.\n");
	printf("  -V, --version        Show program version information.\n\n");
}

int compare(const void *a, const void *b) {
	return (*(int *)a - *(int *)b);
}

parser_options parse_cli(int argc, char *argv[]) {
	int option;
	int option_index = 0;

	char *endptr;

	parser_options parser_options;
	parser_options.mode = 0;
	parser_options.input_files = malloc(argc * sizeof(char *));
	parser_options.input_count = 0;
	parser_options.num_parts = 1;
	parser_options.output[0] = '\0';
	parser_options.verbose_mode = false;
	parser_options.check_sha256 = false;
	parser_options.overwrite = false;

	const char *short_options = "m:i:o:p:hcfvV";

	static struct option long_options[] = {
		/*
			copy (default) = 0
			insert = 1
			merge = 2
			replace = 3
			select_and_merge = 4
			split = 5
		*/
		{"mode", required_argument, 0, 'm'},
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"parts", required_argument, 0, 'p'},
		{"help", no_argument, 0, 'h'},
		{"check-sha256", no_argument, 0, 'c'},
		{"verbose", no_argument, 0, 'v'},
		{"force", no_argument, 0, 'f'},
		{"version", no_argument, 0, 'V'},
		{0, 0, 0, 0}};

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
					parser_options.input_files[parser_options.input_count++] = argv[optind++ - 1];
				}
				parser_options.input_files[parser_options.input_count++] = argv[optind - 1];
				break;
			case 'o':
				strncpy(parser_options.output, optarg, PATH_MAX - 1);
				parser_options.output[PATH_MAX - 1] = '\0';
				break;
			case 'p':
				errno = 0;
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
				parser_options.check_sha256 = true;
				break;
			case 'v':
				parser_options.verbose_mode = true;
				break;
			case 'f':
				parser_options.overwrite = true;
				break;
			case 'h':
				print_help_message();
				exit(EXIT_SUCCESS);
			case '?':
				exit(EXIT_FAILURE);
			default:
				print_help_message();
				exit(EXIT_SUCCESS);
		}
	}

	int positional_args_count = 0;
	int first_positional_index = optind;

	while (optind < argc) {
		positional_args_count++;
		optind++;
	}

	if (strlen(parser_options.output) == 0) {
		if (positional_args_count > 0) {
			char *last_arg = argv[first_positional_index + positional_args_count - 1];
			strncpy(parser_options.output, last_arg, PATH_MAX - 1);
			parser_options.output[PATH_MAX - 1] = '\0';
			positional_args_count--;
		}
	}

	for (int i = 0; i < positional_args_count; i++) {
		parser_options.input_files[parser_options.input_count++] = argv[first_positional_index + i];
	}

	return parser_options;
}