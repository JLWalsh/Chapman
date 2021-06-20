#include "natives.h"
#include "type_check.h"

void ch_native_string_size(ch_context* vm, ch_argcount argcount) {
	if (!ch_checkargcount(vm, 1, argcount)) return;

	ch_string* string;
	if (!ch_popstring(vm, &string)) return;

	ch_push(vm, MAKE_NUMBER(string->size));
}

void ch_native_string_substring(ch_context* vm, ch_argcount argcount) {
	if (argcount != 2 && argcount != 3) {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Incorrect number of arguments passed to substring.");
		return;
	}

	bool has_end = argcount == 3;
	double end;
	if (has_end && !ch_popnumber(vm, &end)) return;

	double start;
	if (!ch_popnumber(vm, &start)) return;

	ch_string* string;
	if (!ch_popstring(vm, &string)) return;

	ch_string* result = ch_substring(vm, string, (size_t) start, has_end ? end : string->size);
	if (result != NULL) {
		ch_push(vm, MAKE_OBJECT(result));
	} else {
		ch_runtime_error(vm, EXIT_USER_ERROR, "Substring start or end index is out of range.");
	}
}

void ch_native_string_contains(ch_context* vm, ch_argcount argcount) {
	if (!ch_checkargcount(vm, 2, argcount)) return;

	ch_string* needle;
	if(!ch_popstring(vm, &needle)) return;

	ch_string* haystack;
	if(!ch_popstring(vm, &haystack)) return;

	bool contains = ch_containsstring(vm, haystack, needle);
	ch_push(vm, MAKE_BOOLEAN(contains));
}