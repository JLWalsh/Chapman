#include "compiler.h"
#include "token.h"
#include "error.h"
#include "util.h"
#include "hash.h"
#include <vm/chapman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} ch_precedence_level;

typedef void (*ch_parse_func)(ch_compilation*);

typedef struct {
    ch_precedence_level prec;
    ch_parse_func prefix_parse;
    ch_parse_func infix_parse;
} ch_parse_rule;

void advance(ch_compilation* comp);
ch_token consume(ch_compilation* comp, ch_token_kind kind, const char* error_message);

void function(ch_compilation* comp);
void identifier(ch_compilation* comp);
void declaration(ch_compilation* comp);

// Expression parsing
void parse(ch_compilation* comp, ch_precedence_level prec);
void grouping(ch_compilation* comp);
void expression(ch_compilation* comp);
void binary(ch_compilation* comp);
void unary(ch_compilation* comp);
void number(ch_compilation* comp);

ch_parse_rule rules[NUM_TOKENS] = {
    [TK_POPEN]  = {PREC_NONE,   grouping,   NULL},
    [TK_MINUS]  = {PREC_TERM,   unary,      binary},
    [TK_PLUS]   = {PREC_TERM,   NULL,       binary},
    [TK_FSLASH] = {PREC_FACTOR, NULL,       binary},
    [TK_STAR]   = {PREC_FACTOR, NULL,       binary},
    [TK_NUM]    = {PREC_NONE,   number,     NULL},
    [TK_ID]     = {PREC_NONE,   identifier, NULL}
};

const ch_parse_rule* get_rule(ch_token_kind kind);

ch_program ch_compile(const uint8_t* program, size_t program_size) {
    ch_compilation comp = {.token_state=ch_token_init(program, program_size), .emit=ch_emit_create()};

    advance(&comp);
    declaration(&comp);
    expression(&comp);
    consume(&comp, TK_SEMI, "Consume semi!!");
    declaration(&comp);
    expression(&comp);
    consume(&comp, TK_SEMI, "Consume semi!!");

    consume(&comp, TK_EOF, "Expected end of file.");
    ch_emit_op(&comp.emit, OP_HALT);

    return ch_assemble(&comp.emit);
}

void advance(ch_compilation* comp) {
    comp->previous = comp->current;

    // Skip over any erroneous tokens
    while(!ch_token_next(&comp->token_state, &comp->current));
}

ch_token consume(ch_compilation* comp, ch_token_kind kind, const char* error_message) {
    if (comp->current.kind == kind) {
        advance(comp);
        return comp->previous;
    }

    ch_pr_error(error_message, comp);
}

void function(ch_compilation* comp) {
    consume(comp, TK_POUND, "Expected start of function.");
    ch_token name = consume(comp, TK_ID, "Expected function name.");
    consume(comp, TK_POPEN, "Expected (.");
    consume(comp, TK_PCLOSE, "Expected ).");
}

void identifier(ch_compilation* comp) {
    ch_token name = comp->previous;
    uint32_t hashed_name = ch_hashstring(name.lexeme.start, name.lexeme.size);
    ch_scope* scope = &comp->scope;

    for(uint8_t i = 0; i < scope->locals_size; i++) {
        if (scope->locals[i].hashed_name == hashed_name) {
            ch_emit_op(&comp->emit, OP_LOAD_LOCAL);
            ch_emit_number(&comp->emit, (double) i);
            return;
        }
    }

    char message[100];
    snprintf(message, sizeof(message), "Unresolved variable: %.*s", name.lexeme.size, name.lexeme.start);
    ch_pr_error(message, comp);
}

void declaration(ch_compilation* comp) {
    consume(comp, TK_VAL, "Expected variable declaration");
    ch_token name = consume(comp, TK_ID, "Expected variable name.");
    consume(comp, TK_EQ, "Expected variable initializer.");

    expression(comp);

    ch_scope* scope = &comp->scope;
    scope->locals[scope->locals_size++].hashed_name = ch_hashstring(name.lexeme.start, name.lexeme.size);

    consume(comp, TK_SEMI, "Expected semicolon.");
}

void parse(ch_compilation* comp, ch_precedence_level prec) {
    advance(comp);
    ch_parse_func prefix = get_rule(comp->previous.kind)->prefix_parse;
    if (!prefix) {
        ch_pr_error("Expected expression.", comp);
        return;
    }

    prefix(comp);

    while(prec <= get_rule(comp->current.kind)->prec) {
        advance(comp);
        ch_parse_func infix = get_rule(comp->previous.kind)->infix_parse;
        infix(comp);
    }
}

void grouping(ch_compilation* comp) {
    expression(comp);
    consume(comp, TK_PCLOSE, "Expected closing parenthesis.");
}

void expression(ch_compilation* comp) {
    parse(comp, PREC_ASSIGNMENT);
}

void binary(ch_compilation* comp) {
    ch_token_kind kind = comp->previous.kind;

    ch_precedence_level prec = get_rule(kind)->prec;
    parse(comp, (ch_precedence_level) (prec + 1));

    switch(kind) {
        case TK_PLUS: ch_emit_op(&comp->emit, OP_ADD); break;
        case TK_MINUS: ch_emit_op(&comp->emit, OP_SUB); break;
        case TK_STAR: ch_emit_op(&comp->emit, OP_MUL); break;
        case TK_FSLASH: ch_emit_op(&comp->emit, OP_DIV); break;
        default:
            return;
    }
}

void unary(ch_compilation* comp) {
    ch_token_kind kind = comp->previous.kind;

    parse(comp, PREC_UNARY);

    switch(kind) {
        case TK_MINUS: ch_emit_op(&comp->emit, OP_NEGATE); break;
        default:
            return;
    }
}

void number(ch_compilation* comp) {
    const char* start = comp->previous.lexeme.start;

    double value = strtod(start, NULL);

    ch_emit_op(&comp->emit, OP_NUMBER);
    ch_emit_number(&comp->emit, value);
}

const ch_parse_rule* get_rule(ch_token_kind kind) {
    return &rules[kind];
}