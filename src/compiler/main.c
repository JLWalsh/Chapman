#include "compiler.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vm/chapman.h>
#include <vm/disassembler.h>

char *load_program() {
  FILE *file = fopen("tests/test.ch", "rb");
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (size == -1) {
    return NULL;
  }

  char *program = malloc(size + 1);
  fread(program, 1, size, file);
  fclose(file);

  program[size] = 0;

  return program;
}

void printvalue(ch_primitive return_value) {
  switch(return_value.type) {
    case PRIMITIVE_BOOLEAN: {
      printf("BOOLEAN: %d", return_value.boolean_value);
      break;
    }
    case PRIMITIVE_NUMBER: {
      printf("NUMBER: %f", return_value.number_value);
      break;
    }
    case PRIMITIVE_NULL: {
      printf("NULL");
      break;
    }
    case PRIMITIVE_CHAR: {
      printf("CHAR: %c", return_value.char_value);
      break;
    }
    case PRIMITIVE_OBJECT: {
      ch_object_print(AS_OBJECT(return_value));
      break;
    }
  }

  printf("\n");
}

void print(ch_context *vm, ch_argcount argcount) {
  for(ch_argcount i = 0; i < argcount; i++) {
    ch_primitive popped = ch_pop(vm);
    printvalue(popped);
  }
}

int main(void) {
  char *program = load_program();

  ch_program compiled_program;
  if (!ch_compile((uint8_t*)program, strlen(program), &compiled_program)) {
    printf("Failed to compile program\n");
    return 0;
  }

  // ch_context vm = ch_newvm(compiled_program);
  // ch_addnative(&vm, print, "print");
  // ch_primitive return_value = ch_runfunction(&vm, "main");
  // ch_freevm(&vm);
  // printvalue(return_value);

  ch_disassemble(&compiled_program);

  return 0;
}