#pragma once
#include "token.h"
#include "emit.h"
#include <vm/chapman.h>
#include <stdint.h>

typedef struct {
    /*
        The order of locals matters, because their index corresponds
        to the offset in the stack they will be found at
    */
    struct ch_scope_locals{
        uint32_t hashed_name;
    } locals[UINT8_MAX];
    uint8_t locals_size;
} ch_scope;

typedef struct {
    ch_token_state token_state;
    ch_token previous;
    ch_token current;

    ch_emit emit;
    ch_scope scope;
} ch_compilation;

ch_program ch_compile(const uint8_t* program, size_t program_size);