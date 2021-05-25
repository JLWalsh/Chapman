#pragma once
#include "token.h"
#include "compiler.h"

// TODO remove this file entirely

// Token-parsing related errors
void ch_tk_error(const char* message, const ch_token_state* state);