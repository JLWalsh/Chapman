#pragma once
#include "defs.h"
#include <stdbool.h>

#define AS_NUMBER(object) (object.number_value)
#define IS_NUMBER(object) (object.type == TYPE_NUMBER)
#define MAKE_NUMBER(value)                                                     \
  ((ch_object){.number_value = value, .type = TYPE_NUMBER})

#define AS_BOOLEAN(object) (object.boolean_value)
#define IS_BOOLEAN(object) (object.type == TYPE_BOOLEAN)
#define MAKE_BOOLEAN(value)                                                    \
  ((ch_object){.boolean_value = value, .type = TYPE_BOOLEAN})

#define IS_NULL(object) (object.type == TYPE_NULL)
#define MAKE_NULL() ((ch_object){.type = TYPE_NULL})

#define AS_STRING(object) (object.string_value)
#define IS_STRING(object) (object.type == TYPE_STRING)
#define MAKE_STRING(value)                                                     \
  ((ch_object){.string_value = value, .type = TYPE_STRING})

#define AS_FUNCTION(object) (object.function)
#define IS_FUNCTION(object) (object.type == TYPE_FUNCTION)
#define MAKE_FUNCTION(function_ptr, argcount)                                  \
  ((ch_object){.function =                                                     \
                   (ch_function){.ptr = function_ptr, .argcount = argcount},   \
               .type = TYPE_FUNCTION})

typedef struct {
  ch_dataptr ptr;
  ch_argcount argcount;
} ch_function;

typedef struct {
  char *value;
  uint32_t size;
  uint32_t hash;
} ch_string;

typedef enum {
  TYPE_NUMBER,
  TYPE_FUNCTION,
  TYPE_STRING,
  TYPE_BOOLEAN,
  TYPE_NULL,
} ch_object_type;

typedef struct {
  union {
    double number_value;
    ch_function function;
    ch_string string_value;
    bool boolean_value;
  };
  ch_object_type type;
} ch_object;

// Assumes that the string does not contain a null byte at end
ch_string *ch_string_load_raw(uint8_t *string_ptr, uint32_t size);

void ch_string_free(ch_string *string);