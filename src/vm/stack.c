#include "stack.h"
#include <stdlib.h>
#include <string.h>

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

bool ch_stack_copy(ch_stack *stack, uint8_t index) {
  if (index >= stack->size || stack->size >= stack->max_size) {
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