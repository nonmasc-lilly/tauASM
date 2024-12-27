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

bool internal_assemble_statement(ASM_STATE *state) {
        uint32_t i;
        INTERN_ASM_FUNC functions[INTERNAL_NUMBER_OF_STATEMENT_FUNCTIONS] = {
                internal_assemble_org,
                internal_assemble_db,
                internal_assemble_times
        };
        for(i = 0; i < INTERNAL_NUMBER_OF_STATEMENT_FUNCTIONS; i++)
                if(functions[i](state)) return true;
        return false;
}
bool internal_assemble_org(ASM_STATE *state) {
        int32_t address;
        /* <org> <number: val> */
        if(state->input_tokens->type[state->token_index] != TOKEN_TYPE_ORG) return false;
        ++state->token_index;
        DASSERT(
                state->input_tokens->type[state->token_index] == TOKEN_TYPE_NUMBER,
                "Expected number after `org` on line %d.\n",
                state->input_tokens->line[state->token_index-1]
        );
        address = strtol(state->input_tokens->value[state->token_index], NULL, 0x00);
        ++state->token_index;
        DASSERT(
                address < 0x100000 && address > -0x100000,
                "Incorrect bounds on address for `org` on line %d.\n",
                state->input_tokens->line[state->token_index-1]
        );
        state->origin = *(uint32_t*)(&address);
        state->curpos = *(uint32_t*)(&address);
        return true;
}
bool internal_assemble_db(ASM_STATE *state) {
        /* <db> <number: val> [*(<,>  <number: val>)] */
        /* TODO: string literals */
        /* TODO: numbers > 0xFF */
        int64_t value;
        uint64_t corrected_value;
        if(state->input_tokens->type[state->token_index] != TOKEN_TYPE_DB) return false;
        ++state->token_index;
        DASSERT(
                state->input_tokens->type[state->token_index] ==  TOKEN_TYPE_NUMBER,
                "Expected number after `db` on line %d.\n",
                state->input_tokens->line[state->token_index-1]
        );
        value = strtol(state->input_tokens->value[state->token_index], NULL, 0x00);
        buffer_pushb(state->ret, *(uint8_t*)(&value));
        state->curpos++;
        ++state->token_index;
        while(state->token_index+1 < state->input_tokens->length &&
                        state->input_tokens->type[state->token_index] == TOKEN_TYPE_COMMA) {
                ++state->token_index;
                DASSERT(
                        state->input_tokens->type[state->token_index] ==  TOKEN_TYPE_NUMBER,
                        "Expected number in `db` argument list on line %d.\n",
                        state->input_tokens->line[state->token_index-1]
                );
                value = strtol(state->input_tokens->value[state->token_index], NULL, 0x00);
                buffer_pushb(state->ret, *(uint8_t*)(&value));
                state->curpos++;
                ++state->token_index;
        }
        return true;
}
bool internal_assemble_times(ASM_STATE *state) {
        bool expr = true;
        int64_t uncorrected_value;
        uint32_t *bracket_stack;
        uint64_t *stack;
        uint64_t i;
        uint32_t stack_size, bracket_length, original;
        if(state->input_tokens->type[state->token_index] != TOKEN_TYPE_TIMES) return false;
        stack = malloc(1);
        stack_size = 0;
        bracket_stack = malloc(1);
        bracket_length = 0;
        state->token_index++;
        while(expr) {
                switch(state->input_tokens->type[state->token_index]) {
                case TOKEN_TYPE_OPEN_PAREN:
                        bracket_stack = realloc(
                                bracket_stack,
                                ++bracket_length * sizeof(*bracket_stack)
                        );
                        bracket_stack[bracket_length-1] =
                                state->input_tokens->line[state->token_index];
                        ++state->token_index;
                        break;
                case TOKEN_TYPE_CLOSE_PAREN:
                        DASSERT(
                                bracket_length > 0,
                                "Unmatched close parenthesis `)` in `times` statement on "
                                        "line %d.\n",
                                state->input_tokens->line[state->token_index]
                        );
                        if(--bracket_length != 0)
                                bracket_stack = realloc(
                                        bracket_stack,
                                        --bracket_length * sizeof(*bracket_stack)
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
        if(bracket_length) DASSERT(
                false,
                "Unmatched open parenthesis `(` in times expression on line %d.\n",
                bracket_stack[bracket_length-1]
        );
        DASSERT(
                stack_size == 1,
                "Extra items in times expression on line %d.\n",
                state->input_tokens->line[state->token_index]
        );
        original = state->token_index;
        for(i = 0; i < *stack; i++) {
                state->token_index = original;
                DASSERT(
                        internal_assemble_statement(state),
                        "Expected statement in times expression on line %d.\n",
                        state->input_tokens->line[original]
                );
        }
        return true;
}

void buffer_pushb(BYTE_BUFFER *ret, uint8_t byte) {
        ret->buffer = realloc(ret->buffer, ++ret->length);
        ret->buffer[ret->length - 1] = byte;
}

void tokens_assemble(BYTE_BUFFER *ret, const TOKENS *input_tokens) {
        ASM_STATE state;
        memset(ret, 0, sizeof(*ret));
        ret->buffer = malloc(1);
        memset(&state, 0, sizeof(state));
        state.input_tokens = input_tokens;
        state.ret = ret;
        while(state.token_index < input_tokens->length) DASSERT(
                internal_assemble_statement(&state),
                "Expected statement on line %d.\n",
                input_tokens->line[state.token_index]
        );
}
