#pragma once
#include "defs.h"
#include <stdbool.h>

#define OBJECT_TYPE(object) ((object)->type)

#define AS_STRING(object) ((ch_string*)object)
#define IS_STRING(object) (OBJECT_TYPE(object) == TYPE_STRING)

#define AS_FUNCTION(object) ((ch_function*)object)
#define IS_FUNCTION(object) (OBJECT_TYPE(object) == TYPE_FUNCTION)
#define MAKE_FUNCTION(function_ptr, argcount)                                  \
  ((ch_function) {.ptr = function_ptr, .argcount = argcount, .object = (ch_object) {.type = TYPE_FUNCTION}})

#define AS_NATIVE(object) (object.native_function)
#define IS_NATIVE(object) (object.type == TYPE_NATIVE)
#define MAKE_NATIVE(native_function)                                  \
  ((ch_native) {.function = native_function, .object = (ch_object) {.type = TYPE_NATIVE}})

typedef enum {
  TYPE_FUNCTION,
  TYPE_NATIVE,
  TYPE_OBJECT,
} ch_object_type;

typedef struct ch_context ch_context;
typedef void (*ch_native_function)(ch_context* context);

typedef struct {
  ch_object_type type;
} ch_object;

typedef struct {
  ch_object object;
  const char *value;
  uint32_t size;
  uint32_t hash;
} ch_string;

typedef struct {
  ch_object object;
  ch_dataptr ptr;
  ch_argcount argcount;
} ch_function;

typedef struct {
  ch_object object;
  ch_native_function function;
} ch_native;

ch_string* ch_loadstring(ch_context* context, const char* value, size_t size);

// Assumes that the string does not contain a null byte at end
ch_string *ch_string_load_raw(uint8_t *string_ptr, uint32_t size);

void ch_string_free(ch_string *string);