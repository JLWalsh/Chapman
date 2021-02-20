#include "parser.h"
#include "token.h"
#include <stdio.h>

void ch_parse_emit(ch_token_state* state) {
    for(int i = 0; i < 10; i++) {
        ch_token current;
        bool ok = ch_token_next(state, &current);
        int x = current.kind;
    }
}