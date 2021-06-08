#include "object.h"
#include "hash.h"
#include "chapman.h"
#include <stdlib.h>
#include <string.h>

ch_function* ch_loadfunction(ch_dataptr function_ptr, ch_argcount argcount) {
  ch_function* function = malloc(sizeof(ch_function));
  function->object.type = TYPE_FUNCTION;
  function->argcount = argcount;
  function->ptr = function_ptr;

  return function;
}

ch_string* ch_copystring(const char* value, size_t size) {
  char* copy_value = malloc(size + 1);
  memcpy(copy_value, value, size);
  copy_value[size] = '\0';

  return ch_loadstring(copy_value, size);
}

ch_string* ch_loadstring(const char* value, size_t size) {
  ch_string* string = (ch_string*) malloc(sizeof(ch_string));
  string->object.type = TYPE_OBJECT;
  string->size = size;
  string->hash = ch_hash_string(value, size);
  string->value = value;

  return string;
}

ch_string* ch_loadistring(ch_context* context, char* value, size_t size) {
  ch_string* interned_string = ch_table_find_string(&context->strings, value, size);

  if (interned_string != NULL) {
    return interned_string;
  }

  ch_string* string = ch_loadstring(value, size);
  ch_table_set(&context->strings, string, MAKE_NULL());

  return string;
}
