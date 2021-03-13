#include "chapman.h"
#include "ops.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void halt(ch_context* context, ch_exit reason) {
    context->exit = reason;
}

bool read_number(ch_context* context, double* value) {
    if (context->pcurrent + sizeof(double) >= context->pend) {
        return false;
    }

    memcpy(value, context->pcurrent, sizeof(double));
    context->pcurrent += sizeof(double);

    return true;
}

ch_context create_context(ch_program program) {
    return (ch_context) {
        .pstart=program.start, 
        .pend=program.start + program.size, 
        .pcurrent=program.start,
        .stack=ch_stack_create(),
        .exit=EXIT_OK,
    };
}

#define MAKE_NUMBER(value) ((ch_stack_entry) {.number_value=value,.primitive=NUMBER})

#define STACK_PUSH(stack, entry) \
    if(!ch_stack_push(stack, entry)) { \
        halt(&context, EXIT_STACK_SIZE_EXCEEDED); \
        running = false; \
    } \

#define STACK_POP(stack, value) \
    if(!ch_stack_pop(stack, value)) { \
        halt(&context, EXIT_STACK_EMPTY); \
        running = false; \
    } \

#define READ_NUMBER(context, value) \
    if(!read_number(context, value)) { \
        halt(context, EXIT_PROGRAM_OUT_OF_INSTRUCTIONS); \
        running = false; \
    } \

void ch_run(ch_program program) {
    ch_context context = create_context(program);

    bool running = true;
    while(running) {
        uint8_t current = *(context.pcurrent);
        context.pcurrent++;
        
        switch(current) {
            case OP_NUMBER: {
                double value;
                READ_NUMBER(&context, &value);
                STACK_PUSH(&context.stack, MAKE_NUMBER(value));
                break;
            }
            case OP_ADD: {
                ch_stack_entry left;
                ch_stack_entry right;
                STACK_POP(&context.stack, &left);
                STACK_POP(&context.stack, &right);

                double result = left.number_value + right.number_value;
                //STACK_PUSH(&context.stack, MAKE_NUMBER(result));
                printf("Just added %f + %f together!\n", left.number_value, right.number_value);
                break;
            }
            case OP_SUB: {
                ch_stack_entry left;
                ch_stack_entry right;
                STACK_POP(&context.stack, &left);
                STACK_POP(&context.stack, &right);

                double result = left.number_value - right.number_value;
                STACK_PUSH(&context.stack, MAKE_NUMBER(result));
                break;
            }
            case OP_MUL: {
                ch_stack_entry left;
                ch_stack_entry right;
                STACK_POP(&context.stack, &left);
                STACK_POP(&context.stack, &right);

                double result = left.number_value * right.number_value;
                STACK_PUSH(&context.stack, MAKE_NUMBER(result));
                break;
            }
            case OP_DIV: {
                ch_stack_entry left;
                ch_stack_entry right;
                STACK_POP(&context.stack, &left);
                STACK_POP(&context.stack, &right);

                double result = left.number_value / right.number_value;
                STACK_PUSH(&context.stack, MAKE_NUMBER(result));
                break;
            }
            case OP_HALT: {
                running = false;
                break;
            }
            case OP_POP: {
                ch_stack_entry entry;
                STACK_POP(&context.stack, &entry);
                break;
            }
            case OP_LOAD_LOCAL: {
                double value;
                READ_NUMBER(&context, &value);
                uint8_t offset = (uint8_t) value;
                ch_stack_copy(&context.stack, offset);
                break;
            }
            default: {
                halt(&context, EXIT_UNKNOWN_INSTRUCTION);
                break;
            }
        }
    }

    if (context.exit != EXIT_OK) {
        printf("Runtime error: %d.\n", context.exit);
    } else {
        printf("Program has halted.\n");
    }
}