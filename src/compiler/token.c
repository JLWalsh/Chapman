#include "token.h"
#include "error.h"
#include "limits.h"
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define IS_AT_END(state_ptr) ((intptr_t) (state_ptr)->current >= (intptr_t) (state_ptr)->program + (intptr_t) (state_ptr)->size)

typedef struct {
  char *name;
  ch_token_kind kind;
} ch_keyword;

const ch_keyword keywords[] = {{"val", TK_VAL},
                               {"var", TK_VAR},
                               {"return", TK_RETURN},
                               {"native", TK_NATIVE}};

// Unspecified tokens are set to false by default
// (Since uninitialized values are always set to 0)
const bool tokens_with_lexemes[NUM_TOKENS] = {[TK_ID] = true, [TK_NUM] = true};

const uint8_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);

char peek(ch_token_state* state);

ch_token_kind parse_token_kind(const char *token_start, size_t token_size);

ch_token get_token(const char *start, ch_token_state *state,
                      ch_token_kind kind);
ch_token get_token_end(const char* start, const char* end, ch_token_kind kind, ch_token_state* state);

ch_token_state init_token(const uint8_t *program, size_t size) {
  return (ch_token_state){
      .program = program, .size = size, .current = (char *)program, .line = 1};
}

bool ch_token_next(ch_token_state *state, ch_token *next) {
  char *start = state->current;

  while (!IS_AT_END(state)) {
    char current = *state->current++;

    switch (current) {
    case '=':
      *next = get_token(start, state, TK_EQ);
      return true;
    case '+':
      *next = get_token(start, state, TK_PLUS);
      return true;
    case '-':
      *next = get_token(start, state, TK_MINUS);
      return true;
    case ';':
      *next = get_token(start, state, TK_SEMI);
      return true;
    case ',':
      *next = get_token(start, state, TK_COMMA);
      return true;
    case '(':
      *next = get_token(start, state, TK_POPEN);
      return true;
    case ')':
      *next = get_token(start, state, TK_PCLOSE);
      return true;
    case '{':
      *next = get_token(start, state, TK_COPEN);
      return true;
    case '}':
      *next = get_token(start, state, TK_CCLOSE);
      return true;
    case '\0':
      *next = get_token(start, state, TK_EOF);
      return true;
    case '#':
      *next = get_token(start, state, TK_POUND);
      return true;
    case '\n':
      state->line++;
      start = state->current;
      continue;
    case '\r':
      start = state->current;
      continue;
    case '/':
      if (*state->current == '/') {
        while (*state->current != '\n') {
          state->current++;
          start++;
        }
        continue;
      }

      *next = get_token(start, state, TK_FSLASH);
      return true;
    case '*':
      *next = get_token(start, state, TK_STAR);
      return true;
    case '"': {
      while(!IS_AT_END(state) && peek(state) != '"') {
        if (*state->current == '\\' && peek(state) == '"') {
          state->current++;
        }

        state->current++;
      }
      
      if (IS_AT_END(state)) {
        ch_tk_error("Expected end of string", state);
        return false;
      }

      // Consume last string char, and "
      state->current += 2;

      get_token_end(start + 1, state->current - 1, TK_STRING, state);

      return true;
    }
    default:
      if (isspace(current)) {
        start = state->current;
        continue;
      }

      // Identifiers
      if (isalpha(current)) {
        while (isalnum(*state->current)) {
          state->current++;
        }

        ptrdiff_t lexeme_size = state->current - start;
        if (lexeme_size >= MAX_ID_LENGTH) {
          ch_tk_error("Identifier size exceeds limit", state);
          return false;
        }

        ch_token_kind token_kind = parse_token_kind(start, lexeme_size);
        *next = get_token(start, state, token_kind);

        return true;
      }

      // Numbers
      if (isdigit(current)) {
        while (isdigit(*state->current)) {
          state->current++;
        }

        if (*state->current == '.') {
          *state->current++;
          char *digitStart = state->current;

          while (isdigit(*state->current)) {
            state->current++;
          }

          // No digits followed the .
          if (digitStart == state->current) {
            ch_tk_error("Unterminated number", state);
            return false;
          }
        }

        *next = get_token(start, state, TK_NUM);
        return true;
      }

      ch_tk_error("Unrecognized character", state);
      return false;
    }
  }

  *next = get_token(start, state, TK_EOF);
  return true;
}

char peek(ch_token_state* state) {
  if (IS_AT_END(state)) {
    return 0;
  }

  return state->current[1];
}

ch_token_kind parse_token_kind(const char *token_start, size_t token_size) {
  for (uint8_t i = 0; i < num_keywords; i++) {
    if (token_size != strlen(keywords[i].name)) {
      continue;
    }

    if (strncmp(token_start, keywords[i].name, token_size) == 0) {
      return keywords[i].kind;
    }
  }

  return TK_ID;
}

ch_token get_token(const char *start, ch_token_state *state,
                      ch_token_kind kind) {
  return get_token_end(start, state->current, kind, state);
}

ch_token get_token_end(const char* start, const char* end, ch_token_kind kind, ch_token_state* state) {
  ch_token token = {.kind = kind, .line = state->line};

  if (tokens_with_lexemes[kind]) {
    token.lexeme = (ch_lexeme){.start = start, .size = end - start};
  }

  return token;
}