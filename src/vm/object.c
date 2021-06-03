#include "object.h"
#include "hash.h"
#include "chapman.h"
#include <stdlib.h>
#include <string.h>

ch_string* ch_loadstring(ch_context* context, const char* value, size_t size) {
  ch_string* string = (ch_string*) malloc(sizeof(ch_string));
  string->size = size;
  string->hash = ch_hash_string(value, size);
  string->value = value;

  ch_table_set(&context->strings, string, MAKE_NULL());

  return string;
}

void ch_string_free(ch_string *string) {
  free(string->value);
  free(string);
}