#pragma once
#include "defs.h"

#define AS_NUMBER(object) (object.number_value)
#define IS_NUMBER(object) (object.type == TYPE_NUMBER)
#define MAKE_NUMBER(value) ((ch_object){.number_value=value,.type=TYPE_NUMBER})

#define AS_FUNCTION(object) (object.number_value)
#define IS_FUNCTION(object) (object.type == TYPE_FUNCTION)
#define MAKE_FUNCTION(function_ptr, argcount) ((ch_object){ \
    .function=(ch_function) {.ptr=function_ptr,.argcount=argcount}, \
    .type=TYPE_FUNCTION} \
)\

typedef struct {
    ch_dataptr ptr;
    ch_argcount argcount;
} ch_function;

typedef enum {
    TYPE_NUMBER,
    TYPE_FUNCTION,
} ch_object_type;

typedef struct {
    union {
        double number_value;
        ch_function function;
    };
    ch_object_type type;
} ch_object;