#include "include/file_info.h"
#include "include/main.h"

int main(int argc, char *argv[]) {
	parser_options cli_options = parse_cli(argc, argv);
	start_copy(cli_options);
	return EXIT_SUCCESS;
}