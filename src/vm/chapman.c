#include "chapman.h"
#include "ops.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Reads a 4 byte pointer (offset from 0) in little endian layout
#define READ_PTR(context) ( \
    (context)->pcurrent += 4,\
    (ch_dataptr) ((context)->pcurrent[0] & (context)->pcurrent[1] << 8 & (context)->pcurrent[2] << 16 & (context)->pcurrent[3] << 24) \
    ) \

#define READ_ARGCOUNT(context) ( \
    (context)->pcurrent += 1, \
    (ch_argcount) ((context)->pcurrent[0]) \
    ) \

// TODO do memory bounds check?
#define LOAD_NUMBER(context, ptr) (*((double*) ((context)->pstart[ptr])))

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
        .pend=program.start + program.total_size, 
        .pcurrent=program.start + program.data_size, // Seek after the data section
        .stack=ch_stack_create(),
        .call_stack=(ch_call_stack) {
            .size=0,
        },
        .exit=EXIT_OK,
    };
}

#define STACK_PUSH(context_ptr, entry) \
    if(!ch_stack_push(&(context_ptr)->stack, entry)) { \
        halt(context_ptr, EXIT_STACK_SIZE_EXCEEDED); \
        break; \
    } \

#define STACK_POP(context_ptr, value) \
    if(!ch_stack_pop(&(context_ptr)->stack, value)) { \
        halt(context_ptr, EXIT_STACK_EMPTY); \
        break; \
    } \

#define READ_NUMBER(context_ptr, value) \
    if(!read_number(context_ptr, value)) { \
        halt(context_ptr, EXIT_PROGRAM_OUT_OF_INSTRUCTIONS); \
        break; \
    } \

void ch_run(ch_program program) {
    ch_context context = create_context(program);

    while(context.exit == RUNNING) {
        uint8_t current = *(context.pcurrent);
        context.pcurrent++;
        
        switch(current) {
            case OP_NUMBER: {
                double value = LOAD_NUMBER(&context, READ_PTR(&context));
                READ_NUMBER(&context, &value);
                STACK_PUSH(&context, MAKE_NUMBER(value));
                break;
            }
            case OP_ADD: {
                ch_object left;
                ch_object right;
                STACK_POP(&context, &left);
                STACK_POP(&context, &right);

                double result = left.number_value + right.number_value;
                STACK_PUSH(&context, MAKE_NUMBER(result));
                break;
            }
            case OP_SUB: {
                ch_object left;
                ch_object right;
                STACK_POP(&context, &left);
                STACK_POP(&context, &right);

                double result = left.number_value - right.number_value;
                STACK_PUSH(&context, MAKE_NUMBER(result));
                break;
            }
            case OP_MUL: {
                ch_object left;
                ch_object right;
                STACK_POP(&context, &left);
                STACK_POP(&context, &right);

                double result = left.number_value * right.number_value;
                STACK_PUSH(&context, MAKE_NUMBER(result));
                break;
            }
            case OP_DIV: {
                ch_object left;
                ch_object right;
                STACK_POP(&context, &left);
                STACK_POP(&context, &right);

                double result = left.number_value / right.number_value;
                STACK_PUSH(&context, MAKE_NUMBER(result));
                break;
            }
            case OP_HALT: {
                context.exit = EXIT_OK;
                break;
            }
            case OP_POP: {
                ch_object entry;
                STACK_POP(&context, &entry);
                break;
            }
            case OP_POPN: {
                double value;
                READ_NUMBER(&context, &value);
                uint8_t num_popped = (uint8_t) value;

                // TODO add runtime check?
                ch_stack_popn(&context.stack, num_popped);
                break;
            }
            case OP_LOAD_LOCAL: {
                double value;
                READ_NUMBER(&context, &value);
                uint8_t offset = (uint8_t) value;
                ch_stack_copy(&context.stack, offset);
                break;
            }
            case OP_FUNCTION: {
                ch_dataptr function_ptr = READ_PTR(&context);
                ch_argcount argcount = READ_ARGCOUNT(&context);
                STACK_PUSH(&context, MAKE_FUNCTION(function_ptr, argcount));
                break;
            }
            case OP_CALL: {
                ch_argcount argcount = READ_ARGCOUNT(&context);

                ch_object entry;
                STACK_POP(&context, &entry);
                if (!IS_NUMBER(entry)) {
                    halt(&context, EXIT_INCORRECT_TYPE);
                    return;
                }
                uint32_t offset = (uint32_t) AS_NUMBER(entry);
                uint8_t* address = context.pstart + offset;
                
                break;
            }
            case OP_RETURN_VALUE: {
                break;
            }
            case OP_RETURN_VOID: {
                break;
            }
            case OP_DEBUG: {
                ch_object entry;
                STACK_POP(&context, &entry);
                if (IS_NUMBER(entry)) {
                    printf("NUMBER: %f\n", AS_NUMBER(entry));
                }
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