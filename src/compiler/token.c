#include "token.h"
#include "limits.h"
#include "error.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    char* name;
    ch_token_kind kind;
} ch_keyword;

const ch_keyword keywords[] = {
    { "val", TK_VAL },
    { "var", TK_VAR },
};

// Unspecified tokens are set to false by default
// (Since uninitialized values are always set to 0)
const bool tokens_with_lexemes[NUM_TOKENS] = {
    [TK_ID] = true,
    [TK_NUM] = true
};

const uint8_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);

ch_token_kind ch_parse_token_kind(const char* token_start, size_t token_size);

ch_token ch_get_token(const char* start, ch_token_state* state, ch_token_kind kind);

ch_token_state ch_token_init(const uint8_t* program, size_t size) {
    return (ch_token_state) {.program=program, .size=size, .current=(char*)program, .line=1};
}

bool ch_token_next(ch_token_state* state, ch_token* next) {
    char* start = state->current;

    while((intptr_t)state->current != (intptr_t)(state->program + state->size)) {
        char current = *state->current++;

        switch (current) {
            case '=':
                *next = ch_get_token(start, state, TK_EQ);
                return true;
            case '+':
                *next = ch_get_token(start, state, TK_PLUS);
                return true;
            case '-':
                *next = ch_get_token(start, state, TK_MINUS);
                return true;
            case ';':
                *next = ch_get_token(start, state, TK_SEMI);
                return true;
            case ',':
                *next = ch_get_token(start, state, TK_COMMA);
                return true;
            case '(':
                *next = ch_get_token(start, state, TK_POPEN);
                return true;
            case ')':
                *next = ch_get_token(start, state, TK_PCLOSE);
                return true;
            case '{':
                *next = ch_get_token(start, state, TK_COPEN);
                return true;
            case '}':
                *next = ch_get_token(start, state, TK_CCLOSE);
                return true;
            case '\0':
                *next = ch_get_token(start, state, TK_EOF);
                return true;
            case '#':
                *next = ch_get_token(start, state, TK_POUND);
                return true;
            case '\n':
                state->line++;
                start = state->current;
                continue;
            case '\r':
                start = state->current;
                continue;
            case '/':
                if (*state->current == '/') {
                    while(*state->current != '\n') {
                        state->current++;
                        start++;
                    }
                    continue;
                }
                
                *next = ch_get_token(start, state, TK_FSLASH);
                return true;
            case '*':
                *next = ch_get_token(start, state, TK_STAR);
                return true;
            default:
                if (isspace(current)) {
                    start = state->current;
                    continue;
                }

                // Identifiers
                if (isalpha(current)) {
                    while(isalnum(*state->current)) {
                        state->current++;
                    }

                    ptrdiff_t lexeme_size = state->current - start;
                    if (lexeme_size >= MAX_ID_LENGTH) {
                        ch_tk_error("Identifier size exceeds limit", state);
                        return false;
                    }

                    ch_token_kind token_kind = ch_parse_token_kind(start, lexeme_size);
                    *next = ch_get_token(start, state, token_kind);

                    return true;
                }

                // Numbers
                if (isdigit(current)) {
                    while(isdigit(*state->current)) {
                        state->current++;
                    }

                    if (*state->current == '.') {
                        *state->current++;
                        char* digitStart = state->current;

                        while(isdigit(*state->current)) {
                            state->current++;
                        }

                        // No digits followed the .
                        if (digitStart == state->current) {
                            ch_tk_error("Unterminated number", state);
                            return false;
                        }
                    }

                    *next = ch_get_token(start, state, TK_NUM);
                    return true;
                }

                ch_tk_error("Unrecognized character", state);
                return false;
        }
    }

    *next = ch_get_token(start, state, TK_EOF);
    return true;
}

ch_token_kind ch_parse_token_kind(const char* token_start, size_t token_size) {
    for(uint8_t i = 0; i < num_keywords; i++) {
        if(strncmp(token_start, keywords[i].name, token_size) == 0) {
            return keywords[i].kind;
        }
    }

    return TK_ID;
}

ch_token ch_get_token(const char* start, ch_token_state* state, ch_token_kind kind) {
    ch_token token = {.kind=kind,.line=state->line};
     
    if (tokens_with_lexemes[kind]) {
        token.lexeme = (ch_lexeme){.start=start, .size=state->current - start};
    }

    return token;
}