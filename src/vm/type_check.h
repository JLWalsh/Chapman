#pragma once
#include "chapman.h"

bool ch_checkargcount(ch_context* vm, ch_argcount expected, ch_argcount actual);

bool ch_checkstring(ch_context* vm, ch_primitive value, ch_string** actual);

bool ch_checkfunction(ch_context* vm, ch_primitive value, ch_function** actual);

bool ch_checknumber(ch_context* vm, ch_primitive value, double* actual);