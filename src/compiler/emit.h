#pragma once
#include "emit.h"
#include <vm/chapman.h>
#include <stdint.h>

#define CH_BLOB_BUFFER_GROWTH_MULTIPLIER 2
#define CH_BLOB_BUFFER_INITIAL_SIZE 10000

#define CH_EMIT_IS_SCOPED(emit_ptr) ((emit_ptr)->function_scope != NULL)

typedef struct {
    uint8_t* start;
    uint8_t* current;
    size_t size;
} ch_blob;

typedef struct ch_function_scope {
    ch_blob bytecode;
    struct ch_function_scope* parent;
} ch_function_scope;

typedef struct {
    ch_function_scope* function_scope;
    ch_blob function_bytecode;
    ch_blob data;
} ch_emit;

ch_emit ch_emit_create();

void ch_emit_create_function(ch_emit* emit, ch_function_scope* out_function);

ch_dataptr ch_emit_commit_function(ch_emit* emit);

ch_program ch_emit_assemble(ch_emit* emit);

void ch_emit_op(ch_emit* emit, ch_op op);

void ch_emit_ptr(ch_emit* emit, ch_dataptr ptr);

void ch_emit_argcount(ch_emit* emit, ch_argcount argcount);

ch_dataptr ch_emit_double(ch_emit* emit, double value);