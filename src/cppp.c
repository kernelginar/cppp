#include "include/main.h"

int main(int argc, char *argv[]) {
	parser_options cli_options = parse_cli(argc, argv);
	ErrorCode status = init_process(cli_options);
	if (status != ERR_OK) {
		free(cli_options.input_files);
		return status;
	}
	free(cli_options.input_files);
	return EXIT_SUCCESS;
}