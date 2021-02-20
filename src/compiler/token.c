#include "token.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

const char* keywords[] = { "HALT" };
const uint8_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);

ch_token_kind ch_parse_token_kind(char* token_content);

ch_token_state ch_token_init(const uint8_t* program, size_t size) {
    return (ch_token_state) {.program=program, .size=size, .current=program};
}

bool ch_token_next(ch_token_state* state, ch_token* next) {
    const uint8_t* start = state->current;

    if ((intptr_t) state->current == (intptr_t) (state->program + state->size - 1)) {
        return false;
    }

    while((intptr_t)state->current != (intptr_t)(state->program + state->size - 1)) {
        while(isalpha(*state->current)) {
            state->current++;
        }

        size_t token_content_size = state->current - start + 1;
        char token_content[token_content_size];

        memcpy(token_content, start, state->current - start);
        token_content[token_content_size - 1] = '\0';

        next->kind = ch_parse_token_kind(token_content);

        return true;
    }

    return false;
}

/*
    Each ch_token_kind is index starting from 1, so that this method can return 0 when the token kind is not found
*/
ch_token_kind ch_parse_token_kind(char* token_content) {
    for(uint8_t i = 0; i < num_keywords; i++) {
        if(strcmp(token_content, keywords[i]) == 0) {
            return i + 1;
        }
    }

    return 0;
}