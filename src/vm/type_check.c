#include "type_check.h"

bool ch_checkargcount(ch_context* vm, ch_argcount expected, ch_argcount actual) {
	bool is_same = expected == actual;

	if (!is_same) {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Expected %d arguments but got %d argument(s) instead", expected, actual);
	}

	return is_same;
}

bool ch_checkstring(ch_context* vm, ch_primitive value, ch_string** actual) {
	if (!IS_OBJECT(value)) {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Expected string type, but got type %d instead", value.type);
		return false;
	}

	ch_object* object_value = AS_OBJECT(value);
	if (!IS_STRING(object_value)) {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Expected string type, but got object type %d instead", object_value->type);
		return false;
	}

	*actual = AS_STRING(object_value);

	return true;
}

bool ch_checknumber(ch_context* vm, ch_primitive value, double* actual) {
	if (!IS_NUMBER(value)) {
		return false;
	}

	*actual = AS_NUMBER(value);
	return true;
}