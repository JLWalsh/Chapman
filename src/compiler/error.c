#include "error.h"
#include <stdio.h>

// TODO make these functions' messages a bit more consistent and helpful
void ch_tk_error(const char* message, const ch_token_state* state) {
    printf("Tokenization error (line %d): %s\n", state->line, message);
}

void ch_pr_error(const char* message, ch_compilation* state) {
    state->has_errors = true;
    printf("Parse error [%d]: %s\n", state->current.line, message);
}