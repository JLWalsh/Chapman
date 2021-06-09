#include "chapman.h"
#include "vm.h"
#include <string.h>

ch_context ch_newvm(ch_program program) {
    return ch_vm_newcontext(program);
}

void ch_freevm(ch_context* context) {
    ch_vm_free(context);
}

ch_primitive ch_runfunction(ch_context* context, const char* function_name) {
    ch_string* name = ch_loadstring(context, function_name, strlen(function_name), COPY_STRING);

    ch_primitive result = ch_vm_call(context, name);

    return result;
}

ch_primitive ch_pop(ch_context* vm) {
    ch_primitive popped = MAKE_NULL();
    ch_stack_pop(&vm->stack, &popped);

    return popped;
}