#include "emit.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define INITIAL_BLOB_SIZE 10000
#define BLOB_CONTENT_SIZE(blob_ptr) ((blob_ptr)->current - (blob_ptr)->start)

ch_blob create_blob();
void free_blob(ch_blob* blob);

ch_emit ch_emit_create(ch_emit_scope* out_scope) {
    ch_emit emit = {
        .emit_scope=NULL,
        .data=create_blob(),
        .function_bytecode=create_blob(),
    };

    ch_emit_create_scope(&emit, out_scope);
    emit.emit_scope = out_scope;

    return emit;
}

void ch_emit_create_scope(ch_emit* emit, ch_emit_scope* out_scope) {
    out_scope->bytecode = create_blob();
    out_scope->parent = emit->emit_scope;

    emit->emit_scope = out_scope;
}

ch_dataptr ch_emit_commit_scope(ch_emit* emit) {
    // TODO check if function_ptr exceeds UINT32_MAX (of ch_dataptr)
    // TODO check if emit_scope is not NULL

    ch_blob* bytecode = &emit->emit_scope->bytecode;
    ch_dataptr function_ptr = ch_emit_write(&emit->function_bytecode, bytecode->start, BLOB_CONTENT_SIZE(bytecode));

    free_blob(bytecode);
    emit->emit_scope = emit->emit_scope->parent;

    return function_ptr;
}

ch_program ch_emit_assemble(ch_emit* emit, ch_dataptr program_start_ptr) {
    // TODO ensure emit_scope is NULL
    size_t data_size = emit->data.current - emit->data.start;
    size_t bytecode_size = emit->function_bytecode.current - emit->function_bytecode.start;
    size_t program_size = data_size + bytecode_size;

    uint8_t* program = (uint8_t*) malloc(program_size);
    memcpy(program, emit->data.start, data_size);
    memcpy(program + data_size, emit->function_bytecode.start, bytecode_size);

    free_blob(&emit->data);
    free_blob(&emit->function_bytecode);

    return (ch_program) {
        .start=program, 
        .data_size=data_size, 
        .total_size=program_size,
        .program_start_ptr=program_start_ptr
    };
}

ch_dataptr ch_emit_write(ch_blob* blob, const void* data_start, size_t write_size) {
    uint32_t new_size = BLOB_CONTENT_SIZE(blob) + write_size;
    // TODO reintroduce exceeded capacity check
    //if (new_size >= CH_DATAPTR_MAX) {
        //return CH_DATAPTR_NULL;
    //}

    if (new_size >= blob->size) {
        size_t max_new_size = MAX(blob->size * CH_BLOB_BUFFER_GROWTH_MULTIPLIER, new_size);
        // TODO check how realloc changes blob->start pointer
        blob->start = realloc((void*) blob->start, MIN(CH_DATAPTR_MAX, max_new_size));
        blob->current = blob->start + BLOB_CONTENT_SIZE(blob);
    }

    ch_dataptr write_ptr = blob->current - blob->start;
    memcpy(blob->current, data_start, write_size);
    blob->current += write_size;

    return write_ptr;
}

inline void ch_uint32_to_le_array(uint32_t value, uint8_t* out_array) {
    out_array[0] = (value & 0xff);
    out_array[1] = (value & 0xff00) >> 8;
    out_array[2] = (value & 0xff0000) >> 16;
    out_array[3] = (value & 0xff000000) >> 24;
}

ch_blob create_blob() {
    uint8_t* start = malloc(INITIAL_BLOB_SIZE);
    return (ch_blob) {
        .size=INITIAL_BLOB_SIZE,
        .start=start,
        .current=start,
    };
}

void free_blob(ch_blob* blob) {
    free(blob->start);

    blob->start = NULL;
    blob->current = NULL;
    blob->size = 0;
}