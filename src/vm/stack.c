#include "stack.h"
#include <string.h>
#include <stdlib.h>

bool ch_stack_push(ch_stack* stack, ch_object entry) {
    if (stack->current >= stack->end) {
        return false;
    }

    *stack->current = entry;
    stack->current++;

    return true;
}

bool ch_stack_pop(ch_stack* stack, ch_object* popped) {
    if (stack->current == stack->start) {
        return false;
    }

    *popped = *(stack->current - 1);
    stack->current--;

    return true;
}

bool ch_stack_copy(ch_stack* stack, uint8_t index) {
    ch_object* position = &stack->start[index];
    if (position < stack->start || position >= stack->current) {
        return false;
    }

    *stack->current = *(position);
    stack->current++;

    return true;
}

bool ch_stack_popn(ch_stack* stack, uint8_t n) {
    if (stack->current - stack->start < n) {
        return false;
    }

    stack->current -= n;

    return true;
}

ch_stack ch_stack_create() {
    // TODO make stack size configurable upon startup
    size_t size = 1000 * sizeof(ch_object);
    void* start = malloc(size);

    return (ch_stack) {
        .start=start,
        .end=start + size,
        .current=start,
    };
}