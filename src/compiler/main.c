#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <vm/chapman.h>
#include "compiler.h"

const char program[] = "12.5 + (2 * 3) / 5"; 

int main(void) {
    ch_program compiled_program = ch_compile(program, sizeof(program));

    ch_run(compiled_program);


    return 0;
}