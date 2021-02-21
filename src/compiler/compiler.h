#pragma once
#include "token.h"
#include "emit.h"
#include <vm/chapman.h>
#include <stdint.h>

typedef struct {
    ch_token_state token_state;
    ch_token previous;
    ch_token current;

    ch_emit emit;
} ch_compilation;

ch_program ch_compile(const uint8_t* program, size_t program_size);