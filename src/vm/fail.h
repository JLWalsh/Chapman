#pragma once

typedef enum {
    FAIL_NONE,
    FAIL_STACK_SIZE_EXCEEDED,
    FAIL_STACK_EMPTY,
    FAIL_PROGRAM_OUT_OF_INSTRUCTIONS,
} ch_fail;