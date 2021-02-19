#include <vm/chapman.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <vm/chapman.h>
#include "token.h"
#include "emit.h"

const char program[] = "HALT"; 

int main(void) {
    uint16_t num_tokens;
    ch_token* tokens = ch_extract_tokens(program, sizeof(program), &num_tokens);
    uint8_t* program = ch_emit(tokens, num_tokens);

    ch_run(program);

    return 0;
}