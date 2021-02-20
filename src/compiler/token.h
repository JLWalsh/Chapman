#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HALT = 1
} ch_token_kind;

typedef struct ch_token {
    ch_token_kind kind;
} ch_token;

typedef struct {
    const uint8_t* program;
    uint8_t* current;
    size_t size;
} ch_token_state;

ch_token_state ch_token_init(const uint8_t* program, size_t size);

bool ch_token_next(ch_token_state* state, ch_token* next);