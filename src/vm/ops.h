#pragma once

typedef enum {
  OP_HALT,
  OP_POP,
  OP_POPN,
  OP_TOP,

  OP_NUMBER,
  OP_NEGATE,
  OP_ADD,
  OP_ADDONE,
  OP_SUB,
  OP_SUBONE,
  OP_MUL,
  OP_DIV,

  OP_STRING,
  OP_FALSE,
  OP_TRUE,
  OP_CHAR,
  OP_NULL,

  OP_LOAD_LOCAL,
  OP_SET_LOCAL,
  OP_LOAD_UPVALUE,
  OP_SET_UPVALUE,
  OP_CLOSE_UPVALUE,
  OP_SET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_LOAD_GLOBAL,

  OP_BEGIN, // Tells the VM that it may invoke the main function (ex. after all globals are setup)
  OP_CALL,
  OP_RETURN_VOID,
  OP_RETURN_VALUE,
  OP_JMP_FALSE, // Jump if false
  OP_JMP,

  OP_FUNCTION,
  OP_CLOSURE,
  OP_NATIVE,

  NUMBER_OF_OPCODES,
} ch_op;