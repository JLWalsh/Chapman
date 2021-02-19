#include "token.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

const char* keywords[] = { "HALT" };
const uint8_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);

ch_token_kind ch_parse_token_kind(char* token_content);

ch_token* ch_extract_tokens(const char* program, size_t size, uint16_t* num_tokens) {
    const char* start = program;
    const char* current = program;

    ch_token* tokens = (ch_token*) malloc(sizeof(ch_token) * 100);
    ch_token* current_token = tokens;

    while((intptr_t)current != (intptr_t)(program + size - 1)) {
        while(isalpha(*current)) {
            current++;
        }

        size_t token_content_size = current - start + 1;
        char token_content[token_content_size];

        memcpy(token_content, start, current-start);
        token_content[token_content_size - 1] = '\0';

        current_token->kind = ch_parse_token_kind(token_content);
        current_token++;
    }

    *num_tokens = current_token - tokens;

    return tokens;
}

ch_token_kind ch_parse_token_kind(char* token_content) {
    for(uint8_t i = 0; i < num_keywords; i++) {
        if(strcmp(token_content, keywords[i]) == 0) {
            return i + 1;
        }
    }

    return 0;
}