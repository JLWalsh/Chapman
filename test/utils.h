#pragma once
#include <memory.h>
#include <string.h>
#include <stdlib.h>

// Functions here are declared in the header, compilation times shouldn't be that bad since we're dealing with test files

char* inmain(char* program) {
    char program_prefix[] = "#main() {";
    char program_suffix[] = "}";
    char* result = (char*) malloc(strlen(program) + 1);
}