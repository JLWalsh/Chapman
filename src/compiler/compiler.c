#include "compiler.h"
#include "error.h"
#include "token.h"
#include "util.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vm/chapman.h>
#include <vm/hash.h>

#define GET_EMIT(comp_ptr) (&((comp_ptr)->emit))

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
} ch_precedence_level;

typedef void (*ch_parse_func)(ch_compilation *);

typedef struct {
  ch_precedence_level prec;
  ch_parse_func prefix_parse;
  ch_parse_func infix_parse;
} ch_parse_rule;

static void advance(ch_compilation *comp);
static bool consume(ch_compilation *comp, ch_token_kind kind,
                    const char *error_message, ch_token *out_token);

static void error(ch_compilation *comp, const char *error_message, ...);
static void synchronize_outside_function(ch_compilation *comp);
static void synchronize_in_function(ch_compilation *comp);

static void free_compiler(ch_compilation *comp);
static ch_dataptr emit_string(ch_compilation *comp, const char *value,
                              size_t size);

static void statement(ch_compilation *comp);
static void function(ch_compilation *comp);
static void native_function(ch_compilation *comp);
static ch_argcount function_arglist(ch_compilation *comp);

/*
    Identifiers are handled differently when read as a standalone statement VS
   when they're part of an expression. When read as a standalone statement, we
   expect the identifier to always be invoked, but in an expression an
   identifier could simply refer to a variable.
*/
static void statement_identifier(ch_compilation *comp);
static void expression_identifier(ch_compilation *comp);
static void identifier(ch_compilation *comp, bool must_have_invocation);

static void declaration(ch_compilation *comp);
static void scope(ch_compilation *comp);
static void function_statement(ch_compilation *comp);

static uint8_t begin_scope(ch_compilation *comp);
static void end_scope(ch_compilation *comp, uint8_t parent_scope_size);
static bool scope_lookup(ch_compilation *comp, ch_lexeme name, uint8_t *offset);
static void add_variable(ch_compilation *comp, ch_lexeme name);
static void add_local(ch_compilation *comp, ch_lexeme name);
static void add_global(ch_compilation *comp, ch_lexeme name);
static void load_local(ch_compilation *comp, uint8_t offset);
static void load_global(ch_compilation *comp, ch_lexeme name);

static void call_main(ch_compilation *comp);

// Expression parsing
static void parse(ch_compilation *comp, ch_precedence_level prec);
static void grouping(ch_compilation *comp);
static void expression(ch_compilation *comp);
static void binary(ch_compilation *comp);
static void unary(ch_compilation *comp);
static void number(ch_compilation *comp);
static void return_statement(ch_compilation *comp);
static ch_argcount invocation(ch_compilation *comp);

ch_parse_rule rules[NUM_TOKENS] = {
    [TK_POPEN] = {PREC_NONE, grouping, NULL},
    [TK_MINUS] = {PREC_TERM, unary, binary},
    [TK_PLUS] = {PREC_TERM, NULL, binary},
    [TK_FSLASH] = {PREC_FACTOR, NULL, binary},
    [TK_STAR] = {PREC_FACTOR, NULL, binary},
    [TK_NUM] = {PREC_NONE, number, NULL},
    [TK_ID] = {PREC_NONE, expression_identifier, NULL},
};

const ch_parse_rule *get_rule(ch_token_kind kind);

bool ch_compile(const uint8_t *program, size_t program_size,
                ch_program *output) {
  ch_emit_scope global_emit_scope;
  ch_compilation comp = {
      .token_state = ch_token_init(program, program_size),
      .scope = {.locals_size = 0},
      .is_panic = false,
      .has_errors = false,
      .emit = ch_emit_create(&global_emit_scope),
  };

  advance(&comp);
  while (comp.current.kind != TK_EOF) {
    if (comp.is_panic) {
      synchronize_outside_function(&comp);
    }

    statement(&comp);
  }

  consume(&comp, TK_EOF, "Expected end of file.", NULL);

  call_main(&comp);

  ch_dataptr program_start_ptr = ch_emit_commit_scope(&comp.emit);

  *output = ch_emit_assemble(GET_EMIT(&comp), program_start_ptr);

  free_compiler(&comp);

  return !comp.has_errors;
}

void call_main(ch_compilation *comp) {
  // Load main function
  EMIT_OP(GET_EMIT(comp), OP_LOAD_GLOBAL);
  ch_lexeme main_name = {.start = "main", .size = 4};
  ch_dataptr string_ptr = emit_string(comp, main_name.start, main_name.size);
  EMIT_PTR(GET_EMIT(comp), string_ptr);

  // Call it
  EMIT_OP(GET_EMIT(comp), OP_CALL);
  ch_argcount argcount = 0;
  EMIT_ARGCOUNT(GET_EMIT(comp), argcount);

  // When main returns, halt the machine
  EMIT_OP(GET_EMIT(comp), OP_HALT);
}

void advance(ch_compilation *comp) {
  comp->previous = comp->current;

  // Skip over any erroneous tokens
  // TODO make compilation fail when erroneous tokens are encountered
  while (!ch_token_next(&comp->token_state, &comp->current))
    ;
}

bool consume(ch_compilation *comp, ch_token_kind kind,
             const char *error_message, ch_token *out_token) {
  if (comp->is_panic)
    return false;

  if (comp->current.kind == kind) {
    advance(comp);

    if (out_token != NULL) {
      *out_token = comp->previous;
    }

    return true;
  }

  error(comp, error_message);
  return false;
}

void error(ch_compilation *comp, const char *error_message, ...) {
  printf("[%" PRIu16 "]: ", comp->token_state.line);

  va_list args;
  va_start(args, error_message);
  vfprintf(stderr, error_message, args);
  va_end(args);
  fputs("\n", stderr);

  comp->is_panic = true;
  comp->has_errors = true;
}

// Error recovery used when the error occurs outside a function declaration
void synchronize_outside_function(ch_compilation *comp) {
  comp->is_panic = false;

  while (comp->current.kind != TK_EOF) {
    if (comp->previous.kind == TK_SEMI)
      return;

    switch (comp->current.kind) {
    case TK_POUND:
    case TK_CCLOSE:
      return;

    default:
      advance(comp);
    }
  }
}

// Error recovery used when the error occurs inside a function declaration
void synchronize_in_function(ch_compilation *comp) {
  comp->is_panic = false;

  while (comp->current.kind != TK_EOF) {
    switch (comp->current.kind) {
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

void free_compiler(ch_compilation *comp) {
  for (uint32_t i = 0; i < comp->strings.size; i++) {
    ch_table_entry entry = comp->strings.entries[i];
    if (entry.key != NULL) {
      // Strings allocated in emit_string
      free(entry.key);
    }
  }
}

ch_string *new_string(const char *value, size_t size) {
  ch_string *string = (ch_string *)malloc(sizeof(ch_string));
  ch_initstring(string, value, size);

  return string;
}

ch_dataptr emit_string(ch_compilation *comp, const char *value, size_t size) {
  ch_string *same_string = ch_table_find_string(&comp->strings, value, size);

  if (same_string != NULL) {
    ch_primitive *position = ch_table_get(&comp->strings, same_string);
    return (ch_dataptr)position->number_value;
  }

  ch_dataptr string_ptr;
  EMIT_DATA_STRING(GET_EMIT(comp), value, size, string_ptr);

  ch_string *key = new_string(value, size);
  ch_table_set(&comp->strings, key, MAKE_NUMBER(string_ptr));

  return string_ptr;
}

void statement(ch_compilation *comp) {
  ch_token_kind kind = comp->current.kind;

  switch (comp->current.kind) {
  case TK_VAL: {
    declaration(comp);
    consume(comp, TK_SEMI, "Expected semicolon.", NULL);
    break;
  }
  case TK_POUND: {
    function(comp);
    break;
  }
  // case TK_NATIVE: {
  //   native_function(comp);
  //   break;
  // }
  default: {
    error(comp, "Expected statement.");
    return;
  }
  }
}

void native_function(ch_compilation *comp) {
  consume(comp, TK_NATIVE, "Expected declaration of native function.", NULL);
  consume(comp, TK_POUND, "Expected start of function.", NULL);
  ch_token name;
  if (!consume(comp, TK_ID, "Expected function name.", &name))
    return;

  ch_argcount argcount = function_arglist(comp);

  consume(comp, TK_SEMI, "Expected semicolon.", NULL);

  EMIT_OP(GET_EMIT(comp), OP_NATIVE);
  EMIT_ARGCOUNT(GET_EMIT(comp), argcount);

  add_variable(comp, name.lexeme);
}

void function(ch_compilation *comp) {
  consume(comp, TK_POUND, "Expected start of function.", NULL);
  ch_token name;
  if (!consume(comp, TK_ID, "Expected function name.", &name))
    return;

  ch_emit_scope emit_scope;
  ch_emit_create_scope(&comp->emit, &emit_scope);

  uint8_t scope_mark = begin_scope(comp);
  ch_argcount argcount = function_arglist(comp);
  scope(comp);
  end_scope(comp, scope_mark);

  // Ensure that all functions return
  EMIT_OP(GET_EMIT(comp), OP_RETURN_VOID);

  ch_dataptr function_ptr = ch_emit_commit_scope(&comp->emit);

  EMIT_OP(GET_EMIT(comp), OP_FUNCTION);
  EMIT_PTR(GET_EMIT(comp), function_ptr);
  EMIT_ARGCOUNT(GET_EMIT(comp), argcount);

  add_variable(comp, name.lexeme);
}

ch_argcount function_arglist(ch_compilation *comp) {
  consume(comp, TK_POPEN, "Expected (.", NULL);

  ch_argcount argcount = 0;
  while (comp->current.kind != TK_PCLOSE) {
    if (++argcount >= CH_ARGCOUNT_MAX) {
      error(comp, "Exceeded argument limit.");
      break;
    }

    ch_token name;
    if (!consume(comp, TK_ID, "Expected variable name.", &name))
      return 0;
    add_local(comp, name.lexeme);

    if (comp->current.kind != TK_PCLOSE) {
      consume(comp, TK_COMMA, "Expected comma.", NULL);
    }
  }

  consume(comp, TK_PCLOSE, "Expected ).", NULL);

  return argcount;
}
void statement_identifier(ch_compilation *comp) {
  /*
      Ensure that we're invoking the variable

      Tolerated:
      print(x);
      Not tolerated:
      print;
  */
  advance(comp); // identifier() expects the TK_ID to already be processed
  identifier(comp, true);
}

void expression_identifier(ch_compilation *comp) { identifier(comp, false); }

void identifier(ch_compilation *comp, bool must_have_invocation) {
  ch_token name = comp->previous;

  uint8_t offset;
  bool is_local = scope_lookup(comp, name.lexeme, &offset);

  bool is_invocation = comp->current.kind == TK_POPEN;
  if (!is_invocation && must_have_invocation) {
    error(comp, "Expected invocation.");
    return;
  }

  ch_argcount argcount;
  if (is_invocation) {
    argcount = invocation(comp);
  }

  if (is_local) {
    load_local(comp, offset);
  } else {
    load_global(comp, name.lexeme);
  }

  if (is_invocation) {
    EMIT_OP(GET_EMIT(comp), OP_CALL);
    EMIT_ARGCOUNT(GET_EMIT(comp), argcount);
  }

  return;
}

ch_argcount invocation(ch_compilation *comp) {
  consume(comp, TK_POPEN, "Expected start of invocation", NULL);

  bool has_comma = false;
  ch_argcount argcount = 0;
  while (comp->current.kind != TK_PCLOSE && comp->current.kind != TK_EOF) {
    expression(comp);
    has_comma = false;

    if (comp->current.kind == TK_COMMA) {
      advance(comp);
      has_comma = true;
    }

    argcount++;
  }

  if (has_comma) {
    error(comp, "Expected argument.");
    return argcount;
  }

  consume(comp, TK_PCLOSE, "Expected end of arguments list.", NULL);

  return argcount;
}

void declaration(ch_compilation *comp) {
  consume(comp, TK_VAL, "Expected variable declaration.", NULL);
  ch_token name;
  if (!consume(comp, TK_ID, "Expected variable name.", &name))
    return;
  consume(comp, TK_EQ, "Expected variable initializer.", NULL);

  expression(comp);

  add_variable(comp, name.lexeme);
}

void add_variable(ch_compilation *comp, ch_lexeme name) {
  if (comp->scope.locals_size == UINT8_MAX) {
    error(comp, "Exceeded variable limit in scope.");
    return;
  }

  if (scope_lookup(comp, name, NULL)) {
    error(comp, "Variable has already been defined: %.*s", name.size,
          name.start);
    return;
  }

  if (CH_EMITTING_GLOBALLY(GET_EMIT(comp))) {
    add_global(comp, name);
  } else {
    add_local(comp, name);
  }
}

void add_local(ch_compilation *comp, ch_lexeme name) {
  ch_local *local = &comp->scope.locals[comp->scope.locals_size++];
  local->name = name;
}

void add_global(ch_compilation *comp, ch_lexeme name) {
  EMIT_OP(GET_EMIT(comp), OP_SET_GLOBAL);
  ch_dataptr string_ptr = emit_string(comp, name.start, name.size);
  EMIT_PTR(GET_EMIT(comp), string_ptr);
}

void load_local(ch_compilation *comp, uint8_t offset) {
  EMIT_OP(GET_EMIT(comp), OP_LOAD_LOCAL);
  EMIT_PTR(GET_EMIT(comp), offset);
}

void load_global(ch_compilation *comp, ch_lexeme name) {
  EMIT_OP(GET_EMIT(comp), OP_LOAD_GLOBAL);
  ch_dataptr string_ptr = emit_string(comp, name.start, name.size);
  EMIT_PTR(GET_EMIT(comp), string_ptr);
}

void scope(ch_compilation *comp) {
  consume(comp, TK_COPEN, "Expected start of scope.", NULL);

  while (comp->current.kind != TK_CCLOSE && comp->current.kind != TK_EOF) {
    function_statement(comp);
  }

  consume(comp, TK_CCLOSE, "Expected end of scope.", NULL);
}

bool scope_lookup(ch_compilation *comp, ch_lexeme name, uint8_t *offset) {
  if (comp->scope.locals_size == 0) {
    return false;
  }

  for (int32_t i = comp->scope.locals_size - 1; i >= 0; i--) {
    ch_lexeme *value_name = &comp->scope.locals[i].name;
    if (value_name->size != name.size) {
      continue;
    }

    if (strncmp(value_name->start, name.start, value_name->size) == 0) {
      if (offset != NULL)
        *offset = i;
      return true;
    }
  }

  return false;
}

void function_statement(ch_compilation *comp) {
  ch_token_kind kind = comp->current.kind;

  switch (comp->current.kind) {
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
  case TK_POUND: {
    function(comp);
    break;
  }
  case TK_ID: {
    statement_identifier(comp);
    break;
  }
  default:
    error(comp, "Expected statement.");
  }

  consume(comp, TK_SEMI, "Expected semicolon.", NULL);

  if (comp->is_panic)
    synchronize_in_function(comp);
}

uint8_t begin_scope(ch_compilation *comp) { return comp->scope.locals_size; }

void end_scope(ch_compilation *comp, uint8_t last_scope_size) {
  if (last_scope_size != comp->scope.locals_size) {
    uint8_t num_values_popped = comp->scope.locals_size - last_scope_size;
    comp->scope.locals_size = last_scope_size;

    EMIT_OP(GET_EMIT(comp), OP_POPN);
    EMIT_PTR(GET_EMIT(comp), num_values_popped);
  }
}

void parse(ch_compilation *comp, ch_precedence_level prec) {
  advance(comp);
  ch_parse_func prefix = get_rule(comp->previous.kind)->prefix_parse;
  if (!prefix) {
    error(comp, "Expected expression");
    return;
  }

  prefix(comp);

  while (prec <= get_rule(comp->current.kind)->prec) {
    advance(comp);
    ch_parse_func infix = get_rule(comp->previous.kind)->infix_parse;
    infix(comp);
  }
}

const ch_parse_rule *get_rule(ch_token_kind kind) { return &rules[kind]; }

void grouping(ch_compilation *comp) {
  expression(comp);
  consume(comp, TK_PCLOSE, "Expected closing parenthesis.", NULL);
}

void expression(ch_compilation *comp) { parse(comp, PREC_ASSIGNMENT); }

void binary(ch_compilation *comp) {
  ch_token_kind kind = comp->previous.kind;

  ch_precedence_level prec = get_rule(kind)->prec;
  parse(comp, (ch_precedence_level)(prec + 1));

  switch (kind) {
  case TK_PLUS:
    EMIT_OP(GET_EMIT(comp), OP_ADD);
    break;
  case TK_MINUS:
    EMIT_OP(GET_EMIT(comp), OP_SUB);
    break;
  case TK_STAR:
    EMIT_OP(GET_EMIT(comp), OP_MUL);
    break;
  case TK_FSLASH:
    EMIT_OP(GET_EMIT(comp), OP_DIV);
    break;
  default:
    return;
  }
}

void unary(ch_compilation *comp) {
  ch_token_kind kind = comp->previous.kind;

  parse(comp, PREC_UNARY);

  switch (kind) {
  case TK_MINUS:
    EMIT_OP(GET_EMIT(comp), OP_NEGATE);
    break;
  default:
    return;
  }
}

void number(ch_compilation *comp) {
  const char *start = comp->previous.lexeme.start;

  double value = strtod(start, NULL);
  ch_dataptr value_ptr = EMIT_DATA_DOUBLE(GET_EMIT(comp), value);

  EMIT_OP(GET_EMIT(comp), OP_NUMBER);
  EMIT_PTR(GET_EMIT(comp), value_ptr);
}

void return_statement(ch_compilation *comp) {
  consume(comp, TK_RETURN, "Expected return statement.", NULL);

  // Return statement without expression
  if (comp->current.kind == TK_SEMI) {
    EMIT_OP(GET_EMIT(comp), OP_RETURN_VOID);
    return;
  }

  // Return statement with expression
  expression(comp);
  EMIT_OP(GET_EMIT(comp), OP_RETURN_VALUE);
}
