#pragma once
#include "defs.h"
#include "ops.h"
#include "stack.h"
#include "table.h"
#include <stdint.h>

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
  EXIT_GLOBAL_ALREADY_EXISTS,
} ch_exit;

#define IS_PROGRAM_PTR_SAFE(context_ptr, program_ptr)                          \
  (program_ptr >= (context_ptr)->pstart && program_ptr < (context_ptr)->pend)

typedef struct {
  // The data section comes first, then the program section after
  uint8_t *start;
  size_t data_size;
  size_t total_size;
  // Where the program setup is located, relative to the start of the bytecode
  // section
  ch_dataptr program_start_ptr;
} ch_program;

typedef struct {
  uint8_t *return_addr;
  ch_stack_addr stack_addr;
} ch_call;

#define CH_CALL_STACK_SIZE 256

typedef struct {
  ch_call calls[CH_CALL_STACK_SIZE];
  uint32_t size;
} ch_call_stack;

typedef struct {
  uint8_t *pstart;
  uint8_t *pend;
  uint8_t *pcurrent;
  size_t data_size;

  ch_stack stack;
  ch_call_stack call_stack;
  ch_exit exit;

  ch_table globals;
} ch_context;

typedef void (*ch_native_function)(ch_context* context);

void ch_run(ch_program program);

bool ch_add_native(ch_context* context, ch_native_function function, void* name, uint32_t name_size);

void ch_runtime_error(ch_context *context, ch_exit exit, const char *error,
                      ...);