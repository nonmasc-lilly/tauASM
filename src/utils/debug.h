#ifndef X__DEBUG_H__X
#define X__DEBUG_H__X
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t ERROR_LEVEL; enum {
        ERROR_LEVEL_ERROR  =0x00,
        ERROR_LEVEL_WARNING=0x01,
        ERROR_LEVEL_INFO   =0x03,
        ERROR_LEVEL_STATUS =0x02
};

void print_with_error_level(ERROR_LEVEL error_level, const char *str);

void eprintf(
        ERROR_LEVEL error_level,
        const char *prefix,
        const char *format,
        ...
);
void evprintf(
        ERROR_LEVEL error_level,
        const char *prefix,
        const char *format,
        va_list args
);
void dassert(
        bool condition,
        ERROR_LEVEL error_level,
        const char *prefix,
        const char *format,
        ...
);
void vdassert(
        bool condition,
        ERROR_LEVEL error_level,
        const char *prefix,
        const char *format,
        va_list args
);
bool nb_dassert(
        bool condition,
        ERROR_LEVEL error_level,
        const char *prefix,
        const char *format,
        ...
);
bool nb_vdassert(
        bool condition,
        ERROR_LEVEL error_level,
        const char *prefix,
        const char *format,
        va_list args
);

#endif
