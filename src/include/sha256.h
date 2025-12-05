#ifndef SHA256_CALCULATE
#define SHA256_CALCULATE

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "cli_parser.h"
#include "progress_bar.h"
#include "file_info.h"
#include "buffer_size.h"
#include "error_codes.h"

#define HASH_STR_LEN (SHA256_DIGEST_LENGTH * 2 + 1)

ErrorCode calculate_sha256(const char *file_name, char hash_str[HASH_STR_LEN], parser_options cli_options);

#endif