#ifndef X__LEX_H__X
#define X__LEX_H__X
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t TOKEN_TYPE; enum {
        TOKEN_TYPE_NULL         = 0x00,
        TOKEN_TYPE_NUMBER       = 0x01,
        TOKEN_TYPE_IDENTIFIER   = 0x02,
        TOKEN_TYPE_ORG          = 0x03,
        TOKEN_TYPE_DB           = 0x04,
        TOKEN_TYPE_DW           = 0x05,
        TOKEN_TYPE_DD           = 0x06,
        TOKEN_TYPE_DQ           = 0x07,
        TOKEN_TYPE_TIMES        = 0x08,
        TOKEN_TYPE_COMMA        = 0x09,
        TOKEN_TYPE_PLUS         = 0x0A,
        TOKEN_TYPE_MINUS        = 0x0B,
        TOKEN_TYPE_CURPOS       = 0x0C,
        TOKEN_TYPE_ORIGPOS      = 0x0D,

        TOKEN_TYPE__MAX
};

typedef struct {
        char           **value;
        uint32_t        *line;
        TOKEN_TYPE      *type;
        uint32_t         length;
        uint32_t         padding;
} TOKENS;

bool token_type_has_value(TOKEN_TYPE type);

void tokens_create(TOKENS *ret);
void tokens_append(TOKENS *ret, TOKEN_TYPE type, uint32_t line, const char *value);
void tokens_print(FILE *out, const TOKENS *ret);
void tokens_destroy(TOKENS *ret);

TOKEN_TYPE token_type_from_char(const char *string, uint32_t *offset);
TOKEN_TYPE token_type_from_string(const char *string, uint32_t line);

void string_lex(TOKENS *ret, const char *string);

#endif
