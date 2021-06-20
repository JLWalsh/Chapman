#pragma once
#include "chapman.h"

void ch_native_string_size(ch_context* vm, ch_argcount argcount);
/*
	substring(string, start)
	substring(string, start, end) End is exclusive
*/
void ch_native_string_substring(ch_context* vm, ch_argcount argcount);
/*
	contains(string, substring)
*/
void ch_native_string_contains(ch_context* vm, ch_argcount argcount);