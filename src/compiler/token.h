#pragma once
#include <stdint.h>

typedef enum {
    HALT = 1
} ch_token_kind;

typedef struct ch_token {
    ch_token_kind kind;
} ch_token;

ch_token* ch_extract_tokens(const char* program, size_t size, uint16_t* num_tokens);