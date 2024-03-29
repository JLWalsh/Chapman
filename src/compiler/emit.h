#pragma once
#include "emit.h"
#include <stdint.h>
#include <vm/chapman.h>

#define CH_BLOB_BUFFER_GROWTH_MULTIPLIER 2
#define CH_BLOB_BUFFER_INITIAL_SIZE 10000
#define CH_BLOB_CONTENT_SIZE(blob_ptr) ((blob_ptr)->current - (blob_ptr)->start)

/*
    The first scope is the global scope, and it has no parent since it's the
   first created
*/
#define CH_EMITTING_GLOBALLY(emit_ptr) ((emit_ptr)->emit_scope->parent == NULL)
#define GET_DATA(emit_ptr) (&(emit_ptr)->data)
#define GET_BYTECODE(emit_ptr) (&(emit_ptr)->emit_scope->bytecode)

#define EMIT_DATA(emit_ptr, value_ptr, size)                                   \
  ch_emit_write(GET_DATA(emit_ptr), value_ptr, size)
#define EMIT_BYTECODE(emit_ptr, value_ptr, size)                               \
  ch_emit_write(GET_BYTECODE(emit_ptr), value_ptr, size)

#define EMIT_PTR(emit_ptr, value)                                              \
  {                                                                            \
    uint8_t le_value[4];                                                       \
    ch_uint32_to_le_array(value, le_value);                                    \
    EMIT_BYTECODE(emit_ptr, &(le_value), sizeof(le_value));                    \
  }
#define EMIT_ARGCOUNT(emit_ptr, value)                                         \
  { \
    ch_argcount _argcount = (value); \
    EMIT_BYTECODE(emit_ptr, &(_argcount), sizeof(ch_argcount));\
  }
#define EMIT_OP(emit_ptr, value)                                               \
  {                                                                            \
    ch_op op = (value);                                                        \
    EMIT_BYTECODE(emit_ptr, &op, 1);                                           \
  }

#define EMIT_DATA_STRING(emit_ptr, string_ptr, string_size, out_dataptr)       \
  {                                                                            \
    uint8_t le_value[4];                                                       \
    ch_uint32_to_le_array((string_size), le_value);                            \
    out_dataptr = EMIT_DATA(emit_ptr, &(le_value), sizeof(le_value));          \
    EMIT_DATA(emit_ptr, (string_ptr), (string_size));                          \
    char null_byte = '\0'; \
    EMIT_DATA((emit_ptr), &null_byte, 1); \
  }

#define EMIT_DATA_DOUBLE(emit_ptr, value)                                      \
  EMIT_DATA(emit_ptr, &(value), sizeof(double))

typedef struct {
  uint8_t *start;
  uint8_t *current;
  size_t size;
} ch_blob;

typedef struct ch_emit_scope {
  ch_blob bytecode;
  struct ch_emit_scope *parent;
} ch_emit_scope;

typedef struct {
  ch_emit_scope *emit_scope;
  ch_blob function_bytecode;
  ch_blob data;
} ch_emit;

ch_emit ch_emit_create(ch_emit_scope *out_scope);

void ch_emit_create_scope(ch_emit *emit, ch_emit_scope *out_scope);

ch_dataptr ch_emit_commit_scope(ch_emit *emit);

ch_program ch_emit_assemble(ch_emit *emit, ch_dataptr program_start_ptr);

ch_dataptr ch_emit_write(ch_blob *emit, const void *value_ptr, size_t size);

void ch_emit_patch_ptr(ch_emit* emit, ch_dataptr ptr, ch_jmpptr patch_at);

// Convert a uint32_t to a uint8_t[4] array that contains the bytes in
// little-endian order
void ch_uint32_to_le_array(uint32_t value, uint8_t *out_array);