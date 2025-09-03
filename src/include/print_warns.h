#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

void print_info(const char *format, ...);
void print_warn(const char *format, ...);
void print_err(const char *format, ...);
void print_success(const char *format, ...);
void print_failure(const char *format, ...);