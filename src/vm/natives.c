#include "natives.h"
#include "type_check.h"

void ch_native_string_size(ch_context* vm, ch_argcount argcount) {
	if (!ch_checkargcount(vm, 1, argcount)) return;

	ch_primitive primitive = ch_pop(vm);
	ch_string* value = ch_checkstring(vm, primitive);
	if(value == NULL) return;

	ch_push(vm, MAKE_NUMBER(value->size));
}