#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "primitive.h"

typedef struct {
    uint8_t* start;
    uint8_t* end;
    uint8_t* current;
} ch_stack;

ch_stack ch_stack_create();

bool ch_stack_push(ch_stack* stack, void* value, ch_primitive primitive);