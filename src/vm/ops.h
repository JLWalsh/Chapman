#pragma once

typedef enum {
    OP_HALT,
    OP_POP,

    OP_NUMBER,
    OP_NEGATE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_LOAD_LOCAL,

    OP_DEBUG,
} ch_op;