#include "vm.h"
#include "bytecode.h"
#include "ops.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define VM_READ_PTR(context)                                                   \
  ((context)->pcurrent += sizeof(ch_dataptr),                                  \
   READ_U32((context)->pcurrent - sizeof(ch_dataptr)))

#define VM_READ_HASH(context)                                                  \
  ((context)->pcurrent += sizeof(uint32_t),                                    \
   READ_U32((context)->pcurrent - sizeof(uint32_t)))

#define VM_READ_ARGCOUNT(context)                                              \
  ((context)->pcurrent += sizeof(ch_argcount),                                 \
   READ_ARGCOUNT((context)->pcurrent - sizeof(ch_argcount)))

// TODO do memory bounds check?
#define LOAD_NUMBER(context, ptr) (*((double *)(&(context)->pstart[ptr])))

#define CURRENT_CALL(context_ptr)                                              \
  ((context_ptr)->call_stack.calls[(context_ptr)->call_stack.size - 1])

static void halt(ch_context *context, ch_exit reason) {
  context->exit = reason;
}

static void call(ch_context *context, ch_function *function,
                 ch_argcount argcount) {
  if (context->call_stack.size >= CH_CALL_STACK_SIZE) {
    ch_runtime_error(context, EXIT_STACK_SIZE_EXCEEDED, "Stack limit reached.");
    return;
  }

  if (argcount != function->argcount) {
    ch_runtime_error(
        context, EXIT_NOT_ENOUGH_ARGS_IN_STACK,
        "Incorrect number of arguments passed to function (expected %" PRIu8
        ", got %" PRIu8 ").",
        function->argcount, argcount);
    return;
  }

  uint8_t *function_ptr = context->pstart + function->ptr + context->data_size;
  if (!IS_PROGRAM_PTR_SAFE(context, function_ptr)) {
    ch_runtime_error(context, EXIT_INVALID_INSTRUCTION_POINTER,
                     "Function pointer exceeds bounds of program.");
    return;
  }

  ch_call *call = &context->call_stack.calls[context->call_stack.size++];
  call->return_addr = context->pcurrent;
  call->stack_addr = CH_STACK_ADDR(&context->stack) - function->argcount;

  context->pcurrent = function_ptr;
}

static void try_call(ch_context *context, ch_primitive primitive,
                     ch_argcount argcount) {
  if (!IS_OBJECT(primitive)) {
    ch_runtime_error(context, EXIT_INCORRECT_TYPE,
                     "Attempted to invoke type %d as a function.",
                     primitive.type);
    return;
  }

  ch_object *object = AS_OBJECT(primitive);

  if (IS_FUNCTION(object)) {
    call(context, AS_FUNCTION(object), argcount);
    return;
  }

  if (IS_NATIVE(object)) {
    ch_native *native = AS_NATIVE(object);
    native->function(context, argcount);
    return;
  }

  ch_runtime_error(context, EXIT_INCORRECT_TYPE,
                   "Attempted to invoke object type %d as a function.",
                   object->type);
}

static void call_return(ch_context *context) {
  ch_call *call = &context->call_stack.calls[--context->call_stack.size];
  context->pcurrent = call->return_addr;
  // TODO check result of call
  ch_stack_seekto(&context->stack, call->stack_addr);
}

ch_context ch_vm_newcontext(ch_program program) {
  ch_context context = {
      .pstart = program.start,
      .pend = program.start + program.total_size,
      // Seek after the data section
      .pcurrent = program.start + program.data_size + program.program_start_ptr,
      .data_size = program.data_size,
      .stack = ch_stack_create(),
      .call_stack =
          (ch_call_stack){
              .size = 0,
          },
      .exit = RUNNING,
      .program = program,
      .program_return_value = MAKE_NULL(),
  };

  ch_table_create(&context.globals);
  ch_table_create(&context.strings);

  return context;
}

void ch_vm_free(ch_context *context) {
  ch_table_free(&context->globals);
  ch_table_free(&context->strings);
}

#define STACK_PUSH(context_ptr, entry)                                         \
  if (!ch_stack_push(&(context_ptr)->stack, entry)) {                          \
    halt(context_ptr, EXIT_STACK_SIZE_EXCEEDED);                               \
    break;                                                                     \
  }

#define STACK_POP(context_ptr, value)                                          \
  if (!ch_stack_pop(&(context_ptr)->stack, value)) {                           \
    halt(context_ptr, EXIT_STACK_EMPTY);                                       \
    break;                                                                     \
  }

static void add_global(ch_context *context, ch_string *name,
                       ch_primitive value) {
  if (!ch_table_set(&context->globals, name, value))
    ch_runtime_error(context, EXIT_GLOBAL_ALREADY_EXISTS,
                     "Global variable has already been defined: %s.",
                     name->value);
}

static ch_string *read_string(ch_context *context) {
  ch_dataptr string_ptr = VM_READ_PTR(context);
  ch_bytecode_string string =
      ch_bytecode_load_string(&context->program, string_ptr);

  return ch_loadstring(context, string.value, string.size, false);
}

ch_primitive ch_vm_call(ch_context *context, ch_string *function_name) {
  while (context->exit == RUNNING) {
    uint8_t current = *(context->pcurrent);
    context->pcurrent++;

    switch (current) {
    case OP_NUMBER: {
      double value = LOAD_NUMBER(context, VM_READ_PTR(context));
      STACK_PUSH(context, MAKE_NUMBER(value));
      break;
    }
    case OP_ADD: {
      ch_primitive left;
      ch_primitive right;
      STACK_POP(context, &left);
      STACK_POP(context, &right);

      double result = left.number_value + right.number_value;
      STACK_PUSH(context, MAKE_NUMBER(result));
      break;
    }
    case OP_SUB: {
      ch_primitive left;
      ch_primitive right;
      STACK_POP(context, &left);
      STACK_POP(context, &right);

      double result = left.number_value - right.number_value;
      STACK_PUSH(context, MAKE_NUMBER(result));
      break;
    }
    case OP_MUL: {
      ch_primitive left;
      ch_primitive right;
      STACK_POP(context, &left);
      STACK_POP(context, &right);

      double result = left.number_value * right.number_value;
      STACK_PUSH(context, MAKE_NUMBER(result));
      break;
    }
    case OP_DIV: {
      ch_primitive left;
      ch_primitive right;
      STACK_POP(context, &left);
      STACK_POP(context, &right);

      double result = left.number_value / right.number_value;
      STACK_PUSH(context, MAKE_NUMBER(result));
      break;
    }
    case OP_HALT: {
      context->exit = EXIT_OK;
      break;
    }
    case OP_POP: {
      ch_primitive entry;
      STACK_POP(context, &entry);
      break;
    }
    case OP_POPN: {
      ch_dataptr num_popped = VM_READ_PTR(context);

      // TODO add runtime check?
      ch_stack_popn(&context->stack, num_popped);
      break;
    }
    case OP_LOAD_LOCAL: {
      uint8_t offset = (uint8_t)VM_READ_PTR(context);
      ch_stack_addr index = CURRENT_CALL(context).stack_addr + offset;
      ch_stack_copy(&context->stack, index);
      break;
    }
    case OP_FUNCTION: {
      ch_dataptr function_ptr = VM_READ_PTR(context);
      ch_argcount argcount = VM_READ_ARGCOUNT(context);
      ch_primitive function =
          MAKE_OBJECT(ch_loadfunction(function_ptr, argcount));
      STACK_PUSH(context, function);
      break;
    }
    case OP_CALL: {
      ch_argcount argcount = VM_READ_ARGCOUNT(context);

      ch_primitive function;
      STACK_POP(context, &function);

      try_call(context, function, argcount);
      break;
    }
    case OP_RETURN_VALUE: {
      ch_primitive returned_value;
      STACK_POP(context, &returned_value);

      call_return(context);

      // Check if we are returning from the main function
      if (context->call_stack.size != 0) {
        STACK_PUSH(context, returned_value);
      } else {
        context->program_return_value = returned_value;
      }

      break;
    }
    case OP_RETURN_VOID: {
      call_return(context);
      break;
    }
    case OP_SET_GLOBAL: {
      ch_string *name = read_string(context);

      ch_primitive entry;
      STACK_POP(context, &entry);

      add_global(context, name, entry);

      break;
    }
    case OP_LOAD_GLOBAL: {
      ch_string *name = read_string(context);

      ch_primitive *global = ch_table_get(&context->globals, name);
      if (global == NULL) {
        ch_runtime_error(context, EXIT_GLOBAL_NOT_FOUND,
                         "Global variable does not exist: %s.", name->value);
        break;
      }

      STACK_PUSH(context, *global);

      break;
    }
    default: {
      ch_runtime_error(context, EXIT_UNKNOWN_INSTRUCTION,
                       "Unknown instruction.");
      break;
    }
    }
  }

  if (context->exit != EXIT_OK) {
    printf("Runtime error: %d.\n", context->exit);
  } else {
    printf("Program has halted.\n");
  }

  return context->program_return_value;
}

bool ch_addnative(ch_context *context, ch_native_function function,
                  const char *name) {
  ch_string *s = ch_loadstring(context, name, strlen(name), true);
  ch_native *native = ch_loadnative(function);

  add_global(context, s, MAKE_OBJECT(native));
}

void ch_runtime_error(ch_context *context, ch_exit exit, const char *error,
                      ...) {
  va_list args;
  va_start(args, error);
  vfprintf(stderr, error, args);
  va_end(args);
  fputs("\n", stderr);

  context->exit = exit;
}
