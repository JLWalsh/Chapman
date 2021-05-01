#pragma once
#include <stdint.h>
#include "stack.h"
#include "ops.h"
#include "function.h"

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

#define CH_DATAPTR_NULL 0
#define CH_DATAPTR_MAX UINT32_MAX
typedef uint32_t ch_dataptr;
typedef uint8_t ch_argcount;

typedef struct {
    uint8_t* start;
    size_t size;
} ch_program;

typedef struct {
    uint8_t* call_ip;
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

    ch_stack stack;
    ch_call_stack call_stack;
    ch_exit exit;
} ch_context;

void ch_run(ch_program program);
