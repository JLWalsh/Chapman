#include "stack.h"
#include <string.h>
#include <stdlib.h>

bool stack_push(ch_stack* stack, void* value, ch_primitive primitive) {
    size_t size = PRIMITIVE_SIZE(primitive);
    if (stack->current + size >= stack->end) {
        return false;
    }

    memcpy(stack->current, value, size);
    stack->current += size;

    return true;
}

ch_stack ch_stack_create() {
    uint8_t* start = malloc(9);

    return (ch_stack) {
        .start=start,
        .end=start + 9,
        .current=start,
    };
}