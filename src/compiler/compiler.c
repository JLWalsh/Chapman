#include "compiler.h"
#include "token.h"
#include "error.h"
#include "util.h"
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
void function_arglist(ch_compilation* comp);
void identifier(ch_compilation* comp);
void declaration(ch_compilation* comp);
void add_local(ch_compilation* comp, ch_lexeme name);
void scope(ch_compilation* comp);
void statement(ch_compilation* comp);
uint8_t begin_scope(ch_compilation* comp);
void end_scope(ch_compilation* comp, uint8_t parent_scope_size);

bool scope_lookup(ch_compilation* comp, ch_lexeme name, uint8_t* offset);

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

bool ch_compile(const uint8_t* program, size_t program_size, ch_program* output) {
    ch_compilation comp = {
        .token_state=ch_token_init(program, program_size), 
        .emit=ch_emit_create(),
        .scope={.locals_size=0},
        .has_errors=false,
    };

    advance(&comp);
    function(&comp);

    consume(&comp, TK_EOF, "Expected end of file.");
    ch_emit_op(&comp.emit, OP_HALT);

    *output = ch_assemble(&comp.emit);

    return !comp.has_errors;
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

    uint8_t scope_mark = begin_scope(comp);

    function_arglist(comp);

    scope(comp);

    end_scope(comp, scope_mark);
}

void function_arglist(ch_compilation* comp) {
    consume(comp, TK_POPEN, "Expected (.");

    while(comp->current.kind != TK_PCLOSE) {
        ch_token name = consume(comp, TK_ID, "Expected variable name.");
        add_local(comp, name.lexeme);

        if (comp->current.kind != TK_PCLOSE) {
            consume(comp, TK_COMMA, "Expected comma.");
        }
    }

    consume(comp, TK_PCLOSE, "Expected ).");
}

void identifier(ch_compilation* comp) {
    ch_token name = comp->previous;

    uint8_t offset;
    if (scope_lookup(comp, name.lexeme, &offset)) {
        ch_emit_op(&comp->emit, OP_LOAD_LOCAL);
        ch_emit_number(&comp->emit, (double) offset);
        return;
    }

    char pattern[] = "Unresolved variable: %.*s";
    char message[sizeof(pattern) + name.lexeme.size];
    snprintf(message, sizeof(message), pattern, name.lexeme.size, name.lexeme.start);
    ch_pr_error(message, comp);
}

void declaration(ch_compilation* comp) {
    consume(comp, TK_VAL, "Expected variable declaration.");
    ch_token name = consume(comp, TK_ID, "Expected variable name.");
    consume(comp, TK_EQ, "Expected variable initializer.");

    expression(comp);

    consume(comp, TK_SEMI, "Expected semicolon.");

    add_local(comp, name.lexeme);
}

void add_local(ch_compilation* comp, ch_lexeme name) {
    if (comp->scope.locals_size == UINT8_MAX) {
        ch_pr_error("Exceeded variable limit in scope.", comp);
        return;
    }

    ch_local* local = &comp->scope.locals[comp->scope.locals_size++];
    local->name = name;
}

void scope(ch_compilation* comp) {
    consume(comp, TK_COPEN, "Expected start of scope.");

    while(comp->current.kind != TK_CCLOSE) {
        statement(comp);
    }

    consume(comp, TK_CCLOSE, "Expected end of scope.");
}

bool scope_lookup(ch_compilation* comp, ch_lexeme name, uint8_t* offset) {
    if (comp->scope.locals_size == 0) {
        return false;
    }

    for(uint8_t i = comp->scope.locals_size - 1; i >= 0; i--) {
        ch_lexeme* value_name = &comp->scope.locals[i].name;
        if (value_name->size != name.size) {
            continue;
        }

        if (strncmp(value_name->start, name.start, MIN(value_name->size, name.size)) == 0) {
            *offset = i;
            return true;
        }
    }

    return false;
}

void tempPrintExpression(ch_compilation* comp);
void statement(ch_compilation* comp) {
    if (comp->current.kind == TK_VAL) {
        declaration(comp);
    } else if (comp->current.kind == TK_COPEN) {
        uint8_t scope_mark = begin_scope(comp);
        scope(comp);
        end_scope(comp, scope_mark);
    } else { // TODO change this to read function invocation, for now we will just parse expressions and declarations
        tempPrintExpression(comp);
    }
}

uint8_t begin_scope(ch_compilation* comp) {
    return comp->scope.locals_size;
}

void end_scope(ch_compilation* comp, uint8_t last_scope_size) {
    if (last_scope_size != comp->scope.locals_size) {
        uint8_t num_values_popped = comp->scope.locals_size - last_scope_size;
        comp->scope.locals_size = last_scope_size;

        ch_emit_op(&comp->emit, OP_POPN);
        ch_emit_number(&comp->emit, (double) num_values_popped);
    }
}

void tempPrintExpression(ch_compilation* comp) {
    expression(comp);
    consume(comp, TK_SEMI, "Expected semicolon.");
    ch_emit_op(&comp->emit, OP_DEBUG);
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