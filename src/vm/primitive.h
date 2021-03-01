#pragma once
#include <stdlib.h>

typedef enum {
    NUMBER,

    // Do not add any enum values after this
    NUMBER_OF_PRIMITIVES,
} ch_primitive;

#define PRIMITIVE_SIZE(primitive) (PRIMITIVES[primitive].size)

typedef struct {
    size_t size;
} ch_primitive_table_entry;

ch_primitive_table_entry PRIMITIVES[NUMBER_OF_PRIMITIVES];