#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "primitive.h"

typedef struct {
    union {
        uint8_t data_start;

        double number_value;
    };
    ch_primitive primitive;
} ch_stack_entry;

typedef struct {
    ch_stack_entry* start;
    ch_stack_entry* end;
    ch_stack_entry* current;
} ch_stack;

typedef ptrdiff_t ch_stack_addr;

ch_stack ch_stack_create();

bool ch_stack_push(ch_stack* stack, ch_stack_entry entry);

bool ch_stack_pop(ch_stack* stack, ch_stack_entry* popped);

bool ch_stack_popn(ch_stack* stack, uint8_t n);

bool ch_stack_copy(ch_stack* stack, uint8_t index);

bool ch_stack_set_addr(ch_stack* stack, ch_stack_addr addr);

ch_stack_addr ch_stack_get_addr(ch_stack* stack);