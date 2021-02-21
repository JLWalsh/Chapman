#include "compiler.h"
#include "token.h"
#include "error.h"
#include "util.h"
#include <vm/chapman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void advance(ch_compilation* comp);
void consume(ch_compilation* comp, ch_token_kind kind, const char* error_message);

void expression(ch_compilation* comp);
void unary(ch_compilation* comp);
void number(ch_compilation* comp);

ch_program ch_compile(const uint8_t* program, size_t program_size) {
    ch_compilation comp = {.token_state=ch_token_init(program, program_size), .emit=ch_emit_create()};

    advance(&comp);
    number(&comp);

    return ch_emit_assemble(&comp.emit);
}

void advance(ch_compilation* comp) {
    comp->previous = comp->current;

    // Skip over any erroneous tokens
    while(!ch_token_next(&comp->token_state, &comp->current));
}

void consume(ch_compilation* comp, ch_token_kind kind, const char* error_message) {
    if (comp->current.kind == kind) {
        advance(comp);
        return;
    }

    ch_pr_error(error_message, comp);
}

void expression(ch_compilation* comp) {
    
}

void unary(ch_compilation* comp) {

}

void number(ch_compilation* comp) {
    const char* start = comp->current.lexeme.start;

    double value = strtod(start, NULL);

    ch_dataptr ptr = ch_emit_data(&comp->emit, &value, sizeof(double));
    if (ptr == CH_DATAPTR_NULL) {
        ch_pr_error("Data section has reached max capacity.", comp);
        return;
    }

    ch_emit_op(&comp->emit, OP_NUMBER);
    ch_emit_ptr(&comp->emit, ptr);
}