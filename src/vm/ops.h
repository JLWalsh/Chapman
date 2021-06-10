#pragma once

typedef enum {
  OP_HALT,
  OP_POP,
  OP_POPN,

  OP_NUMBER,
  OP_NEGATE,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,

  OP_STRING,

  OP_LOAD_LOCAL,
  OP_SET_LOCAL,
  OP_SET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_LOAD_GLOBAL,

  OP_CALL,
  OP_RETURN_VOID,
  OP_RETURN_VALUE,

  OP_FUNCTION,
  OP_NATIVE,

  NUMBER_OF_OPCODES,
} ch_op;