#include "stack.h"
#include <stdlib.h>
#include <string.h>

#define ADDRESS_OK(stack_ptr, address) ((address) <= (stack_ptr)->size)
#define IS_FULL(stack_ptr) ((stack_ptr)->size == (stack_ptr)->max_size)

bool ch_stack_push(ch_stack *stack, ch_primitive entry) {
  if (stack->size >= stack->max_size) {
    return false;
  }

  stack->start[stack->size] = entry;
  stack->size++;

  return true;
}

bool ch_stack_pop(ch_stack *stack, ch_primitive *popped) {
  if (stack->size <= 0) {
    return false;
  }

  *popped = stack->start[stack->size - 1];
  stack->size--;

  return true;
}

bool ch_stack_copy(ch_stack *stack, ch_stack_addr index) {
  if (!ADDRESS_OK(stack, index) || IS_FULL(stack)) {
    return false;
  }

  stack->start[stack->size] = stack->start[index];
  stack->size++;

  return true;
}

bool ch_stack_popn(ch_stack *stack, uint8_t n) {
  if (stack->size < n) {
    return false;
  }

  stack->size -= n;

  return true;
}

bool ch_stack_seekto(ch_stack *stack, ch_stack_addr addr) {
  if (addr > stack->size) {
    return false;
  }

  stack->size = addr;

  return true;
}

ch_stack ch_stack_create() {
  // TODO make stack size configurable upon startup
  size_t max_size = 1000;
  size_t size = max_size * sizeof(ch_primitive);
  void *start = malloc(size);

  return (ch_stack){.start = start, .max_size = max_size, .size = 0};
}

void ch_stack_set(ch_stack *stack, ch_stack_addr addr, ch_primitive entry) {
  if (!ADDRESS_OK(stack, addr)) {
    // TODO signal error
    return;
  }

  stack->start[addr] = entry;
}

ch_primitive ch_stack_peek(ch_stack *stack, ch_stack_addr addr_from_top) {
  return stack->start[stack->size - addr_from_top - 1];
}