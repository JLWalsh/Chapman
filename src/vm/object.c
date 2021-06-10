#include "object.h"
#include "chapman.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>

static ch_string *new_string(const char *value, size_t size) {
  ch_string *string = (ch_string *)malloc(sizeof(ch_string));
  ch_initstring(string, value, size);

  return string;
}

ch_function *ch_loadfunction(ch_dataptr function_ptr, ch_argcount argcount) {
  ch_function *function = malloc(sizeof(ch_function));
  function->object.type = TYPE_FUNCTION;
  function->argcount = argcount;
  function->ptr = function_ptr;

  return function;
}

ch_native *ch_loadnative(ch_native_function function) {
  ch_native *native = malloc(sizeof(ch_native));
  native->function = function;
  native->object.type = TYPE_NATIVE;

  return native;
}

ch_string *ch_loadstring(ch_context *vm, const char *value, size_t size,
                         bool copy_string) {
  ch_string *interned_string = ch_table_find_string(&vm->strings, value, size);

  if (interned_string != NULL) {
    return interned_string;
  }

  const char *final_value = value;
  if (copy_string) {
    char *copied_string = malloc(size + 1);
    memcpy(copied_string, value, size);
    copied_string[size] = '\0';

    final_value = copied_string;
  }

  ch_string *string = new_string(value, size);
  ch_table_set(&vm->strings, string, MAKE_NULL());

  return string;
}

ch_string *ch_concatstring(ch_context *vm, ch_string* left, ch_string* right) {
  size_t size = left->size + right->size;
  char *value = (char*) malloc(size + 1);
  memcpy(value, left->value, left->size);
  memcpy(value + left->size, right->value, right->size);
  value[size] = '\0';

  ch_string *interned_string = ch_table_find_string(&vm->strings, value, size);
  if (interned_string != NULL) {
    free(value);
    return interned_string;
  }

  ch_string *string = new_string(value, size);
  ch_table_set(&vm->strings, string, MAKE_NULL());

  return string;
}

void ch_initstring(ch_string *string, const char *value, size_t size) {
  string->value = value;
  string->size = size;
  string->hash = ch_hash_string(value, size);
  string->object.type = TYPE_STRING;
}