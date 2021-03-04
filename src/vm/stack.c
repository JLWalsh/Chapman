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

#define FOOTER_SIZE 1

bool ch_stack_push(ch_stack* stack, void* value, ch_primitive primitive) {
    size_t size = PRIMITIVE_SIZE(primitive) + FOOTER_SIZE;
    if (stack->current + size > stack->end) {
        return false;
    }

    memcpy(stack->current, value, size);
    stack->current += size;

    *(stack->current - 1) = primitive;

    return true;
}

bool ch_stack_pop(ch_stack* stack, ch_stack_entry* popped) {
    if (stack->current == stack->start) {
        return false;
    }

    popped->primitive = *(stack->current - 1);
    size_t size = PRIMITIVE_SIZE(popped->primitive);
    stack->current -= size + FOOTER_SIZE;

    memcpy(&popped->data_start, stack->current, size);

    return true;
}

ch_stack ch_stack_create() {
    // TODO make stack size configurable upon startup
    size_t size = 1000;
    uint8_t* start = malloc(size);

    return (ch_stack) {
        .start=start,
        .end=start + size,
        .current=start,
    };
}