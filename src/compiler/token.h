#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    TK_VAL,
    TK_VAR,
    
    TK_EQ,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    // Forwardslash
    TK_FSLASH,

    // Semicolon
    TK_SEMI,
    TK_COMMA,

    TK_POUND,       // #

    // Parenthesis
    TK_POPEN,
    TK_PCLOSE,

    // Curly bracket
    TK_COPEN,
    TK_CCLOSE,

    TK_ID,
    TK_NUM,

    TK_EOF,

    // Do not declare any tokens after this
    NUM_TOKENS
} ch_token_kind;

typedef struct {
    const char* start;
    size_t size;
} ch_lexeme;

typedef struct ch_token {
    ch_token_kind kind;
    ch_lexeme lexeme;
} ch_token;

typedef struct {
    const uint8_t* program;
    char* current;
    size_t size;
    uint16_t line;
} ch_token_state;

ch_token_state ch_token_init(const uint8_t* program, size_t size);

bool ch_token_next(ch_token_state* state, ch_token* next);