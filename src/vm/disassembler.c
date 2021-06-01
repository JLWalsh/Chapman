#include "disassembler.h"
#include "bytecode.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define NAME(op, name) [op] = #name
#define LOAD_NUMBER(program, ptr) (*((double *)(&(program)->start[ptr])))

const char *OPCODE_NAMES[NUMBER_OF_OPCODES] = {
    NAME(OP_HALT, HALT),
    NAME(OP_POP, POP),
    NAME(OP_POPN, POPN),

    NAME(OP_NUMBER, NUMBER),
    NAME(OP_NEGATE, NEGATE),
    NAME(OP_ADD, ADD),
    NAME(OP_SUB, SUB),
    NAME(OP_MUL, MUL),
    NAME(OP_DIV, DIV),

    NAME(OP_LOAD_LOCAL, LOAD_LOCAL),
    NAME(OP_SET_GLOBAL, SET_GLOBAL),
    NAME(OP_LOAD_GLOBAL, LOAD_GLOBAL),

    NAME(OP_CALL, CALL),
    NAME(OP_RETURN_VOID, RETURN_VOID),
    NAME(OP_RETURN_VALUE, RETURN_VALUE),
    NAME(OP_FUNCTION, FUNCTION),
    NAME(OP_NATIVE, NATIVE),
};

void header(const char *name) { printf("--------- %s ---------\n", name); }

size_t print_double_ptr(const ch_program *program, uint8_t *i) {
  ch_dataptr ptr = READ_U32(i);
  double value = LOAD_NUMBER(program, ptr);

  printf("(ptr: %" PRIu32 " <%f>) ", ptr, value);

  return sizeof(ch_dataptr);
}

size_t print_ptr(const ch_program *program, uint8_t *i) {
  ch_dataptr ptr = READ_U32(i);
  printf("(ptr: %" PRIu32 ") ", ptr);

  return sizeof(ch_dataptr);
}

size_t print_hash(const ch_program *program, uint8_t *i) {
  uint32_t hash = READ_U32(i);
  printf("(hash: %" PRIu32 ") ", hash);

  return sizeof(uint32_t);
}

size_t print_function_ptr(const ch_program *program, uint8_t *i) {
  ch_dataptr ptr = READ_U32(i);
  ch_dataptr ptr_with_data_offset = ptr + program->data_size;
  printf("(ptr %" PRIu32 ") ", ptr_with_data_offset);

  return sizeof(uint32_t);
}

size_t print_string_ptr(const ch_program *program, uint8_t *i) {
  ch_dataptr ptr = READ_U32(i);
  ch_string *string = ch_bytecode_load_string(program, ptr);
  printf("(%s) ", string->value, ptr);

  ch_string_free(string);

  return sizeof(uint32_t);
}

size_t print_argcount(const ch_program *program, uint8_t *i) {
  ch_argcount argcount = *i;
  printf("(argc %" PRIu8 ") ", argcount);

  return sizeof(ch_argcount);
}

void ch_disassemble(const ch_program *program) {
  header("METADATA");

  printf("Program size: %zu b\n", program->total_size);
  printf("Data section size: %zu b\n", program->data_size);
  printf("Program section size: %zu b\n",
         program->total_size - program->data_size);
  printf("Program starts at: %zu b\n",
         program->data_size + program->program_start_ptr);

  header("INSTRUCTIONS");
  uint8_t *i = program->start + program->data_size;
  while (i < program->start + program->total_size) {
    uint32_t offset = i - program->start;
    uint8_t opcode = *i;
    i++;
    const char *opcode_name = OPCODE_NAMES[opcode];
    printf("%" PRIu32 ": %s ", offset, opcode_name);

    switch (opcode) {
    case OP_NUMBER: {
      i += print_double_ptr(program, i);
      break;
    }
    case OP_LOAD_LOCAL:
    case OP_POPN: {
      i += print_ptr(program, i);
      break;
    }
    case OP_FUNCTION: {
      i += print_function_ptr(program, i);
      i += print_argcount(program, i);
      break;
    }
    case OP_NATIVE:
    case OP_CALL: {
      i += print_argcount(program, i);
      break;
    }
    case OP_LOAD_GLOBAL:
    case OP_SET_GLOBAL: {
      i += print_string_ptr(program, i);
      break;
    }
    default:
      break;
    }
    printf("\n");
  }
}