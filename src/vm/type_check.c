#include "type_check.h"

bool ch_checkargcount(ch_context* vm, ch_argcount expected, ch_argcount actual) {
	bool is_same = expected == actual;

	if (!is_same) {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Expected %d arguments but got %d argument(s) instead", expected, actual);
	}

	return is_same;
}

ch_string* ch_checkstring(ch_context* vm, ch_primitive value) {
	if (!IS_OBJECT(value)) {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Expected string type, but got type %d instead", value.type);
		return NULL;
	}

	ch_object* object_value = AS_OBJECT(value);
	if (!IS_STRING(object_value)) {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Expected string type, but got object type %d instead", object_value->type);
		return NULL;
	}

	return AS_STRING(object_value);
}