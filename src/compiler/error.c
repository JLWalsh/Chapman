#include "error.h"
#include <stdio.h>

// TODO make these functions' messages a bit more consistent and helpful
void ch_tk_error(const char *message, const ch_token_state *state) {
  printf("Tokenization error (line %d): %s\n", state->line, message);
}