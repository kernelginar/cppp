#ifndef PROGRESS_BAR
#define PROGRESS_BAR

#include <sys/time.h>
#include "file_info.h"
#include "print_warns.h"
#define PROGRESS_BAR_WIDTH 60
#define UPDATE_INTERVAL 0.5f

void format_eta(int total_seconds, char *buffer, size_t buffer_size);
void format_elapsed(float seconds, char *buffer, size_t buffer_size);
float time_diff(struct timeval start, struct timeval end);
void print_progress(off_t total, off_t current, float speed_MBps, int eta_seconds, float elapsed_seconds);

#endif