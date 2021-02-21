#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <vm/chapman.h>
#include "compiler.h"

const char program[] = "12.5"; 

int main(void) {
    ch_program compiled_program = ch_compile(program, sizeof(program));



    return 0;
}