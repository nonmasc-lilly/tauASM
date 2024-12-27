#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "lex.h"
#include "utils/debug.h"

bool token_type_has_value(TOKEN_TYPE type) {
        static const bool has_value[TOKEN_TYPE__MAX] = {
                false,  true,  true,
                false, false, false,
                false, false, false,
                false, false, false,
                false, false, false,
                false
        };
        return has_value[type];
}
const char *token_type_repr_string(TOKEN_TYPE type) {
        static const char *repr_strings[TOKEN_TYPE__MAX] = {
                "NULL",                         "<number>",     "<identifier>",
                "org",                          "db",           "dw",
                "dd",                           "dq",           "times",
                "comma `,`",                    "plus `+`",     "minus `-`",
                "position `$`",                 "origin `$$`",  "open parenthesis `(`",
                "close parenthesis `)`"
        };
        return repr_strings[type];
}

void tokens_create(TOKENS *ret) {
        ret->padding    = 1;
        ret->length     = 0;
        ret->value = malloc(ret->padding);
        ret->line  = malloc(ret->padding);
        ret->type  = malloc(ret->padding);
}
void tokens_destroy(TOKENS *ret) {
        uint32_t i;
        for(i = 0; i < ret->length; i++) if(ret->value[i]) free(ret->value[i]);
        free(ret->value);
        free(ret->line);
        free(ret->type);
        ret->padding = 0;
        ret->length  = 0;
}
void tokens_append(TOKENS *ret, TOKEN_TYPE type, uint32_t line, const char *value) {
        ret->length++;
        if(ret->length > ret->padding-1) {
                ret->padding *= 2;
                ret->value = realloc(ret->value, ret->padding * sizeof(*ret->value));
                ret->line  = realloc(ret->line,  ret->padding *  sizeof(*ret->line));
                ret->type  = realloc(ret->type,  ret->padding *  sizeof(*ret->type));
        }
        if(!value || !token_type_has_value(type)) ret->value[ret->length-1] = NULL;
        else {
                ret->value[ret->length-1] = malloc(strlen(value)+1);
                strcpy(ret->value[ret->length-1], value);
        }
        ret->line[ret->length-1] = line;
        ret->type[ret->length-1] = type;
}
void tokens_print(FILE *out, const TOKENS *ret) {
        uint32_t i;
        for(i = 0; i < ret->length; i++) {
                fprintf(out, "%d: %s", i, token_type_repr_string(ret->type[i]));
                if(token_type_has_value(ret->type[i])) {
                        fprintf(out, " {%s}", ret->value[i]);
                }
                fprintf(out, " on line %d\n", ret->line[i]);
        }
}

TOKEN_TYPE token_type_from_char(const char *string, uint32_t *offset) {
        switch(string[*offset]) {
        case ',': return       TOKEN_TYPE_COMMA;
        case '+': return        TOKEN_TYPE_PLUS;
        case '-': return       TOKEN_TYPE_MINUS;
        case '(': return  TOKEN_TYPE_OPEN_PAREN;
        case ')': return TOKEN_TYPE_CLOSE_PAREN;
        case '$': if(string[*offset+1] == '$') return (++*offset, TOKEN_TYPE_ORIGPOS);
                  else return TOKEN_TYPE_CURPOS;
        default:  return TOKEN_TYPE_NULL;
        }
}

TOKEN_TYPE token_type_from_string(const char *string, uint32_t line) {
        uint32_t i;
        char *lower_copy, *eptr;
        if(!*string) return TOKEN_TYPE_NULL;
        lower_copy = malloc(strlen(string)+1);
        strcpy(lower_copy, string);
        for(i = 0; lower_copy[i]; i++) {
                lower_copy[i] = tolower(lower_copy[i]);
        }
        if(!strcmp(lower_copy,   "org"))        return TOKEN_TYPE_ORG;
        if(!strcmp(lower_copy,    "db"))        return TOKEN_TYPE_DB;
        if(!strcmp(lower_copy,    "dw"))        return TOKEN_TYPE_DW;
        if(!strcmp(lower_copy,    "dd"))        return TOKEN_TYPE_DD;
        if(!strcmp(lower_copy,    "dq"))        return TOKEN_TYPE_DQ;
        if(!strcmp(lower_copy, "times"))        return TOKEN_TYPE_TIMES;
        if(!memcmp(lower_copy, "0x", 0x02))     strtol(lower_copy, &eptr, 0x10);
        else                                    strtol(lower_copy, &eptr, 0x0A);
        if(!*eptr) return TOKEN_TYPE_NUMBER;
        if(isalpha(*string)) for(i = 1; string[i]; i++)
                dassert(
                        isalnum(string[i]) || string[i] == '_',
                        ERROR_LEVEL_ERROR,
                        "Invalid token `%s` on line: %d\n",
                        string, line
                );
        return TOKEN_TYPE_IDENTIFIER;
}

void string_lex(TOKENS *ret, const char *string) {
        uint32_t i, line;
        char *buffer;
        TOKEN_TYPE types[2];
        tokens_create(ret);
        buffer = calloc(1,1);
        line = 0;
        for(i = 0; string[i]; i++) {
                if(string[i] == '\n') ++line;
                if(!(isspace(string[i]) || (types[0] = token_type_from_char(string, &i)) ||
                                string[i] == ';')) {
                        buffer = realloc(buffer, strlen(buffer)+2);
                        buffer[strlen(buffer)+1] = 0x00;
                        buffer[strlen(buffer)+0] = string[i];
                        continue;
                }
                if(types[1] = token_type_from_string(buffer, line))
                        tokens_append(ret, types[1], line, buffer);
                *(buffer = realloc(buffer, 1)) = 0;
                if(string[i] == ';') {
                        ++line;
                        for(; string[i] && string[i] != '\n'; i++);
                        if(!string[i]) break;
                        continue;
                }
                if(types[0]) {
                        tokens_append(ret, types[0], line, NULL);
                        types[0] = TOKEN_TYPE_NULL;
                }
        }
        if(types[1] = token_type_from_string(buffer, line))
                tokens_append(ret, types[1], line, buffer);
        free(buffer);
}
