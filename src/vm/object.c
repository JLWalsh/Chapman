#include "object.h"
#include "chapman.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ch_string *new_string(const char *value, size_t size) {
  ch_string *string = (ch_string *)malloc(sizeof(ch_string));
  ch_initstring(string, value, size);

  return string;
}

// Used when we are allowed to free() the string, in the event that it is already interned
// This is only used by functions in this file that allocate strings (ex. substring, concat)
static ch_string *register_allocated_string(ch_context* vm, char* value, size_t size) {
  ch_string *interned_string = ch_table_find_string(&vm->strings, value, size);
  if (interned_string != NULL) {
    free(value);
    return interned_string;
  }

  ch_string *string = new_string(value, size);
  ch_table_set(&vm->strings, string, MAKE_NULL());

  return string;
}

ch_function *ch_loadfunction(ch_dataptr function_ptr, ch_argcount argcount) {
  ch_function *function = malloc(sizeof(ch_function));
  function->object.type = TYPE_FUNCTION;
  function->argcount = argcount;
  function->ptr = function_ptr;

  return function;
}

ch_closure *ch_loadclosure(ch_function* function, uint8_t upvalue_count) {
  ch_upvalue** upvalues = malloc(sizeof(ch_upvalue*) * upvalue_count);
  for(uint8_t i = 0; i < upvalue_count; i++) {
    upvalues[i] = NULL;
  }

  ch_closure *closure = malloc(sizeof(ch_closure));
  closure->function = function;
  closure->object.type = TYPE_CLOSURE;
  closure->upvalues = upvalues;
  closure->upvalue_count = upvalue_count;

  return closure;
}

ch_upvalue *ch_loadupvalue(ch_primitive* value) {
  ch_upvalue *upvalue = malloc(sizeof(ch_upvalue));
  upvalue->object.type = TYPE_UPVALUE;
  upvalue->value = value;
  upvalue->next = NULL;
  upvalue->closed = MAKE_NULL();

  return upvalue;
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

  return register_allocated_string(vm, value, size);
}

ch_string *ch_substring(ch_context *vm, ch_string* target, size_t start, size_t end) {
  if (end <= start || start >= target->size || end > target->size) return NULL;
  size_t size = end - start;
  char *value = (char*) malloc(size + 1);
  memcpy(value, &target->value[start], size);
  value[size] = '\0';

  return register_allocated_string(vm, value, size);
}

bool ch_containsstring(ch_context *vm, ch_string* haystack, ch_string* needle) {
  if (needle->size > haystack->size) return false;

  return strstr(haystack->value, needle->value) != NULL;
}

void ch_initstring(ch_string *string, const char *value, size_t size) {
  string->value = value;
  string->size = size;
  string->hash = ch_hash_string(value, size);
  string->object.type = TYPE_STRING;
}

bool ch_object_isfalsy(ch_object* object) {
  switch(object->type) {
    case TYPE_NATIVE:
    case TYPE_FUNCTION:
      return false;
    case TYPE_STRING:
      return AS_STRING(object)->size == 0;
    default:
      return false;
  }

}

void ch_object_print(ch_object* object) {
  switch(object->type) {
    case TYPE_CLOSURE: {
      printf("CLOSURE FUNCTION");
      break;
    }
    case TYPE_UPVALUE: {
      printf("UPVALUE");
      break;
    }
    case TYPE_FUNCTION: {
      printf("FUNCTION");
      break;
    }
    case TYPE_NATIVE: {
      printf("NATIVE");
      break;
    }
    case TYPE_STRING: {
      printf("STRING %s", AS_STRING(object)->value);
      break;
    }
  }

  printf("\n");
}