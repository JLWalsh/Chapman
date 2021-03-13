#include "stack.h"
#include <string.h>
#include <stdlib.h>

/*
    The stack's elements can be of variable size.
    This is done by appending a 1 byte footer after every element pushed on the stack.
    This footer indicates the type of the element that was pushed.
    Since different types have different sizes, this information is
    used to know how many bytes to roll back the stack pointer when popping an element.
*/

bool ch_stack_push(ch_stack* stack, ch_stack_entry entry) {
    if (stack->current >= stack->end) {
        return false;
    }

    *stack->current = entry;
    stack->current++;

    return true;
}

bool ch_stack_pop(ch_stack* stack, ch_stack_entry* popped) {
    if (stack->current == stack->start) {
        return false;
    }

    *popped = *(stack->current - 1);
    stack->current--;

    return true;
}

bool ch_stack_copy(ch_stack* stack, uint8_t index) {
    ch_stack_entry* position = &stack->start[index];
    if (position < stack->start || position >= stack->current) {
        return false;
    }

    *stack->current = *(position);
    stack->current++;

    return true;
}

ch_stack ch_stack_create() {
    // TODO make stack size configurable upon startup
    size_t size = 1000 * sizeof(ch_stack_entry);
    uint8_t* start = malloc(size);

    return (ch_stack) {
        .start=start,
        .end=start + size,
        .current=start,
    };
}