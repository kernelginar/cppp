#include "progress_bar.h"

void format_eta(int total_seconds, char *buffer, size_t buffer_size) {
	int minutes = total_seconds / 60;
	int seconds = total_seconds % 60;
	snprintf(buffer, buffer_size, "%02d:%02d", minutes, seconds);
}

void format_elapsed(float seconds, char *buffer, size_t buffer_size) {
	int mins = (int)(seconds) / 60;
	int secs = (int)(seconds) % 60;
	snprintf(buffer, buffer_size, "%02d:%02d", mins, secs);
}

float time_diff(struct timeval start, struct timeval end) {
	return (float)(end.tv_sec - start.tv_sec) +
           (float)(end.tv_usec - start.tv_usec) / 1000000.0f;
}

void print_progress(off_t total, off_t current,
                    float speed_MBps, int eta_seconds, float elapsed_seconds) {
	printf("\033[?25l");
	float progress = (float)current / total;
	int filled_chars = (int)(progress * PROGRESS_BAR_WIDTH);
	int dot_pos = filled_chars;
	int complete = (progress >= 1.0f);

	char eta_str[16], elapsed_str[16];
	format_eta(eta_seconds, eta_str, sizeof(eta_str));
	format_elapsed(elapsed_seconds, elapsed_str, sizeof(elapsed_str));

	//printf("\r%-17s[");
	printf("\r[");

	for (int i = 0; i < PROGRESS_BAR_WIDTH; ++i) {
		if (i == dot_pos && !complete)
			printf("\033[92m>");
		else if (i < dot_pos || complete)
			printf("=");
		else
			printf("\033[30m·");
	}

	printf("\033[0m] ");

	int percentage = (int)(progress * 100);
	printf("\033[36m%3d%%\033[0m [%.2f MB/s] [ETA: %s] [Elapsed: %s] ", percentage, speed_MBps, eta_str, elapsed_str);

	fflush(stdout);
	printf("\033[?25h");
}