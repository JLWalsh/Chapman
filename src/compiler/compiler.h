#pragma once
#include "token.h"
#include "emit.h"
#include <vm/chapman.h>
#include <stdint.h>

typedef struct {
    ch_lexeme name;
    uint8_t scope_id;
} ch_local;

typedef struct ch_scope {
    ch_local locals[UINT8_MAX];

    uint8_t locals_size;
} ch_scope;

typedef struct {
    ch_token_state token_state;
    ch_token previous;
    ch_token current;

    ch_emit emit;
    ch_scope scope;

    bool has_errors;
} ch_compilation;

bool ch_compile(const uint8_t* program, size_t program_size, ch_program* output);