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

bool ch_stack_popn(ch_stack* stack, uint8_t n) {
    if (stack->current - stack->start < n) {
        return false;
    }

    stack->current -= n;

    return true;
}

ch_stack ch_stack_create() {
    // TODO make stack size configurable upon startup
    size_t size = 1000 * sizeof(ch_stack_entry);
    void* start = malloc(size);

    return (ch_stack) {
        .start=start,
        .end=start + size,
        .current=start,
    };
}

bool ch_stack_set_addr(ch_stack* stack, ch_stack_addr addr) {
    ch_stack_entry* resolved_address = stack->start + addr;
    if (resolved_address < stack->start || resolved_address >= stack->end) return false;

    stack->current = resolved_address;
    return true;
}

ch_stack_addr ch_stack_get_addr(ch_stack* stack) {
    return stack->current - stack->start;
}