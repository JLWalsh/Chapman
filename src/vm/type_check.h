#pragma once
#include "chapman.h"

bool ch_checkargcount(ch_context* vm, ch_argcount expected, ch_argcount actual);

ch_string* ch_checkstring(ch_context* vm, ch_primitive value);

bool ch_checkboolean(ch_context* vm, ch_primitive, bool* out_value);