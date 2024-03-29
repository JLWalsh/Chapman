#pragma once
#include "emit.h"
#include "token.h"
#include <stdint.h>
#include <vm/chapman.h>

#define MAX_FUNCTION_BLOBS 256

typedef struct {
  ch_lexeme name;
  bool is_captured;
} ch_local;

typedef struct {
  bool is_local;
  uint8_t offset;
} ch_comp_upvalue;

typedef struct ch_scope {
  ch_local locals[UINT8_MAX];
  uint8_t locals_size;

  uint8_t upvalue_count;
  ch_comp_upvalue upvalues[UINT8_MAX];

  struct ch_scope* parent;
} ch_scope;

typedef struct {
  ch_token_state token_state;
  ch_token previous;
  ch_token current;

  ch_scope* scope;

  /*
      is_panic is a state of the compiler, e.g. when an error is found it will
     be in panic state until it has a chance to synchronize has_errors is a flag
     that is set to true whenever an error is first encountered. This is used to
     signal to the user that there were compilation errors.
  */
  bool is_panic;
  bool has_errors;

  ch_emit emit;
  ch_table strings;
} ch_compilation;

bool ch_compile(const uint8_t *program, size_t program_size,
                ch_program *output);