#pragma once
#include "emit.h"
#include <vm/chapman.h>
#include <stdint.h>

#define CH_BLOB_BUFFER_GROWTH_MULTIPLIER 2
#define CH_BLOB_BUFFER_INITIAL_SIZE 10000

typedef struct {
    uint8_t* start;
    uint8_t* current;
    size_t size;
} ch_blob;

ch_program ch_assemble(ch_blob* data, ch_blob* bytecode);

ch_blob ch_create_blob(size_t initial_size);

ch_blob ch_merge_blobs(ch_blob* blobs, uint8_t blob_count);

void ch_emit_op(ch_blob* blob, ch_op op);

void ch_emit_ptr(ch_blob* blob, ch_dataptr ptr);

ch_dataptr ch_emit_double(ch_blob* blob, double value);