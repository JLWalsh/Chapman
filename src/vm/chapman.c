#include "chapman.h"
#include "vm.h"
#include "natives.h"
#include "type_check.h"
#include <string.h>

ch_context ch_newvm(ch_program program) { 
  ch_context context = ch_vm_newcontext(program); 

  ch_addnative(&context, ch_native_string_size, "size");
  ch_addnative(&context, ch_native_string_substring, "substring");
  ch_addnative(&context, ch_native_string_contains, "contains");

  return context;
}

void ch_freevm(ch_context *context) { ch_vm_free(context); }

ch_primitive ch_runfunction(ch_context *context, const char *function_name) {
  ch_string *name =
      ch_loadstring(context, function_name, strlen(function_name), COPY_STRING);

  ch_primitive result = ch_vm_call(context, name);

  return result;
}

bool ch_popnumber(ch_context* vm, double* popped) {
  ch_primitive value = ch_pop(vm);
  return ch_checknumber(vm, value, popped);
}

bool ch_popstring(ch_context* vm, ch_string** popped) {
  ch_primitive value = ch_pop(vm);
  return ch_checkstring(vm, value, popped);
}

ch_primitive ch_pop(ch_context *vm) {
  ch_primitive popped = MAKE_NULL();
  ch_stack_pop(&vm->stack, &popped);

  return popped;
}

void ch_push(ch_context *vm, ch_primitive primitive) {
  ch_stack_push(&vm->stack, primitive);
}