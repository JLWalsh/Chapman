#pragma once

typedef enum {
    NUMBER,

    // Do not add any enum values after this
    NUMBER_OF_PRIMITIVES,
} ch_primitive;

#define PRIMITIVE_SIZE(primitive) (PRIMITIVE_SIZES[primitive])

size_t PRIMITIVE_SIZES[NUMBER_OF_PRIMITIVES] = {
    [NUMBER] = sizeof(double)
};