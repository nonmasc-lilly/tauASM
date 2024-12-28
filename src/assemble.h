#ifndef X__ASSEMBLE_H__X
#define X__ASSEMBLE_H__X
#include <stdint.h>
#include <stdbool.h>
#include "lex.h"

typedef struct {
        uint8_t *buffer;
        uint32_t length;
} BYTE_BUFFER;

typedef struct {
        struct {
                char *name;
                uint32_t position;
        } *contents;
        uint32_t length;
} LABELS;

typedef struct {
        LABELS          labels;
        BYTE_BUFFER    *ret;
        const TOKENS   *input_tokens;
        uint32_t        token_index;
        uint32_t        curpos;
        uint32_t        origin;
        bool            debug;
} ASM_STATE;

#define INTERNAL_NUMBER_OF_STATEMENT_FUNCTIONS 4
typedef bool (*INTERN_ASM_FUNC)(ASM_STATE *state);
bool internal_assemble_statement(ASM_STATE *state);
bool internal_assemble_org(ASM_STATE *state);
bool internal_assemble_db(ASM_STATE *state);
bool internal_assemble_times(ASM_STATE *state);
bool internal_assemble_label(ASM_STATE *state);

void buffer_pushb(BYTE_BUFFER *ret, uint8_t byte);

void tokens_assemble(BYTE_BUFFER *ret, const TOKENS *input_tokens, bool debug);

#endif
