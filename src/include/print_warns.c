#include "print_warns.h"

void print_info(const char *format, ...) {
	time_t current_time;
	struct tm *time_info;
	char timestamp[9];

	time(&current_time);
	time_info = localtime(&current_time);

	snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d",
			 time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

	fprintf(stdout, "\033[90m%s\033[0m [\033[94mINFO\033[0m]: ", timestamp);

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	fprintf(stdout, "\n");
}

void print_warn(const char *format, ...) {
	time_t current_time;
	struct tm *time_info;
	char timestamp[9];

	time(&current_time);
	time_info = localtime(&current_time);

	snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d",
			 time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

	fprintf(stdout, "\033[90m%s\033[0m [\033[93mWARNING\033[0m]: ", timestamp);

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	fprintf(stdout, "\n");
}

void print_err(const char *format, ...) {
	time_t current_time;
	struct tm *time_info;
	char timestamp[9];

	time(&current_time);
	time_info = localtime(&current_time);

	snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d",
			 time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

	fprintf(stderr, "\033[90m%s\033[0m [\033[91mERROR\033[0m]: ", timestamp);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	if (errno != 0)
	{
		fprintf(stderr, ": %s", strerror(errno));
	}

	fprintf(stderr, "\n");
}

void print_success(const char *format, ...) {
	time_t current_time;
	struct tm *time_info;
	char timestamp[9];

	time(&current_time);
	time_info = localtime(&current_time);

	snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d",
			 time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

	fprintf(stdout, "\033[90m%s\033[0m [\033[92mSUCCESS\033[0m]: ", timestamp);

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	fprintf(stdout, "\n");
}

void print_failure(const char *format, ...) {
	time_t current_time;
	struct tm *time_info;
	char timestamp[9];

	time(&current_time);
	time_info = localtime(&current_time);

	snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d",
			 time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

	fprintf(stderr, "\033[90m%s\033[0m [\033[31mFAILURE\033[0m]: ", timestamp);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fprintf(stderr, "\n");
}