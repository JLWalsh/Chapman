#include "emit.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

ch_dataptr blob_write(ch_blob* blob, void* data_start, size_t write_size);
void free_blob(ch_blob* blob);

ch_program ch_assemble(ch_blob* data, ch_blob* bytecode) {
    size_t program_size = data->size + bytecode->size;

    uint8_t* program = (uint8_t*) malloc(program_size);
    memcpy(program, data->start, data->size);
    memcpy(program + data->size, bytecode->start, bytecode->size);

    free_blob(data);
    free_blob(bytecode);

    return (ch_program) {.start=program, .data_size=data->size, .total_size=program_size};
}

void ch_emit_op(ch_blob* blob, ch_op op) {
    // Using sizeof(ch_op) would be unreliable, as enum sizes are compiler-dependent
    ch_blob_write(blob, &op, 1);
}

void ch_emit_ptr(ch_blob* blob, ch_dataptr ptr) {
    // TODO account for endianess when writing
    ch_blob_write(blob, &ptr, sizeof(ch_dataptr));
}

ch_dataptr ch_emit_double(ch_blob* blob, double value) {
    return ch_blob_write(blob, &value, sizeof(double));
}

ch_blob ch_create_blob(size_t initial_size) {
    uint8_t* start = (uint8_t*)malloc(initial_size);
    return (ch_blob) {.start=start, .current=start, .size=initial_size};
}

ch_dataptr blob_write(ch_blob* blob, void* data_start, size_t write_size) {
    uint32_t new_size = (ptrdiff_t) (blob->current - blob->start) + write_size;
    // TODO reintroduce exceeded capacity check
    //if (new_size >= CH_DATAPTR_MAX) {
        //return CH_DATAPTR_NULL;
    //}

    if (new_size >= blob->size) {
        size_t max_new_size = MAX(blob->size * CH_BLOB_BUFFER_GROWTH_MULTIPLIER, new_size);
        realloc((void*) blob->start, MIN(CH_DATAPTR_MAX, max_new_size));
    }

    ch_dataptr write_ptr = blob->current - blob->start;
    memcpy(blob->current, data_start, write_size);
    blob->current += write_size;

    return write_ptr;
}

void free_blob(ch_blob* blob) {
    free(blob->start);

    blob->start = NULL;
    blob->current = NULL;
    blob->size = 0;
}