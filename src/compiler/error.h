#pragma once
#include "token.h"

// Token-parsing related errors
void ch_tk_error(const char* message, const ch_token_state* state);