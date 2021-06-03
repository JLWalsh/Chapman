#pragma once
#include "object.h"

#define AS_NUMBER(p) (p.number_value)
#define IS_NUMBER(p) (p.type == PRIMITIVE_NUMBER)
#define MAKE_NUMBER(value)                                                     \
  ((ch_primitive){.number_value = value, .type = PRIMITIVE_NUMBER})

#define AS_BOOLEAN(p) (p.boolean_value)
#define IS_BOOLEAN(p) (p.type == PRIMITIVE_BOOLEAN)
#define MAKE_BOOLEAN(value)                                                    \
  ((ch_primitive){.boolean_value = value, .type = PRIMITIVE_BOOLEAN})

#define IS_NULL(p) (p.type == PRIMITIVE_NULL)
#define MAKE_NULL() ((ch_primitive){.type = PRIMITIVE_NULL})

#define AS_OBJECT(p) (p.object_value)
#define IS_OBJECT(p) (p.type == PRIMITIVE_OBJECT)
#define MAKE_OBJECT(value) \
    ((ch_primitive){.object_value = value, .type = PRIMITIVE_OBJECT })

typedef enum {
    PRIMITIVE_NUMBER,
    PRIMITIVE_BOOLEAN,
    PRIMITIVE_NULL,
    PRIMITIE_OBJECT
} ch_primitive_type;

typedef struct {
    ch_primitive_type type;
    union {
        double number_value;
        bool boolean_value;
        ch_object* object_value;
    };
} ch_primitive;