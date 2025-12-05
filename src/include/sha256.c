#include "sha256.h"
#include "progress_bar.h"

ErrorCode calculate_sha256(const char *file_name, char hash_str[HASH_STR_LEN], parser_options cli_options) {
	file_info file_info = get_file_info(file_name, 1);

	int fd_file = open(file_info.file_name, O_RDONLY);
	if (fd_file == -1) {
		print_err("ERR_SHA_FILE_OPEN: '%s' couldn't be opened", file_info.file_name);
		return ERR_SHA_FILE_OPEN;
	}

	off_t file_size = file_info.file_size;

	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	if (!mdctx) {
		print_err("ERR_SHA_CTX_NEW: EVP_MD_CTX_new failed");
		close(fd_file);
		return ERR_SHA_CTX_NEW;
	}

	if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
		print_err("ERR_SHA_DIGEST_INIT: EVP_DigestInit_ex failed");
		EVP_MD_CTX_free(mdctx);
		close(fd_file);
		return ERR_SHA_DIGEST_INIT;
	}

	unsigned char buffer[BUFFER_SIZE];
	ssize_t bytes_read;
	off_t offset = 0;
	off_t total_written = 0;

	struct timeval start_time, current_time, last_update;

	gettimeofday(&start_time, NULL);
	last_update = start_time;

	print_info("hash calculating: \033[92m%s\033[0m", file_info.file_name);
	while ((bytes_read = pread(fd_file, buffer, BUFFER_SIZE, offset)) > 0) {
		if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1) {
			print_err("ERR_SHA_DIGEST_UPDATE: EVP_DigestUpdate failed");
			EVP_MD_CTX_free(mdctx);
			close(fd_file);
			return ERR_SHA_DIGEST_UPDATE;
		}

		offset += bytes_read;
		total_written += bytes_read;

		gettimeofday(&current_time, NULL);
		float since_last = time_diff(last_update, current_time);

		if (cli_options.verbose_mode) {
			if (since_last >= UPDATE_INTERVAL || offset == file_size) {
				float elapsed = time_diff(start_time, current_time);
				float speed_MBps = (elapsed > 0) ? ((float)offset / (1024 * 1024)) / elapsed : 0.0f;
				float remaining = (file_size - offset) / (1024.0 * 1024.0);
				int eta_seconds = (speed_MBps > 0) ? (int)(remaining / speed_MBps) : 0;
				print_progress(file_size, offset, speed_MBps, eta_seconds, elapsed);
				last_update = current_time;
			}
		}
	}

	if (bytes_read < 0) {
		print_err("ERR_SHA_FILE_READ: '%s' couldn't be read", file_info.file_name);
		EVP_MD_CTX_free(mdctx);
		close(fd_file);
		return ERR_SHA_FILE_READ;
	}

	close(fd_file);

	unsigned char hash[SHA256_DIGEST_LENGTH];
	if (EVP_DigestFinal_ex(mdctx, hash, NULL) != 1) {
		print_err("ERR_SHA_DIGEST_FINAL: EVP_DigestFinal_ex failed");
		EVP_MD_CTX_free(mdctx);
		return ERR_SHA_DIGEST_FINAL;
	}

	EVP_MD_CTX_free(mdctx);

	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		snprintf(hash_str + i * 2, 3, "%02x", hash[i]);
	}

	if (cli_options.verbose_mode) {
		printf("\n");
	}
	return ERR_OK;
}