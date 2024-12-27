#ifndef X__ASSEMBLE_H__X
#define X__ASSEMBLE_H__X
#include <stdint.h>
#include <stdbool.h>
#include "lex.h"

typedef struct {
        const TOKENS   *input_tokens;
        uint32_t        token_index;
        uint32_t        curpos;
        uint32_t        origin;
} ASM_STATE;

#define INTERNAL_NUMBER_OF_STATEMENT_FUNCTIONS 3
bool internal_assemble_statement(ASM_STATE *state);
bool internal_assemble_org(ASM_STATE *state);
bool internal_assemble_db(ASM_STATE *state);
bool internal_assemble_times(ASM_STATE *state);

void tokens_assemble(const TOKENS *input_tokens);

#endif
