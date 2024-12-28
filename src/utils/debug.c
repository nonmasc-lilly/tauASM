#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "debug.h"

void print_with_error_level(ERROR_LEVEL error_level, const char *str) {
        switch(error_level) {
        case ERROR_LEVEL_ERROR:
                fprintf(stderr, "\033[1;31m");
                break;
        case ERROR_LEVEL_WARNING:
                fprintf(stderr, "\033[93m");
                break;
        case ERROR_LEVEL_INFO:
                fprintf(stderr, "\033[90m");
                break;
        case ERROR_LEVEL_STATUS:
                fprintf(stderr, "\033[32m");
                break;
        }
        fprintf(stderr, "%s\033[0m", str);
}

void eprintf(
                ERROR_LEVEL error_level,
                const char *prefix,
                const char *format,
                ...) {
        va_list args;
        va_start(args, format);
        evprintf(error_level, prefix, format, args);
        va_end(args);
}
void evprintf(
                ERROR_LEVEL error_level,
                const char *prefix,
                const char *format,
                va_list args) {
        print_with_error_level(error_level, prefix);
        vfprintf(stderr, format, args);
}
void dassert(
                bool condition,
                ERROR_LEVEL error_level,
                const char *prefix,
                const char *format,
                ...) {
        va_list args;
        va_start(args, format);
        vdassert(condition, error_level, prefix, format, args);
        va_end(args);
}
void vdassert(
                bool condition,
                ERROR_LEVEL error_level,
                const char *prefix,
                const char *format,
                va_list args) {
        if(!condition) {
                evprintf(error_level, prefix, format, args);
                exit(1);
        }
}

bool nb_dassert(
                bool condition,
                ERROR_LEVEL error_level,
                const char *prefix,
                const char *format,
                ...) {
        bool ret;
        va_list args;
        va_start(args, format);
        ret = nb_vdassert(condition, error_level, prefix, format, args);
        va_end(args);
        return ret;
}
bool nb_vdassert(
                bool condition,
                ERROR_LEVEL error_level,
                const char *prefix,
                const char *format,
                va_list args) {
        if(!condition) {
                evprintf(error_level, prefix, format, args);
                return false;
        }
        return true;
}
