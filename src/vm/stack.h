#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "object.h"

#define CH_STACK_ADDR(stack_ptr) ((ch_stack_addr) ((stack_ptr)->current - (stack_ptr)->start))

typedef struct {
    ch_object* start;
    ch_object* end;
    ch_object* current;
} ch_stack;

typedef ptrdiff_t ch_stack_addr;

ch_stack ch_stack_create();

bool ch_stack_push(ch_stack* stack, ch_object entry);

bool ch_stack_pop(ch_stack* stack, ch_object* popped);

bool ch_stack_popn(ch_stack* stack, uint8_t n);

bool ch_stack_seekto(ch_stack* stack, ch_stack_addr addr);

bool ch_stack_copy(ch_stack* stack, uint8_t index);