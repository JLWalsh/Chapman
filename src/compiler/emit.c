#include "emit.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define INITIAL_BLOB_SIZE 10000
#define BLOB_CONTENT_SIZE(blob_ptr) ((blob_ptr)->current - (blob_ptr)->start)

ch_dataptr format_ptr_in_little_endian(ch_dataptr ptr) {
    uint8_t* byte_ptr = (uint8_t*) &ptr;
    return (ch_dataptr) (byte_ptr[0] << 0 | byte_ptr[1] << 8 | byte_ptr[2] << 16 | byte_ptr[3] << 24);
}

ch_blob create_blob();
ch_dataptr write_blob(ch_blob* blob, void* data_start, size_t write_size);
void free_blob(ch_blob* blob);

ch_emit ch_emit_create() {
    return (ch_emit) {
        .function_scope=NULL,
        .data=create_blob(),
        .function_bytecode=create_blob(),
    };
}

void ch_emit_create_function(ch_emit* emit, ch_function_scope* out_function) {
    out_function->bytecode = create_blob();
    out_function->parent = emit->function_scope;

    emit->function_scope = out_function;
}

ch_dataptr ch_emit_commit_function(ch_emit* emit) {
    // TODO check if function_ptr exceeds UINT32_MAX (of ch_dataptr)
    // TODO check if function_scope is not NULL

    ch_blob* bytecode = &emit->function_scope->bytecode;
    ch_dataptr function_ptr = write_blob(&emit->function_bytecode, bytecode->start, BLOB_CONTENT_SIZE(bytecode));

    free_blob(bytecode);
    emit->function_scope = emit->function_scope->parent;

    return function_ptr;
}

ch_program ch_emit_assemble(ch_emit* emit) {
    // TODO ensure function_scope is NULL
    size_t data_size = emit->data.current - emit->data.start;
    size_t bytecode_size = emit->function_bytecode.current - emit->function_bytecode.start;
    size_t program_size = data_size + bytecode_size;

    uint8_t* program = (uint8_t*) malloc(program_size);
    memcpy(program, emit->data.start, data_size);
    memcpy(program + data_size, emit->function_bytecode.start, bytecode_size);

    free_blob(&emit->data);
    free_blob(&emit->function_bytecode);

    return (ch_program) {.start=program, .data_size=data_size, .total_size=program_size};
}

void ch_emit_op(ch_emit* emit, ch_op op) {
    // Using sizeof(ch_op) would be unreliable, as enum sizes are compiler-dependent
    write_blob(&emit->function_scope->bytecode, &op, 1);
}
void ch_emit_ptr(ch_emit* emit, ch_dataptr ptr) {
    uint8_t buffer[sizeof(ptr)];
    buffer[0] = (ptr & 0xff);
    buffer[1] = (ptr & 0xff00) >> 8;
    buffer[2] = (ptr & 0xff0000) >> 16;
    buffer[3] = (ptr & 0xff000000) >> 24;

    write_blob(&emit->function_scope->bytecode, buffer, sizeof(buffer));
}

void ch_emit_argcount(ch_emit* emit, ch_argcount argcount) {
    write_blob(&emit->function_scope->bytecode, &argcount, sizeof(argcount));
}

ch_dataptr ch_emit_double(ch_emit* emit, double value) {
    return write_blob(&emit->data, &value, sizeof(double));
}

ch_blob create_blob() {
    uint8_t* start = malloc(INITIAL_BLOB_SIZE);
    return (ch_blob) {
        .size=INITIAL_BLOB_SIZE,
        .start=start,
        .current=start,
    };
}

ch_dataptr write_blob(ch_blob* blob, void* data_start, size_t write_size) {
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

void free_blob(ch_blob* blob) {
    free(blob->start);

    blob->start = NULL;
    blob->current = NULL;
    blob->size = 0;
}