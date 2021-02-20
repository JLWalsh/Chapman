#include <vm/chapman.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <vm/chapman.h>
#include "parser.h"

const char program[] = "HALT"; 

int main(void) {
    uint16_t num_tokens;
    ch_token_state state = ch_token_init(program, sizeof(program));

    ch_parse_emit(&state);

    return 0;
}