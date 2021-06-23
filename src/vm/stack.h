#pragma once
#include "primitive.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CH_STACK_ADDR(stack_ptr) ((ch_stack_addr)((stack_ptr)->size))

typedef struct {
  ch_primitive *start;
  size_t max_size;
  size_t size;
} ch_stack;

typedef uint32_t ch_stack_addr;

ch_stack ch_stack_create();

bool ch_stack_push(ch_stack *stack, ch_primitive entry);

bool ch_stack_pop(ch_stack *stack, ch_primitive *popped);

bool ch_stack_popn(ch_stack *stack, uint8_t n);

bool ch_stack_seekto(ch_stack *stack, ch_stack_addr addr);

bool ch_stack_copy(ch_stack *stack, ch_stack_addr addr);

void ch_stack_set(ch_stack *stack, ch_stack_addr addr, ch_primitive entry);

ch_primitive* ch_stack_get(ch_stack *stack, ch_stack_addr addr);

ch_primitive ch_stack_peek(ch_stack *stack, ch_stack_addr addr_from_top);