#include <compiler.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write(ch_context *vm, ch_argcount argcount);
void prompt(ch_context *vm, ch_argcount argcount);

char* load_file(char* path);
void write_file(const char* path, const char* content);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./ask out_file\n");
        return -1;
    }

    char* ask = load_file("ask.ch");
    if (ask == NULL) {
        printf("ask.ch file not found");
        return -1;
    }

    ch_program program;
    if(!ch_compile((uint8_t*) ask, strlen(ask), &program)) {
        printf("ask.ch had compilation errors");
        return -2;
    }

    ch_context vm = ch_newvm(program);

    ch_addnative(&vm, write, "write");
    ch_addnative(&vm, prompt, "prompt");

    ch_string* filename = ch_loadstring(&vm, argv[1], strlen(argv[1]), COPY_STRING);
    ch_push(&vm, MAKE_OBJECT(filename));

    ch_primitive return_value = ch_runfunction(&vm, "ask");
    ch_freevm(&vm);

    return 0;
}

/*
    Usage: write(path, value)
    Writes the file with the given value to the provided path.

    path: string
    value: string
*/
void write(ch_context *vm, ch_argcount argcount) {
    if (!ch_checkargcount(vm, 2, argcount)) return;

    ch_string* content;
    ch_primitive value = ch_pop(vm);
    if(!ch_checkstring(vm, value, &content)) return;

    ch_string* path;
    value = ch_pop(vm);
    if(!ch_checkstring(vm, value, &path)) return;

    write_file(path->value, content->value);
}

void prompt(ch_context *vm, ch_argcount argcount) {
   if (!ch_checkargcount(vm, 1, argcount)) return;

    ch_string* question;
    ch_primitive value = ch_pop(vm);
    if(!ch_checkstring(vm, value, &question)) return;

    printf("%s\n", question->value);
    char answer[100];
    fgets(answer, sizeof(answer) - 1, stdin);

    ch_string* loaded_answer = ch_loadstring(vm, answer, strlen(answer), COPY_STRING);
    ch_push(vm, MAKE_OBJECT(loaded_answer));
}

char* load_file(char* path) {
  FILE *file = fopen(path, "rb");
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (size == -1) {
    return NULL;
  }

  char *contents = malloc(size + 1);
  fread(contents, 1, size, file);
  fclose(file);

  contents[size] = 0;

  return contents;
}

void write_file(const char* path, const char* content) {
    FILE *file = fopen(path, "wb");
    fwrite(content, 1, strlen(content), file);
    fclose(file);
}