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

#define AS_NATIVE(object) ((ch_native*)object)
#define IS_NATIVE(object) (OBJECT_TYPE(object) == TYPE_NATIVE)
#define MAKE_NATIVE(native_function)                                  \
  ((ch_native) {.function = (ch_native_function) (native_function), .object = (ch_object) {.type = TYPE_NATIVE}})

#define COPY_STRING true
#define NOCOPY_STRING false

typedef enum {
  TYPE_FUNCTION,
  TYPE_NATIVE,
  TYPE_OBJECT,
  TYPE_STRING,
} ch_object_type;

typedef struct ch_context ch_context;
typedef void (*ch_native_function)(ch_context* context, ch_argcount argcount);

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

ch_function* ch_loadfunction(ch_dataptr function_ptr, ch_argcount argcount);

ch_native* ch_loadnative(ch_native_function function);

ch_string* ch_loadstring(ch_context* vm, const char* value, size_t size, bool copy_string);

void ch_initstring(ch_string* string, const char* value, size_t size);