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
  PREC_OR,         // ||
  PREC_AND,        // && 
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
// Optional consume, does not report error if did not consume
static bool opt_consume(ch_compilation *comp, ch_token_kind kind, ch_token *out_token);

static void error(ch_compilation *comp, const char *error_message, ...);
static void synchronize_outside_function(ch_compilation *comp);
static void synchronize_in_function(ch_compilation *comp);

static void free_compiler(ch_compilation *comp);
static ch_dataptr emit_string(ch_compilation *comp, const char *value,
                              size_t size);
static ch_jmpptr emit_jump(ch_compilation *comp, ch_op jump_instruction);
static void patch_jump(ch_compilation *comp, ch_jmpptr patch_address);

static void emit_loop(ch_compilation *comp, ch_jmpptr offset);
static ch_jmpptr record_loop(ch_compilation *comp);

static void statement(ch_compilation *comp);
static void function(ch_compilation *comp);
static void closure(ch_compilation *comp);
static ch_argcount function_arglist(ch_compilation *comp);

/*
    Identifiers are handled differently when read as a standalone statement VS
   when they're part of an expression. When read as a standalone statement, we
   expect the identifier to always be invoked, but in an expression an
   identifier could simply refer to a variable.
*/
static void statement_identifier(ch_compilation *comp);
static void expression_identifier(ch_compilation *comp);
static void prefix_identifier(ch_compilation *comp);
static void postfix_identifier(ch_compilation* comp, ch_token name);
static void identifier(ch_compilation *comp, bool must_have_invocation);

static void declaration(ch_compilation *comp);
static void scope(ch_compilation *comp);
static void function_statement(ch_compilation *comp);
static void assignement(ch_compilation* comp, ch_lexeme name);

static void semicolon(ch_compilation* comp);

static void if_statement(ch_compilation* comp);
static void while_statement(ch_compilation* comp);
static void do_while_statement(ch_compilation* comp);
static void for_statement(ch_compilation* comp);

static ch_scope new_localscope();
static uint8_t begin_scope(ch_compilation *comp);
static void end_scope(ch_compilation *comp, uint8_t parent_scope_size);
static bool scope_lookup(ch_scope *scope, ch_lexeme name, uint8_t *offset);
static bool upvalue_lookup(ch_compilation *comp, ch_scope* scope, ch_lexeme name, uint8_t *index);
static void add_variable(ch_compilation *comp, ch_lexeme name);
static void add_local(ch_compilation *comp, ch_lexeme name);
// Returns the upvalue's index
static uint8_t add_upvalue(ch_compilation *comp, ch_scope* scope, uint8_t offset, bool is_local);
static void add_global(ch_compilation *comp, ch_lexeme name);
static void load_variable(ch_compilation *comp, ch_lexeme name);
static void set_variable(ch_compilation* comp, ch_lexeme name);

// Expression parsing
static void parse(ch_compilation *comp, ch_precedence_level prec);
static void grouping(ch_compilation *comp);
static void expression(ch_compilation *comp);
static void binary(ch_compilation *comp);
static void unary(ch_compilation *comp);
static void return_statement(ch_compilation *comp);
static void invocation(ch_compilation *comp, ch_lexeme name);
static ch_argcount invocation_arguments(ch_compilation* comp);
static void number(ch_compilation *comp);
static void string(ch_compilation* comp);
static void boolean(ch_compilation* comp);
static void expression_char(ch_compilation* comp);
static void expression_null(ch_compilation* comp);
static void and(ch_compilation* comp);
static void or(ch_compilation* comp);

ch_parse_rule rules[NUM_TOKENS] = {
    [TK_POPEN] = {PREC_NONE, grouping, NULL},
    [TK_MINUS] = {PREC_TERM, unary, binary},
    [TK_PLUS] = {PREC_TERM, NULL, binary},
    [TK_FSLASH] = {PREC_FACTOR, NULL, binary},
    [TK_STAR] = {PREC_FACTOR, NULL, binary},
    [TK_NUM] = {PREC_NONE, number, NULL},
    [TK_STRING] = {PREC_NONE, string, NULL},
    [TK_ID] = {PREC_NONE, expression_identifier, NULL},
    [TK_AND] = {PREC_AND, NULL, and},
    [TK_OR] = {PREC_OR, NULL, or},
    [TK_FALSE] = {PREC_NONE, boolean, NULL},
    [TK_TRUE] = {PREC_NONE, boolean, NULL},
    [TK_CHAR] = {PREC_NONE, expression_char, NULL},
    [TK_NULL] = {PREC_NONE, expression_null, NULL},
    [TK_PLUS_PLUS] = {PREC_NONE, prefix_identifier, NULL},
    [TK_MINUS_MINUS] = {PREC_NONE, prefix_identifier, NULL},
};

const ch_parse_rule *get_rule(ch_token_kind kind);

bool ch_compile(const uint8_t *program, size_t program_size,
                ch_program *output) {
  ch_emit_scope global_emit_scope;
  ch_scope global_scope = new_localscope();
  ch_compilation comp = {
      .token_state = init_token(program, program_size),
      .scope = &global_scope,
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

  EMIT_OP(GET_EMIT(&comp), OP_BEGIN);

  ch_dataptr program_start_ptr = ch_emit_commit_scope(&comp.emit);

  *output = ch_emit_assemble(GET_EMIT(&comp), program_start_ptr);

  free_compiler(&comp);

  return !comp.has_errors;
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
    bool result = opt_consume(comp, kind, out_token);
    if (!result) {
      error(comp, error_message);
    }

    return result;
}

bool opt_consume(ch_compilation *comp, ch_token_kind kind, ch_token *out_token) {
  if (comp->is_panic)
    return false;

  if (comp->current.kind == kind) {
    advance(comp);

    if (out_token != NULL) {
      *out_token = comp->previous;
    }

    return true;
  }

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

  advance(comp);

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
      free((char*) entry.key->value);
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

  char* copied_value = (char*) malloc(size + 1);
  memcpy(copied_value, value, size);
  copied_value[size] = '\0';

  ch_string *key = new_string(copied_value, size);
  ch_table_set(&comp->strings, key, MAKE_NUMBER(string_ptr));

  return string_ptr;
}

ch_jmpptr emit_jump(ch_compilation *comp, ch_op jump_instruction) {
  EMIT_OP(GET_EMIT(comp), jump_instruction);
  EMIT_PTR(GET_EMIT(comp), 0)

  ch_blob* bytecode = &GET_EMIT(comp)->emit_scope->bytecode;
  return CH_BLOB_CONTENT_SIZE(bytecode) - sizeof(ch_jmpptr);
}

void patch_jump(ch_compilation *comp, ch_jmpptr patch_address) {
  ch_blob* bytecode = &GET_EMIT(comp)->emit_scope->bytecode;
  ch_jmpptr offset = CH_BLOB_CONTENT_SIZE(bytecode) - patch_address - sizeof(ch_jmpptr);

  ch_emit_patch_ptr(GET_EMIT(comp), offset, patch_address);
}

void emit_loop(ch_compilation *comp, ch_jmpptr offset) {
  EMIT_OP(GET_EMIT(comp), OP_JMP);
  EMIT_PTR(GET_EMIT(comp), 0);

  ch_blob* bytecode = &GET_EMIT(comp)->emit_scope->bytecode;
  ch_jmpptr current = CH_BLOB_CONTENT_SIZE(bytecode);
  ch_jmpptr real_offset = (current - offset);

  ch_emit_patch_ptr(GET_EMIT(comp), -real_offset, current - sizeof(ch_jmpptr));
}

ch_jmpptr record_loop(ch_compilation *comp) {
  ch_blob* bytecode = &GET_EMIT(comp)->emit_scope->bytecode;
  return CH_BLOB_CONTENT_SIZE(bytecode);
}

void statement(ch_compilation *comp) {
  ch_token_kind kind = comp->current.kind;

  switch (comp->current.kind) {
  case TK_VAL: {
    advance(comp);
    declaration(comp);
    semicolon(comp);
    break;
  }
  case TK_POUND: {
    function(comp);
    break;
  }
  default: {
    error(comp, "Expected statement.");
    return;
  }
  }
}

void function(ch_compilation *comp) {
  ch_scope locals_scope = new_localscope();
  locals_scope.parent = comp->scope;
  comp->scope = &locals_scope;

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

  if (!CH_EMITTING_GLOBALLY(GET_EMIT(comp))) {
    closure(comp);
  }

  comp->scope = comp->scope->parent;
  add_variable(comp, name.lexeme);
}

void closure(ch_compilation *comp) {
  EMIT_OP(GET_EMIT(comp), OP_CLOSURE);
  EMIT_ARGCOUNT(GET_EMIT(comp), comp->scope->upvalue_count);

  for(uint8_t i = 0; i < comp->scope->upvalue_count; i++) {
    ch_comp_upvalue* upvalue = &comp->scope->upvalues[i];
    EMIT_ARGCOUNT(GET_EMIT(comp), upvalue->is_local ? 1 : 0);
    EMIT_ARGCOUNT(GET_EMIT(comp), upvalue->offset);
  }
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

void prefix_identifier(ch_compilation *comp) {
  ch_token token = comp->previous;
  ch_token identifier;
  if (!consume(comp, TK_ID, "Expected identifier", &identifier)) return;

  load_variable(comp, identifier.lexeme);

  if (token.kind == TK_PLUS_PLUS) {
    EMIT_OP(GET_EMIT(comp), OP_ADDONE);
  } else {
    EMIT_OP(GET_EMIT(comp), OP_SUBONE);
  }

  // Duplicate its value so that the new value stays on the stack
  EMIT_OP(GET_EMIT(comp), OP_TOP);
  set_variable(comp, identifier.lexeme);
  return;
}

void identifier(ch_compilation *comp, bool is_statement) {
  ch_token name = comp->previous;

  switch(comp->current.kind) {
    case TK_POPEN: {
      invocation(comp, name.lexeme);
      break;
    }
    case TK_EQ: {
      assignement(comp, name.lexeme);
      break;
    }
    default: {
      load_variable(comp, name.lexeme);
      postfix_identifier(comp, name);

      if (is_statement) {
        EMIT_OP(GET_EMIT(comp), OP_POP);
      }
    }
  }
}

void postfix_identifier(ch_compilation* comp, ch_token name) {
  // The identifier should already be loaded on the stack when this function is called
  ch_token operator;
  if (opt_consume(comp, TK_PLUS_PLUS, &operator) || opt_consume(comp, TK_MINUS_MINUS, &operator)) {
    EMIT_OP(GET_EMIT(comp), OP_TOP);
    if (operator.kind == TK_PLUS_PLUS) {
      EMIT_OP(GET_EMIT(comp), OP_ADDONE);
    } else {
      EMIT_OP(GET_EMIT(comp), OP_SUBONE);
    }
    set_variable(comp, name.lexeme);
  }
}

void invocation(ch_compilation *comp, ch_lexeme name) {
  consume(comp, TK_POPEN, "Expected start of invocation", NULL);
  ch_argcount argcount = invocation_arguments(comp);
  consume(comp, TK_PCLOSE, "Expected end of arguments list.", NULL);

  load_variable(comp, name);

  EMIT_OP(GET_EMIT(comp), OP_CALL);
  EMIT_ARGCOUNT(GET_EMIT(comp), argcount);
}

ch_argcount invocation_arguments(ch_compilation* comp) {
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
  }

  return argcount;
}

void declaration(ch_compilation *comp) {
  ch_token name;
  if (!consume(comp, TK_ID, "Expected variable name.", &name))
    return;

  /*
    We still want the variable to be registered even if the user forgot its initializer.
    That way, it won't cause another error where the variable is said not to exist even if it was declared.
  */
  if(consume(comp, TK_EQ, "Expected variable initializer.", NULL)) {
    expression(comp);
  }

  add_variable(comp, name.lexeme);
}

void add_variable(ch_compilation *comp, ch_lexeme name) {
  if (comp->scope->locals_size == UINT8_MAX) {
    error(comp, "Exceeded variable limit in scope.");
    return;
  }

  if (scope_lookup(comp->scope, name, NULL)) {
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
  ch_local *local = &comp->scope->locals[comp->scope->locals_size++];
  local->name = name;
  local->is_captured = false;
}

uint8_t add_upvalue(ch_compilation *comp, ch_scope * scope, uint8_t offset, bool is_local) {
  uint8_t upvalue = scope->upvalue_count;

  for(uint8_t i = 0; i < scope->upvalue_count; i++) {
    ch_comp_upvalue* upvalue = &scope->upvalues[i];
    if (upvalue->offset == offset && upvalue->is_local == is_local) return i;
  }
  
  if(upvalue == UINT8_MAX) {
    error(comp, "Too many upvalues in current scope");
    return 0;
  }

  scope->upvalues[upvalue].is_local = is_local;
  scope->upvalues[upvalue].offset = offset;

  return scope->upvalue_count++;
}

void add_global(ch_compilation *comp, ch_lexeme name) {
  EMIT_OP(GET_EMIT(comp), OP_DEFINE_GLOBAL);
  ch_dataptr string_ptr = emit_string(comp, name.start, name.size);
  EMIT_PTR(GET_EMIT(comp), string_ptr);
}

void load_variable(ch_compilation *comp, ch_lexeme name) {
  uint8_t offset;

  if (scope_lookup(comp->scope, name, &offset)) {
    EMIT_OP(GET_EMIT(comp), OP_LOAD_LOCAL);
    EMIT_PTR(GET_EMIT(comp), offset);
  } else if(upvalue_lookup(comp, comp->scope, name, &offset)) {
    EMIT_OP(GET_EMIT(comp), OP_LOAD_UPVALUE);
    EMIT_ARGCOUNT(GET_EMIT(comp), offset);
  } else {
    EMIT_OP(GET_EMIT(comp), OP_LOAD_GLOBAL);
    ch_dataptr string_ptr = emit_string(comp, name.start, name.size);
    EMIT_PTR(GET_EMIT(comp), string_ptr);
  }
}

void set_variable(ch_compilation* comp, ch_lexeme name) {
  uint8_t offset;

  if (scope_lookup(comp->scope, name, &offset)) {
    EMIT_OP(GET_EMIT(comp), OP_SET_LOCAL);
    EMIT_PTR(GET_EMIT(comp), offset);
  } else if(upvalue_lookup(comp, comp->scope, name, &offset)) {
    EMIT_OP(GET_EMIT(comp), OP_SET_UPVALUE);
    EMIT_ARGCOUNT(GET_EMIT(comp), offset);
  } else {
    EMIT_OP(GET_EMIT(comp), OP_SET_GLOBAL);
    ch_dataptr string_ptr = emit_string(comp, name.start, name.size);
    EMIT_PTR(GET_EMIT(comp), string_ptr);
  }
}

void scope(ch_compilation *comp) {
  consume(comp, TK_COPEN, "Expected start of scope.", NULL);

  while (comp->current.kind != TK_CCLOSE && comp->current.kind != TK_EOF) {
    function_statement(comp);
  }

  consume(comp, TK_CCLOSE, "Expected end of scope.", NULL);
}

bool scope_lookup(ch_scope *scope, ch_lexeme name, uint8_t *offset) {
  if (scope->locals_size == 0) {
    return false;
  }

  for (int32_t i = scope->locals_size - 1; i >= 0; i--) {
    ch_lexeme *value_name = &scope->locals[i].name;
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

bool upvalue_lookup(ch_compilation* comp, ch_scope *scope, ch_lexeme name, uint8_t *index) {
  if (scope->parent == NULL) return false;

  uint8_t offset;
  if(scope_lookup(scope->parent, name, &offset)) {
    *index = add_upvalue(comp, scope, offset, true);
    scope->parent->locals[offset].is_captured = true;
    return true;
  }

  uint8_t upvalue;
  if(upvalue_lookup(comp, scope->parent, name, &upvalue)) {
    *index = add_upvalue(comp, scope, upvalue, false);
    return true;
  }

  return false;
}

void function_statement(ch_compilation *comp) {
  ch_token_kind kind = comp->current.kind;

  switch (comp->current.kind) {
  case TK_VAL: {
    advance(comp);
    declaration(comp);
    semicolon(comp);
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
    semicolon(comp);
    break;
  }
  case TK_POUND: {
    function(comp);
    break;
  }
  case TK_ID: {
    statement_identifier(comp);
    semicolon(comp);
    break;
  }
  case TK_IF: {
    if_statement(comp);
    break;
  }
  case TK_WHILE: {
    while_statement(comp);
    break;
  }
  case TK_DO: {
    do_while_statement(comp);
    opt_consume(comp, TK_SEMI, NULL);
    break;
  }
  case TK_FOR: {
    for_statement(comp);
    break;
  }
  default:
    error(comp, "Expected statement.");
  }

  if (comp->is_panic)
    synchronize_in_function(comp);
}

void assignement(ch_compilation* comp, ch_lexeme name) {
  if(!consume(comp, TK_EQ, "Expected assignement.", NULL)) {
    return;
  }

  expression(comp);

  set_variable(comp, name);
}

void semicolon(ch_compilation* comp) {
  consume(comp, TK_SEMI, "Expected semicolon.", NULL);
}

void if_statement(ch_compilation* comp) {
  consume(comp, TK_IF, "Expected if statement", NULL);
  consume(comp, TK_POPEN, "Expected opening parenthesis", NULL);
  expression(comp);
  consume(comp, TK_PCLOSE, "Expected closing parenthesis", NULL);

  ch_dataptr false_branch_patch = emit_jump(comp, OP_JMP_FALSE);
  scope(comp);

  if (!opt_consume(comp, TK_ELSE, NULL)) {
    patch_jump(comp, false_branch_patch);
    return;
  }
 
  // If we have an else section, we emit a jump so that the "true" branch can skip over the "false" branch
  ch_dataptr true_branch_patch = emit_jump(comp, OP_JMP);
  patch_jump(comp, false_branch_patch);
  scope(comp);
  patch_jump(comp, true_branch_patch);
}

void while_statement(ch_compilation* comp) {
  consume(comp, TK_WHILE, "Expected while statement", NULL);

  consume(comp, TK_POPEN, "Expected opening parenthesis", NULL);
  ch_jmpptr loop = record_loop(comp);
  expression(comp);
  ch_jmpptr false_branch = emit_jump(comp, OP_JMP_FALSE);
  EMIT_OP(GET_EMIT(comp), OP_POP);
  consume(comp, TK_PCLOSE, "Expected opening parenthesis", NULL);

  uint8_t scope_mark = begin_scope(comp);
  scope(comp);
  end_scope(comp, scope_mark);

  emit_loop(comp, loop);
  patch_jump(comp, false_branch);
}

void do_while_statement(ch_compilation* comp) {
  consume(comp, TK_DO, "Expected do while statement", NULL);
  uint8_t scope_mark = begin_scope(comp);
  ch_jmpptr loop_jump = record_loop(comp);
  scope(comp);
  end_scope(comp, scope_mark);

  consume(comp, TK_WHILE, "Expected while keyword", NULL);
  consume(comp, TK_POPEN, "Expected opening parenthesis", NULL);
  expression(comp);
  uint8_t exit_jump = emit_jump(comp, OP_JMP_FALSE);
  EMIT_OP(GET_EMIT(comp), OP_POP);
  emit_loop(comp, loop_jump);
  patch_jump(comp, exit_jump);

  consume(comp, TK_PCLOSE, "Expected closing parenthesis", NULL);
}

void for_statement(ch_compilation* comp) {
  uint8_t scope_mark = begin_scope(comp);
  consume(comp, TK_FOR, "Expected for statement", NULL);
  consume(comp, TK_POPEN, "Expected opening parenthesis", NULL);

  /*
    The bytecode of the loop is laid out as follows:
    1. initializer
    2. condition
    3. increment
    4. body
    5. exit

    Jumps required:
    J1: 2 -> 5 (false branch)
    J2: 2 -> 4 (true branch)
    J3: 3 -> 2 (check condition after increment)
    J4: 4 -> 3 (increment after executing body)

    Jumps J1, J2, J3 are only emitted if a condition is specified
  */

  // Initializer
  if (opt_consume(comp, TK_SEMI, NULL)) {
    // No initializer, do nothing
  } else if (opt_consume(comp, TK_VAL, NULL)) {
    declaration(comp);
    semicolon(comp);
  }

  ch_jmpptr loop_start = record_loop(comp);

  bool has_condition = false;
  ch_jmpptr exit_jump;
  // Condition
  if (opt_consume(comp, TK_SEMI, NULL)) {
    // No condition, do nothing
  } else {
    has_condition = true;
    expression(comp);
    semicolon(comp);
    exit_jump = emit_jump(comp, OP_JMP_FALSE);
    EMIT_OP(GET_EMIT(comp), OP_POP);
  }

  // Increment
  if (!opt_consume(comp, TK_PCLOSE, NULL)) {
    ch_jmpptr body_jump = emit_jump(comp, OP_JMP);
    ch_jmpptr increment_start = record_loop(comp);
    expression(comp);
    // Since we only care about the expression's side-effect, we pop its value
    EMIT_OP(GET_EMIT(comp), OP_POP);
    consume(comp, TK_PCLOSE, "Expected closing parenthesis", NULL);

    emit_loop(comp, loop_start);
    // Since the increment is optional, the body will either jump to increment start or condition start
    loop_start = increment_start;
    patch_jump(comp, body_jump);
  }
  
  scope(comp);
  emit_loop(comp, loop_start);

  if (has_condition) {
    patch_jump(comp, exit_jump);
    EMIT_OP(GET_EMIT(comp), OP_POP);
  }

  end_scope(comp, scope_mark);
}

ch_scope new_localscope() {
  return (ch_scope) {
    .locals_size=0,
    .upvalue_count=0,
    .parent=NULL,
  };
}

uint8_t begin_scope(ch_compilation *comp) { return comp->scope->locals_size; }

void end_scope(ch_compilation *comp, uint8_t last_scope_size) {
  if (last_scope_size != comp->scope->locals_size) {
    uint8_t num_values_popped = comp->scope->locals_size - last_scope_size;

    for(uint8_t i = 0; i < num_values_popped; i++) {
      if (comp->scope->locals[comp->scope->locals_size - i - 1].is_captured) {
        EMIT_OP(GET_EMIT(comp), OP_CLOSE_UPVALUE);
      } else {
        EMIT_OP(GET_EMIT(comp), OP_POP);
      }
    }

    comp->scope->locals_size = last_scope_size;
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

void number(ch_compilation *comp) {
  const char *start = comp->previous.lexeme.start;

  double value = strtod(start, NULL);
  ch_dataptr value_ptr = EMIT_DATA_DOUBLE(GET_EMIT(comp), value);

  EMIT_OP(GET_EMIT(comp), OP_NUMBER);
  EMIT_PTR(GET_EMIT(comp), value_ptr);
}

void string(ch_compilation* comp) {
  ch_token string = comp->previous;

  // We strip the string of escape chars
  char cleaned_string[string.lexeme.size];
  uint32_t output_i = 0;
  for(uint32_t i = 0; i < string.lexeme.size; i++) {
    if(string.lexeme.start[i] == '\\') i++;
    cleaned_string[output_i++] = string.lexeme.start[i]; 
  }

  EMIT_OP(GET_EMIT(comp), OP_STRING);
  // TODO make emit_string copy cleaned_string, because the buffer becomes out of scope once the function returns
  ch_dataptr string_ptr = emit_string(comp, cleaned_string, output_i);
  EMIT_PTR(GET_EMIT(comp), string_ptr);
}

void boolean(ch_compilation* comp) {
  ch_token_kind kind = comp->previous.kind;

  switch(kind) {
    case TK_TRUE: 
      EMIT_OP(GET_EMIT(comp), OP_TRUE);
      return;
    case TK_FALSE: 
      EMIT_OP(GET_EMIT(comp), OP_FALSE);
      return;
    default:
      error(comp, "Expected boolean expression");
  }
}

void expression_char(ch_compilation* comp) {
  ch_lexeme value = comp->previous.lexeme;
  
  EMIT_OP(GET_EMIT(comp), OP_CHAR);
  // 2 characters means the char expression is ''
  // In that case, we emit a null byte
  if (value.size == 2) {
      EMIT_OP(GET_EMIT(comp), 0);
      return;
  }

  EMIT_OP(GET_EMIT(comp), value.start[1]);
}

void expression_null(ch_compilation* comp) {
  EMIT_OP(GET_EMIT(comp), OP_NULL);
}

void and(ch_compilation* comp) {
  ch_dataptr patch = emit_jump(comp, OP_JMP_FALSE); // If false, no need to evaluate the next expression
  EMIT_OP(GET_EMIT(comp), OP_POP);
  parse(comp, PREC_AND);
  patch_jump(comp, patch);
}

void or(ch_compilation* comp) {
  ch_dataptr patch_false = emit_jump(comp, OP_JMP_FALSE);  
  ch_dataptr patch_true = emit_jump(comp, OP_JMP); // If true, no need to evaluate the next expression
  patch_jump(comp, patch_false);
  EMIT_OP(GET_EMIT(comp), OP_POP);
  parse(comp, PREC_OR);
  patch_jump(comp, patch_true);
}