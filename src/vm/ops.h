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

    OP_LOAD_LOCAL,

    OP_CALL,
    OP_RETURN_VOID,
    OP_RETURN_VALUE,

    OP_DEBUG,
} ch_op;