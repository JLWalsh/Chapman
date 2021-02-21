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

typedef struct {
    ch_blob data;
    ch_blob bytecode;
} ch_emit;

typedef struct {
    uint8_t* start;
    size_t size;
} ch_program;

ch_emit ch_emit_create();

// Assembles the final program
ch_program ch_emit_assemble(ch_emit* emit);

ch_dataptr ch_emit_data(ch_emit* emit, void* data_start, size_t data_size);

void ch_emit_op(ch_emit* emit, ch_op op);

void ch_emit_ptr(ch_emit* emit, ch_dataptr ptr);