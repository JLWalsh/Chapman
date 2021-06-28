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
  EXIT_GLOBAL_NOT_FOUND,
  EXIT_UNSUPPORTED_OPERATION,
  EXIT_USER_ERROR,
} ch_exit;

#define IS_PROGRAM_PTR_SAFE(context_ptr, program_ptr)                          \
  (program_ptr >= (context_ptr)->pstart && program_ptr < (context_ptr)->pend)

typedef struct {
  // The data section comes first, then the program section is after
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
  ch_closure* closure;
} ch_call;

#define CH_CALL_STACK_SIZE 256

typedef struct {
  ch_call calls[CH_CALL_STACK_SIZE];
  uint32_t size;
} ch_call_stack;

typedef struct ch_context {
  uint8_t *pstart;
  uint8_t *pend;
  uint8_t *pcurrent;
  size_t data_size;

  ch_stack stack;
  ch_call_stack call_stack;
  ch_exit exit;

  ch_upvalue* open_upvalues;
  ch_table globals;
  // For interned strings
  ch_table strings;
  ch_program program;

  ch_primitive program_return_value;
} ch_context;

ch_context ch_newvm(ch_program program);

void ch_freevm(ch_context *context);

ch_primitive ch_runfunction(ch_context *context, const char *function_name);

void ch_runtime_error(ch_context *context, ch_exit exit, const char *error,
                      ...);

// name_size should include null byte for strlen()
void ch_addnative(ch_context *context, ch_native_function function,
                  const char *name);

// All ch_popx functions will still modify the popped argument even if it doesn't match the requested type
// So before using the popped value, it's important to check the returned flag
bool ch_popnumber(ch_context* vm, double* popped);

bool ch_popstring(ch_context* vm, ch_string** popped);

ch_primitive ch_pop(ch_context *vm);

void ch_push(ch_context *vm, ch_primitive primitive);