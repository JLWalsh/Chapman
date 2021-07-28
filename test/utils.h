#pragma once
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <compiler.h>
#include <vm/chapman.h>

// Functions here are declared in the header, compilation times shouldn't be that bad since we're dealing with test files
// We don't have to worry about deallocating what we allocated here, since all tests are short lived

bool compile(char* program, ch_program* compiled_program);

ch_primitive run(char* program) {
    ch_program compiled_program;
    if(!compile(program, &compiled_program)) {
        return MAKE_NULL();
    }

    ch_context vm = ch_newvm(compiled_program);
    return ch_runfunction(&vm, "main");
}

bool doescompile(char* program) {
    ch_program unused_program;
    return compile(program, &unused_program);
}

bool compile(char* program, ch_program* compiled_program) {
    char prefix[] = "#main() {";
    char suffix[] = "}";

    size_t prefix_size = sizeof(prefix) - 1;
    size_t suffix_size = sizeof(suffix) - 1;
    size_t size = strlen(program);

    char* result = (char*) malloc(size + prefix_size + suffix_size + 1);

    memcpy(result, prefix, prefix_size);
    memcpy(result + prefix_size, program, size);
    memcpy(result + prefix_size + size, suffix, suffix_size);

    if (!ch_compile((uint8_t*)result, strlen(result), compiled_program)) {
        printf("Failed to compile program\n");
        return false;
    }

    return true;
}