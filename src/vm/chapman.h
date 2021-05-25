#pragma once
#include <stdint.h>
#include "stack.h"
#include "ops.h"
#include "defs.h"

typedef enum {
    RUNNING,
    EXIT_OK,
    EXIT_UNKNOWN_INSTRUCTION,
    EXIT_STACK_SIZE_EXCEEDED,
    EXIT_STACK_EMPTY,
    EXIT_PROGRAM_OUT_OF_INSTRUCTIONS,
    EXIT_CALL_STACK_SIZE_EXCEEDED,
    EXIT_INVALID_INSTRUCTION_POINTER,
    EXIT_NOT_ENOUGH_ARGS_IN_STACK,
    EXIT_INCORRECT_TYPE,
} ch_exit;

#define IS_PROGRAM_PTR_SAFE(context_ptr, program_ptr) (program_ptr >= (context_ptr)->pstart && program_ptr < (context_ptr)->pend)

typedef struct {
    // The data section comes first, then the program section after
    uint8_t* start;
    size_t data_size;
    size_t total_size;
} ch_program;

typedef struct {
    uint8_t* return_addr;
    ch_stack_addr stack_addr;
} ch_call;

#define CH_CALL_STACK_SIZE 256

typedef struct {
    ch_call calls[CH_CALL_STACK_SIZE];
    uint32_t size;
} ch_call_stack;

typedef struct {
    uint8_t* pstart;
    uint8_t* pend;
    uint8_t* pcurrent;
    size_t data_size;

    ch_stack stack;
    ch_call_stack call_stack;
    ch_exit exit;
} ch_context;

void ch_run(ch_program program);

void ch_runtime_error(ch_context* context, ch_exit exit, const char* error, ...);