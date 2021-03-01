#include "chapman.h"
#include "ops.h"
#include "fail.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void fail(ch_context* context, ch_fail reason) {
    printf("Failed: %d", reason);
}

bool read_number(ch_context* context, double* value);

ch_context create_context(ch_program program) {
    return (ch_context) {
        .pstart=program.start, 
        .pend=program.start + program.size, 
        .pcurrent=program.start,
        .stack=ch_stack_create()
    };
}

#define STACK_PUSH(stack, value, primitive) \
    if(!ch_stack_push((stack), (value), (primitive))) { \
        fail(&context, FAIL_STACK_SIZE_EXCEEDED); \
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
                if(!read_number(&context, &value)) {
                    printf("Error reading number\n");
                }
                STACK_PUSH(&context.stack, &value, NUMBER);
                printf("Numbah! %f \n", value);
                break;
            }
            case OP_ADD: {
                printf("Add!\n");
                break;
            }
            case OP_SUB: {
                printf("Sub!\n");
                break;
            }
            case OP_MUL: {
                printf("Mul!\n");
                break;
            }
            case OP_DIV: {
                printf("Div!\n");
                break;
            }
            case OP_HALT: {
                running = false;
                break;
            }
            case OP_POP: {
                ch_stack_entry entry;
                if(!ch_stack_pop(&context.stack, &entry)) {
                    printf("Failed to POPPOPOPOP");
                } else {
                    printf("We popped %d", entry.primitive);
                }
                break;
            }
            default: {
                printf("IDk");
            }
        }
    }

    printf("VM cleanup \n");
}

bool read_number(ch_context* context, double* value) {
    if (context->pcurrent + sizeof(double) >= context->pend) {
        return false;
    }

    memcpy(value, context->pcurrent, sizeof(double));
    context->pcurrent += sizeof(double);

    return true;
}