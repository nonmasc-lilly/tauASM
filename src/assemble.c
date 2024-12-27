#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "lex.h"
#include "utils/debug.h"

bool internal_assemble_statement(ASM_STATE *state) {

}

void tokens_assemble(const TOKENS *input_tokens) {
        ASM_STATE state;
        memset(&state, 0, sizeof(state));
        state.input_tokens = input_tokens;
        while(state.token_index < input_tokens->length) dassert(
                internal_assemble_statement(&state),
                ERROR_LEVEL_ERROR,
                "Assembly-Error: ",
                "Expected statement on line %d.\n",
                input_tokens->line[state.token_index]
        );
}
