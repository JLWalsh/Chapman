#pragma once
#include "compiler.h"
#include "token.h"

// TODO remove this file entirely

// Token-parsing related errors
void ch_tk_error(const char *message, const ch_token_state *state);