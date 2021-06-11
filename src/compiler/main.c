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

void print(ch_context *vm, ch_argcount argcount) {
  ch_primitive popped = ch_pop(vm);
  printf("Got number: %f", popped.number_value);
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
    case PRIMITIVE_OBJECT: {
      printf("OBJECT: ");
      ch_object* obj = AS_OBJECT(return_value);
      switch(obj->type) {
        case TYPE_FUNCTION: {
          printf("FUNCTION");
          break;
        }
        case TYPE_NATIVE: {
          printf("NATIVE");
          break;
        }
        case TYPE_STRING: {
          printf("STRING %s", AS_STRING(obj)->value);
          break;
        }
      }
      break;
    }
  }

}

int main(void) {
  char *program = load_program();

  ch_program compiled_program;
  if (!ch_compile(program, strlen(program), &compiled_program)) {
    printf("Failed to compile program\n");
    return 0;
  }

  ch_context vm = ch_newvm(compiled_program);
  ch_addnative(&vm, print, "print");
  ch_primitive return_value = ch_runfunction(&vm, "main");
  ch_freevm(&vm);
  printvalue(return_value);

  // ch_disassemble(&compiled_program);

  return 0;
}