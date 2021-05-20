#pragma once

#define AS_NUMBER(object) (object.number_value)
#define IS_NUMBER(object) (object.type == TYPE_NUMBER)
#define MAKE_NUMBER(value) ((ch_object){.number_value=value,.type=TYPE_NUMBER})

typedef enum {
    TYPE_NUMBER,
} ch_object_type;

typedef struct {
    union {
        double number_value;
    };
    ch_object_type type;
} ch_object;