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

int main(void) {
  char *program = load_program();

  ch_program compiled_program;
  if (!ch_compile(program, strlen(program), &compiled_program)) {
    printf("Failed to compile program\n");
    return 0;
  }

  ch_run(compiled_program);
  //ch_disassemble(&compiled_program);

  return 0;
}