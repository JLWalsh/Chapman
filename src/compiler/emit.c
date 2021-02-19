#include "emit.h"
#include <string.h>
#include <vm/chapman.h>

uint8_t* ch_emit(ch_token* tokens, uint16_t num_tokens) {
    size_t program_size = sizeof(uint32_t) * 100;
    uint32_t* program = (uint32_t*) malloc(program_size);
    memset(program, 0, program_size);

    uint32_t* current = program;

    for(uint16_t i = 0; i < num_tokens; i++) {
        ch_token token = tokens[i];

        if (token.kind == HALT) {
            *current = OP_HALT;
        }

        current++;
    }

    return program;
}
