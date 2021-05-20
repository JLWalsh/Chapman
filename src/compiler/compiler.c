#include "compiler.h"
#include "token.h"
#include "error.h"
#include "util.h"
#include <vm/chapman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO review these values
#define INITIAL_FUNCTION_BYTECODE_SIZE 10000
#define INITIAL_DATA_BLOB_SIZE 20000

#define CURRENT_BLOB(comp_ptr) (&((comp_ptr)->blobscope->bytecode_blobs[0]))

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
bool consume(ch_compilation* comp, ch_token_kind kind, const char* error_message, ch_token* out_token);
void panic(ch_compilation* comp, const char* error_message);
void synchronize_outside_function(ch_compilation* comp);
void synchronize_in_function(ch_compilation* comp);

void function(ch_compilation* comp);
void function_arglist(ch_compilation* comp);
void identifier(ch_compilation* comp);
void declaration(ch_compilation* comp);
void add_local(ch_compilation* comp, ch_lexeme name);
void scope(ch_compilation* comp);
void function_statement(ch_compilation* comp);
uint8_t begin_scope(ch_compilation* comp);
void end_scope(ch_compilation* comp, uint8_t parent_scope_size);

void begin_blobscope(ch_compilation* comp, ch_blobscope* current);
void end_blobscope(ch_compilation* comp);

bool scope_lookup(ch_compilation* comp, ch_lexeme name, uint8_t* offset);

// Expression parsing
void parse(ch_compilation* comp, ch_precedence_level prec);
void grouping(ch_compilation* comp);
void expression(ch_compilation* comp);
void binary(ch_compilation* comp);
void unary(ch_compilation* comp);
void number(ch_compilation* comp);
void return_statement(ch_compilation* comp);

ch_parse_rule rules[NUM_TOKENS] = {
    [TK_POPEN]  = {PREC_NONE,   grouping,   NULL},
    [TK_MINUS]  = {PREC_TERM,   unary,      binary},
    [TK_PLUS]   = {PREC_TERM,   NULL,       binary},
    [TK_FSLASH] = {PREC_FACTOR, NULL,       binary},
    [TK_STAR]   = {PREC_FACTOR, NULL,       binary},
    [TK_NUM]    = {PREC_NONE,   number,     NULL},
    [TK_ID]     = {PREC_NONE,   identifier, NULL},
};

const ch_parse_rule* get_rule(ch_token_kind kind);

bool ch_compile(const uint8_t* program, size_t program_size, ch_program* output) {
    ch_compilation comp = {
        .token_state=ch_token_init(program, program_size), 
        .scope={.locals_size=0},
        .is_panic=false,
        .has_errors=false,
        .blobscope=NULL,
        .data_blob=ch_create_blob(INITIAL_DATA_BLOB_SIZE),
    };

    ch_blobscope blobscope;
    begin_blobscope(&comp, &blobscope);

    advance(&comp);
    while (comp.current.kind != TK_EOF) {
        if (comp.is_panic) {
            synchronize_outside_function(&comp);
        }

        function(&comp);
    }

    consume(&comp, TK_EOF, "Expected end of file.", NULL);
    ch_emit_op(CURRENT_BLOB(&comp), OP_HALT);

    *output = ch_assemble(&comp.data_blob, &comp.blobscope->bytecode_blobs[0]);

    return !comp.has_errors;
}

void advance(ch_compilation* comp) {
    comp->previous = comp->current;

    // Skip over any erroneous tokens
    while(!ch_token_next(&comp->token_state, &comp->current));
}

bool consume(ch_compilation* comp, ch_token_kind kind, const char* error_message, ch_token* out_token) {
    if (comp->is_panic) return false;

    if (comp->current.kind == kind) {
        advance(comp);

        if (out_token != NULL) {
            *out_token = comp->previous;
        }

        return true;
    }

    panic(comp, error_message);
    return false;
}

void panic(ch_compilation* comp, const char* error_message) {
    comp->is_panic = true;
    comp->has_errors = true;
    ch_pr_error(error_message, comp);
}

// Error recovery used when the error occurs outside a function declaration
void synchronize_outside_function(ch_compilation* comp) {
    comp->is_panic = false;

    while(comp->current.kind != TK_EOF) {
        if (comp->previous.kind == TK_SEMI) return;
        
        switch(comp->current.kind) {
            case TK_POUND:
            case TK_CCLOSE:
                return;

            default:
                advance(comp);
        }
    }
}

// Error recovery used when the error occurs inside a function declaration
void synchronize_in_function(ch_compilation* comp) {
    comp->is_panic = false;

    while(comp->current.kind != TK_EOF) {
        switch(comp->current.kind) {
            case TK_SEMI: {
                // We want to restart parsing after the semi
                advance(comp);
                return;
            }

            // TODO add other keywords here (when implemented)
            case TK_CCLOSE:
            case TK_VAL:
                return;

            default:
                advance(comp);
        }
    }
}

void begin_blobscope(ch_compilation* comp, ch_blobscope* current) {
    // Blob 0 is the current function
    current->bytecode_blobs[0] = ch_create_blob(INITIAL_FUNCTION_BYTECODE_SIZE);
    current->num_blobs = 1;
    current->parent = comp->blobscope;

    comp->blobscope = current;
}

void end_blobscope(ch_compilation* comp) {
    ch_blob merged_blobs = ch_merge_blobs(comp->blobscope->bytecode_blobs, comp->blobscope->num_blobs);
    comp->blobscope = comp->blobscope->parent;

    if (comp->blobscope->num_blobs >= MAX_FUNCTION_BLOBS) {
        panic(comp, "Exceeded number of nested functions.");
        return;
    }

    comp->blobscope->bytecode_blobs[comp->blobscope->num_blobs++] = merged_blobs;
}

void function(ch_compilation* comp) {
    ch_blobscope blobscope;
    begin_blobscope(comp, &blobscope);

    consume(comp, TK_POUND, "Expected start of function.", NULL);
    ch_token name;
    if(!consume(comp, TK_ID, "Expected function name.", &name)) return;

    uint8_t scope_mark = begin_scope(comp);

    function_arglist(comp);

    scope(comp);

    end_scope(comp, scope_mark);

    end_blobscope(comp);
}

void function_arglist(ch_compilation* comp) {
    consume(comp, TK_POPEN, "Expected (.", NULL);

    while(comp->current.kind != TK_PCLOSE) {
        ch_token name;
        if(!consume(comp, TK_ID, "Expected variable name.", &name)) return;
        add_local(comp, name.lexeme);

        if (comp->current.kind != TK_PCLOSE) {
            consume(comp, TK_COMMA, "Expected comma.", NULL);
        }
    }

    consume(comp, TK_PCLOSE, "Expected ).", NULL);
}

void identifier(ch_compilation* comp) {
    ch_token name = comp->previous;

    uint8_t offset;
    if (scope_lookup(comp, name.lexeme, &offset)) {
        ch_emit_op(CURRENT_BLOB(comp), OP_LOAD_LOCAL);
        ch_emit_number(CURRENT_BLOB(comp), (double) offset);
        return;
    }

    char pattern[] = "Unresolved variable: %.*s";
    char message[sizeof(pattern) + name.lexeme.size];
    snprintf(message, sizeof(message), pattern, name.lexeme.size, name.lexeme.start);
    panic(comp, message);
}

void declaration(ch_compilation* comp) {
    consume(comp, TK_VAL, "Expected variable declaration.", NULL);
    ch_token name;
    if(!consume(comp, TK_ID, "Expected variable name.", &name)) return;
    consume(comp, TK_EQ, "Expected variable initializer.", NULL);

    expression(comp);

    consume(comp, TK_SEMI, "Expected semicolon.", NULL);

    add_local(comp, name.lexeme);
}

void add_local(ch_compilation* comp, ch_lexeme name) {
    if (comp->scope.locals_size == UINT8_MAX) {
        panic(comp, "Exceeded variable limit in scope.");
        return;
    }

    ch_local* local = &comp->scope.locals[comp->scope.locals_size++];
    local->name = name;
}

void scope(ch_compilation* comp) {
    consume(comp, TK_COPEN, "Expected start of scope.", NULL);

    while(comp->current.kind != TK_CCLOSE && comp->current.kind != TK_EOF) {
        function_statement(comp);
    }

    consume(comp, TK_CCLOSE, "Expected end of scope.", NULL);
}

bool scope_lookup(ch_compilation* comp, ch_lexeme name, uint8_t* offset) {
    if (comp->scope.locals_size == 0) {
        return false;
    }

    for(int32_t i = comp->scope.locals_size - 1; i >= 0; i--) {
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

void function_statement(ch_compilation* comp) {
    ch_token_kind kind = comp->current.kind;

    switch(comp->current.kind) {
        case TK_VAL: {
            declaration(comp);
            break;
        }
        case TK_COPEN: {
            uint8_t scope_mark = begin_scope(comp);
            scope(comp);
            end_scope(comp, scope_mark);
            break;
        }
        case TK_RETURN: {
            return_statement(comp);
            break;
        }
        default: panic(comp, "Expected statement.");
    }

    if (comp->is_panic) synchronize_in_function(comp);
}

uint8_t begin_scope(ch_compilation* comp) {
    return comp->scope.locals_size;
}

void end_scope(ch_compilation* comp, uint8_t last_scope_size) {
    if (last_scope_size != comp->scope.locals_size) {
        uint8_t num_values_popped = comp->scope.locals_size - last_scope_size;
        comp->scope.locals_size = last_scope_size;

        ch_emit_op(CURRENT_BLOB(comp), OP_POPN);
        ch_emit_number(CURRENT_BLOB(comp), (double) num_values_popped);
    }
}

void parse(ch_compilation* comp, ch_precedence_level prec) {
    advance(comp);
    ch_parse_func prefix = get_rule(comp->previous.kind)->prefix_parse;
    if (!prefix) {
        panic(comp, "Expected expression");
        return;
    }

    prefix(comp);

    while(prec <= get_rule(comp->current.kind)->prec) {
        advance(comp);
        ch_parse_func infix = get_rule(comp->previous.kind)->infix_parse;
        infix(comp);
    }
}

const ch_parse_rule* get_rule(ch_token_kind kind) {
    return &rules[kind];
}

void grouping(ch_compilation* comp) {
    expression(comp);
    consume(comp, TK_PCLOSE, "Expected closing parenthesis.", NULL);
}

void expression(ch_compilation* comp) {
    parse(comp, PREC_ASSIGNMENT);
}

void binary(ch_compilation* comp) {
    ch_token_kind kind = comp->previous.kind;

    ch_precedence_level prec = get_rule(kind)->prec;
    parse(comp, (ch_precedence_level) (prec + 1));

    switch(kind) {
        case TK_PLUS: ch_emit_op(CURRENT_BLOB(comp), OP_ADD); break;
        case TK_MINUS: ch_emit_op(CURRENT_BLOB(comp), OP_SUB); break;
        case TK_STAR: ch_emit_op(CURRENT_BLOB(comp), OP_MUL); break;
        case TK_FSLASH: ch_emit_op(CURRENT_BLOB(comp), OP_DIV); break;
        default:
            return;
    }
}

void unary(ch_compilation* comp) {
    ch_token_kind kind = comp->previous.kind;

    parse(comp, PREC_UNARY);

    switch(kind) {
        case TK_MINUS: ch_emit_op(CURRENT_BLOB(comp), OP_NEGATE); break;
        default:
            return;
    }
}

void number(ch_compilation* comp) {
    const char* start = comp->previous.lexeme.start;

    double value = strtod(start, NULL);
    ch_dataptr value_ptr = ch_emit_double(&comp->data_blob, value);

    ch_emit_op(CURRENT_BLOB(comp), OP_NUMBER);
    ch_emit_ptr(CURRENT_BLOB(comp), value_ptr);
}

void return_statement(ch_compilation* comp) {
    consume(comp, TK_RETURN, "Expected return statement.", NULL);

    // Return statement without expression
    if (comp->current.kind == TK_SEMI) {
        ch_emit_op(CURRENT_BLOB(comp), OP_RETURN_VOID);
        return;
    }

    // Return statement with expression
    expression(comp);
    ch_emit_op(CURRENT_BLOB(comp), OP_RETURN_VALUE);
}
