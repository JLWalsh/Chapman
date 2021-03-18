#pragma once
#include "token.h"
#include "compiler.h"

// Token-parsing related errors
void ch_tk_error(const char* message, const ch_token_state* state);

// Parser-related errors
void ch_pr_error(const char* message, ch_compilation* state);