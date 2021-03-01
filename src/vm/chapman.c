#include "chapman.h"
#include "ops.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

bool read_number(ch_context* context, double* value);

void ch_run(ch_program program) {
    ch_context context = {.pstart=program.start, .pend=program.start + program.size, .pcurrent=program.start};

    bool running = true;
    while(running) {
        uint8_t current = *(context.pcurrent);
        context.pcurrent++;
        
        switch(current) {
            case OP_NUMBER: {
                double number;
                if(!read_number(&context, &number)) {
                    printf("Error reading number");
                }
                printf("Numbah! %f \n", number);
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