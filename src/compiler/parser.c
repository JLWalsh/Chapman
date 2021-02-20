#include "parser.h"
#include "token.h"
#include <stdio.h>

void ch_parse_emit(ch_token_state* state) {
    ch_token current;
    
    if(!ch_token_next(state, &current)) {
        printf("Error parsing token!");
    } else {
        printf("All good!");
    }
}