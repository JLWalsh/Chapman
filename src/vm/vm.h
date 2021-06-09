#pragma once
#include "chapman.h"

ch_context ch_vm_newcontext(ch_program program);

void ch_vm_free(ch_context *context);

ch_primitive ch_vm_call(ch_context *context, ch_string *function_name);
