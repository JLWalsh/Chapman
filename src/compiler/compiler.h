#pragma once
#include "token.h"
#include "emit.h"
#include <vm/chapman.h>
#include <stdint.h>

typedef struct ch_scope {
    /*
        The order of locals matters, because their index corresponds
        to the offset in the stack they will be found at
    */
    struct ch_scope_locals{
        uint32_t hashed_name;
    } locals[UINT8_MAX];
    uint8_t locals_size;
    uint32_t base_offset;

    struct ch_scope* parent;
} ch_scope;

typedef struct {
    ch_token_state token_state;
    ch_token previous;
    ch_token current;

    ch_emit emit;
    ch_scope* scope;

    bool has_errors;
} ch_compilation;

bool ch_compile(const uint8_t* program, size_t program_size, ch_program* output);