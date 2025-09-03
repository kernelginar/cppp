#ifndef SHA256_CALCULATE
#define SHA256_CALCULATE

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "progress_bar.h"
#include "file_info.h"

#define BUFFER_SIZE 4 * 1024
#define HASH_STR_LEN (SHA256_DIGEST_LENGTH * 2 + 1)

int calculate_sha256(const char *file_name, char hash_str[HASH_STR_LEN]) {
	int fd_file = open(file_name, O_RDONLY);
	if (fd_file == -1) {
		print_err("File couldn't be opened (sha256.h)");
		return EXIT_FAILURE;
	}

	file_info file_info = get_file_info(file_name, 1);
	off_t file_size = file_info.file_size;

	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	if (!mdctx) {
		print_err("EVP_MD_CTX_new failed (sha256.h)");
		close(fd_file);
		return EXIT_FAILURE;
	}

	if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
		print_err("EVP_DigestInit_ex failed (sha256.h)");
		EVP_MD_CTX_free(mdctx);
		close(fd_file);
		return EXIT_FAILURE;
	}

	unsigned char buffer[BUFFER_SIZE];
	ssize_t bytes_read;
	off_t offset = 0;

	struct timeval start_time, current_time;
	gettimeofday(&start_time, NULL);

	char *resolved_path = realpath(file_name, NULL);
	print_info("SHA256 Hash value calculating for: \033[92m%s\033[0m", resolved_path);
	while ((bytes_read = pread(fd_file, buffer, BUFFER_SIZE, offset)) > 0) {
		if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1)
		{
			print_err("EVP_DigestUpdate failed (sha256.h)");
			EVP_MD_CTX_free(mdctx);
			close(fd_file);
			return EXIT_FAILURE;
		}

		offset += bytes_read;

		gettimeofday(&current_time, NULL);
		float elapsed = time_diff(start_time, current_time);
		float speed_MBps = (elapsed > 0) ? ((float)offset / (1024 * 1024)) / elapsed : 0.0f;
		float remaining = (file_size - offset) / (1024.0 * 1024.0);
		int eta_seconds = (speed_MBps > 0) ? (int)(remaining / speed_MBps) : 0;

		print_progress(file_size, offset, speed_MBps, eta_seconds, elapsed);
	}

	if (bytes_read < 0) {
		print_err("File couldn't be read (sha256.h)");
		EVP_MD_CTX_free(mdctx);
		close(fd_file);
		return EXIT_FAILURE;
	}

	close(fd_file);

	unsigned char hash[SHA256_DIGEST_LENGTH];
	if (EVP_DigestFinal_ex(mdctx, hash, NULL) != 1) {
		print_err("EVP_DigestFinal_ex failed (sha256.h)");
		EVP_MD_CTX_free(mdctx);
		return EXIT_FAILURE;
	}

	EVP_MD_CTX_free(mdctx);

	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		snprintf(hash_str + i * 2, 3, "%02x", hash[i]);
	}

	free(resolved_path);
	printf("\n");
	return EXIT_SUCCESS;
}

#endif