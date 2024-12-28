#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "lex.h"
#include "assemble.h"
#include "utils/debug.h"

static void DASSERT(bool cond, const char *format, ...) {
        va_list args;
        va_start(args, format);
        vdassert(cond, ERROR_LEVEL_ERROR, "Assembly-Error: ", format, args);
        va_end(args);
}

/* 64 bit conversions */
#define I64_U64(identifier) *(uint64_t*)(&identifier)
#define I64_U32(identifier) *(uint32_t*)(&identifier)
#define I64_U16(identifier) *(uint16_t*)(&identifier)
#define  I64_U8(identifier)  *(uint8_t*)(&identifier)
#define U64_I64(identifier)  *(int64_t*)(&identifier)
#define U64_I32(identifier)  *(int32_t*)(&identifier)
#define U64_I16(identifier)  *(int16_t*)(&identifier)
#define  U64_I8(identifier)   *(int8_t*)(&identifier)
/* 32 bit conversions */
#define I32_U32(identifier) *(uint32_t*)(&identifier)
#define I32_U16(identifier) *(uint16_t*)(&identifier)
#define  I32_U8(identifier)  *(uint8_t*)(&identifier)
#define U32_I32(identifier)  *(int32_t*)(&identifier)
#define U32_I16(identifier)  *(int16_t*)(&identifier)
#define  U32_I8(identifier)   *(int8_t*)(&identifier)
/* 16 bit conversions */
#define I16_U16(identifier)  *(uint16_t)(&identifier)
#define  I16_U8(identifier)   *(uint8_t)(&identifier)
#define U16_I16(identifier)   *(int16_t)(&identifier)
#define  U16_I8(identifier)    *(int8_t)(&identifier)
/*  8 bit conversions */
#define I8_U8(identifier)     *(uint8_t)(&identifier)
#define U8_I8(identifier)     *(uint8_t)(&identifier)

static TOKEN_TYPE get_token_type(ASM_STATE *state) {
        return state->input_tokens->type[state->token_index];
}
static TOKEN_TYPE peek_token_type(ASM_STATE *state, int32_t offset) {
        return state->input_tokens->type[state->token_index+offset];
}
static const char  *get_token_value(ASM_STATE *state) {
        return state->input_tokens->value[state->token_index];
}
static const char *peek_token_value(ASM_STATE *state, int32_t offset) {
        return state->input_tokens->value[state->token_index+offset];
}
static uint32_t  get_token_line(ASM_STATE *state) {
        return state->input_tokens->line[state->token_index];
}
static uint32_t peek_token_line(ASM_STATE *state, int32_t offset) {
        return state->input_tokens->line[state->token_index+offset];
}
static void consume_token(ASM_STATE *state) {
        ++state->token_index;
}
static bool token_in_bounds(ASM_STATE *state) {
        return state->token_index < state->input_tokens->length;
}
static bool peeked_token_in_bounds(ASM_STATE *state, int32_t offset) {
        return state->token_index+offset < state->input_tokens->length;
}
static void inc_cursor(ASM_STATE *state) {
        state->curpos++;
}
static void advance_cursor(ASM_STATE *state, uint32_t offset) {
        state->curpos+=offset;
}

static uint64_t evaluate_times_expression(ASM_STATE *state) {
        uint64_t *stack;
        uint32_t *bracket_stack;
        uint64_t ret;
        int64_t  uncorrected_value;
        uint32_t stack_size, bracket_size;
        bool expr = true;
        stack           = malloc(1);
        bracket_stack   = malloc(1);
        stack_size      = 0;
        bracket_size    = 0;
        while(expr) {
                switch(state->input_tokens->type[state->token_index]) {
                case TOKEN_TYPE_OPEN_PAREN:
                        bracket_stack = realloc(
                                bracket_stack,
                                ++bracket_size * sizeof(*bracket_stack)
                        );
                        bracket_stack[bracket_size-1] =
                                state->input_tokens->line[state->token_index];
                        ++state->token_index;
                        break;
                case TOKEN_TYPE_CLOSE_PAREN:
                        DASSERT(
                                bracket_size > 0,
                                "Unmatched close parenthesis `)` in `times` statement on "
                                        "line %d.\n",
                                state->input_tokens->line[state->token_index]
                        );
                        if(--bracket_size != 0)
                                bracket_stack = realloc(
                                        bracket_stack,
                                        --bracket_size * sizeof(*bracket_stack)
                                );
                        ++state->token_index;
                        break;
                case TOKEN_TYPE_PLUS:
                        DASSERT(
                                stack_size >= 2,
                                "Not enough input to `+` operator in `times` statement on "
                                        "line %d.\n",
                                state->input_tokens->line[state->token_index]
                        );
                        stack[stack_size-2] = stack[stack_size-1]+stack[stack_size-2];
                        stack = realloc(stack, --stack_size * sizeof(*stack));
                        ++state->token_index;
                        break;
                case TOKEN_TYPE_MINUS:
                        DASSERT(
                                stack_size >= 2,
                                "Not enough input to `-` operator in `times` statement on "
                                        "line %d.\n",
                                state->input_tokens->line[state->token_index]
                        );
                        stack[stack_size-2] = stack[stack_size-1]-stack[stack_size-2];
                        stack = realloc(stack, --stack_size * sizeof(*stack));
                        ++state->token_index;
                        break;
                case TOKEN_TYPE_NUMBER:
                        stack = realloc(stack, ++stack_size * sizeof(*stack));
                        uncorrected_value = strtol(
                                state->input_tokens->value[state->token_index],
                                NULL,
                                0x00
                        );
                        stack[stack_size-1] = *(uint64_t*)(&uncorrected_value);
                        state->token_index++;
                        break;
                case TOKEN_TYPE_ORIGPOS:
                        stack = realloc(stack, ++stack_size * sizeof(*stack));
                        stack[stack_size-1] = state->origin;
                        ++state->token_index;
                        break;
                case TOKEN_TYPE_CURPOS:
                        stack = realloc(stack, ++stack_size * sizeof(*stack));
                        stack[stack_size-1] = state->curpos;
                        ++state->token_index;
                        break;
                default: expr = false; break;
                }
        }
        if(bracket_size) DASSERT(
                false,
                "Unmatched open parenthesis `(` in times expression on line %d.\n",
                bracket_stack[bracket_size-1]
        );
        DASSERT(
                stack_size == 1,
                "Extra items in times expression on line %d.\n",
                state->input_tokens->line[state->token_index]
        );
        ret = *stack;
        free(stack);
        free(bracket_stack);
        return ret;
}

#define INTERNAL_STATEMENT_HANDLED 4
bool internal_assemble_statement(ASM_STATE *state) {
        uint32_t i;
        static bool warned_uneq_hndl_act = false;
        INTERN_ASM_FUNC functions[INTERNAL_NUMBER_OF_STATEMENT_FUNCTIONS] = {
                internal_assemble_org,
                internal_assemble_db,
                internal_assemble_times,
                internal_assemble_label
        };
        warned_uneq_hndl_act = warned_uneq_hndl_act || !nb_dassert(
                INTERNAL_STATEMENT_HANDLED == INTERNAL_NUMBER_OF_STATEMENT_FUNCTIONS,
                ERROR_LEVEL_WARNING, "ASSEMBLER-ERROR: ",
                "Number of handled statements is not the same as the number of statements\n"
                "\tin %s on line %d.\n", __FILE__, __LINE__
        );
        for(i = 0; i < INTERNAL_NUMBER_OF_STATEMENT_FUNCTIONS; i++)
                if(functions[i](state)) return true;
        return false;
}
bool internal_assemble_org(ASM_STATE *state) {
        int32_t address;
        /* <org> <number: val> */
        if(get_token_type(state) != TOKEN_TYPE_ORG) return false;
        consume_token(state);
        DASSERT(
                get_token_type(state) == TOKEN_TYPE_NUMBER,
                "Expected number after `org` on line %d.\n",
                peek_token_line(state, -1)
        );
        address = strtol(get_token_value(state), NULL, 0x00);
        consume_token(state);
        DASSERT(
                address < 0x100000 && address > -0x100000,
                "Incorrect bounds on address for `org` on line %d.\n",
                peek_token_line(state, -1)
        );
        state->origin = I32_U32(address);
        state->curpos = I32_U32(address);
        return true;
}
bool internal_assemble_db(ASM_STATE *state) {
        /* <db> <number: val> [*(<,>  <number: val>)] */
        /* TODO: string literals */
        /* TODO: numbers > 0xFF */
        int64_t value;
        uint64_t corrected_value;
        if(get_token_type(state) != TOKEN_TYPE_DB) return false;
        consume_token(state);
        DASSERT(
                get_token_type(state) == TOKEN_TYPE_NUMBER,
                "Expected number after `db` on line %d.\n",
                peek_token_line(state, -1)
        );
        value = strtol(get_token_value(state), NULL, 0x00);
        buffer_pushb(state->ret, I64_U8(value));
        inc_cursor(state);
        consume_token(state);
        while(peeked_token_in_bounds(state, 1) && get_token_type(state) == TOKEN_TYPE_COMMA) {
                consume_token(state);
                DASSERT(
                        get_token_type(state) == TOKEN_TYPE_NUMBER,
                        "Expected number after `db` on line %d.\n",
                        peek_token_line(state, -1)
                );
                value = strtol(get_token_value(state), NULL, 0x00);
                buffer_pushb(state->ret, I64_U8(value));
                inc_cursor(state);
                consume_token(state);
        }
        return true;
}
bool internal_assemble_times(ASM_STATE *state) {
        uint64_t i;
        uint32_t original;
        uint64_t expression_value;
        if(get_token_type(state) != TOKEN_TYPE_TIMES) return false;
        consume_token(state);
        expression_value = evaluate_times_expression(state);
        original = state->token_index;
        for(i = 0; i < expression_value; i++) {
                state->token_index = original;
                DASSERT(
                        internal_assemble_statement(state),
                        "Expected statement in times expression on line %d.\n",
                        state->input_tokens->line[original]
                );
        }
        return true;
}
bool internal_assemble_label(ASM_STATE *state) {
        uint32_t i;
        if( get_token_type(state) != TOKEN_TYPE_IDENTIFIER ||
           peek_token_type(state,1) != TOKEN_TYPE_COLON) return false;
        for(i = 0; i < state->labels.length; i++) DASSERT(
                !!strcmp(get_token_value(state), state->labels.contents[i].name),
                "Redefinition of label %s on line %d.\n",
                get_token_value(state),
                get_token_line(state)
        );
        state->labels.contents = realloc(
                state->labels.contents,
                ++state->labels.length * sizeof(*state->labels.contents)
        );
        state->labels.contents[state->labels.length-1].name =
                malloc(strlen(get_token_value(state))+1);
        strcpy(
                state->labels.contents[state->labels.length-1].name,
                get_token_value(state)
        );
        state->labels.contents[state->labels.length-1].position = state->curpos;
        consume_token(state);
        consume_token(state);
        return true;
}

void buffer_pushb(BYTE_BUFFER *ret, uint8_t byte) {
        ret->buffer = realloc(ret->buffer, ++ret->length);
        ret->buffer[ret->length - 1] = byte;
}

void tokens_assemble(BYTE_BUFFER *ret, const TOKENS *input_tokens, bool debug) {
        uint32_t i;
        ASM_STATE state;

        memset(ret, 0, sizeof(*ret));
        ret->buffer = malloc(1);

        memset(&state, 0, sizeof(state));
        state.input_tokens = input_tokens;
        state.ret = ret;
        state.labels.contents = malloc(1);
        state.debug = debug;

        while(state.token_index < input_tokens->length) DASSERT(
                internal_assemble_statement(&state),
                "Expected statement on line %d.\n",
                input_tokens->line[state.token_index]
        );
        
        if(state.debug) {
                print_with_error_level(ERROR_LEVEL_INFO, "Debug (Labels):\n");
                for(i = 0; i < state.labels.length; i++) {
                        fprintf(
                                stderr,
                                "`%s` at: 0x%0X\n",
                                state.labels.contents[i].name,
                                state.labels.contents[i].position
                        );
                }
        }
        for(i = 0; i < state.labels.length; i++) free(state.labels.contents[i].name);
        free(state.labels.contents);
}
