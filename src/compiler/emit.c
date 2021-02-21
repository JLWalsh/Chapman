#include "emit.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

ch_blob ch_blob_init(size_t initial_size);
ch_dataptr ch_blob_write(ch_blob* blob, void* data_start, size_t write_size);

ch_emit ch_emit_create() {
    return (ch_emit) {.data=ch_blob_init(CH_BLOB_BUFFER_INITIAL_SIZE), .bytecode=ch_blob_init(CH_BLOB_BUFFER_INITIAL_SIZE)};
}

ch_program ch_emit_assemble(ch_emit* emit) {
    size_t data_size = emit->data.current - emit->data.start;
    size_t bytecode_size = emit->bytecode.current - emit->bytecode.start;
    size_t program_size = data_size + bytecode_size;

    uint8_t* program = (uint8_t*) malloc(program_size);
    memcpy(program, emit->data.start, data_size);
    memcpy(program + data_size, emit->bytecode.start, bytecode_size);

    free(emit->bytecode.start);
    free(emit->data.start);

    emit->bytecode.start = NULL;
    emit->data.start = NULL;

    return (ch_program) {.start=program, .size=program_size};
}

ch_dataptr ch_emit_data(ch_emit* emit, void* data_start, size_t data_size) {
    return ch_blob_write(&emit->data, data_start, data_size);
}

void ch_emit_op(ch_emit* emit, ch_op op) {
    ch_blob_write(&emit->bytecode, &op, sizeof(ch_op));
}

void ch_emit_ptr(ch_emit* emit, ch_dataptr ptr) {
    // TODO account for endianess when writing
    ch_blob_write(&emit->bytecode, &ptr, sizeof(ch_dataptr));
}

ch_blob ch_blob_init(size_t initial_size) {
    uint8_t* start = (uint8_t*)malloc(initial_size);
    // The first byte is reserved so that null can be represented using index 0
    return (ch_blob) {.start=start, .current=start + 1, .size=initial_size};
}

ch_dataptr ch_blob_write(ch_blob* blob, void* data_start, size_t write_size) {
    uint32_t new_size = (ptrdiff_t) (blob->current - blob->start) + write_size;
    if (new_size >= CH_DATAPTR_MAX) {
        return CH_DATAPTR_NULL;
    }

    if (new_size >= blob->size) {
        size_t max_new_size = MAX(blob->size * CH_BLOB_BUFFER_GROWTH_MULTIPLIER, new_size);
        realloc((void*) blob->start, MIN(CH_DATAPTR_MAX, max_new_size));
    }

    ch_dataptr write_ptr = blob->current - blob->start;
    for(size_t i = 0; i < write_size; i++) {
        blob->current++[i] = ((uint8_t*) data_start)[i];
    }

    return write_ptr;
}
