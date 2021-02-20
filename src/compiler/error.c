#include "error.h"
#include <stdio.h>

void ch_tk_error(const char* message, const ch_token_state* state) {
    printf("Tokenization error (line %d): %s", state->line, message);
}