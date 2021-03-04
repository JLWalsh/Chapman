#pragma once
#include <stdint.h>
#include "stack.h"
#include "ops.h"

typedef enum {
    EXIT_OK,
    EXIT_UNKNOWN_INSTRUCTION,
    EXIT_STACK_SIZE_EXCEEDED,
    EXIT_STACK_EMPTY,
    EXIT_PROGRAM_OUT_OF_INSTRUCTIONS,
} ch_exit;

#define CH_DATAPTR_NULL 0
#define CH_DATAPTR_MAX UINT32_MAX
typedef uint32_t ch_dataptr;

typedef struct {
    uint8_t* start;
    size_t size;
} ch_program;

typedef struct {
    uint8_t* pstart;
    uint8_t* pend;
    uint8_t* pcurrent;

    ch_stack stack;
    ch_exit exit;
} ch_context;

void ch_run(ch_program program);