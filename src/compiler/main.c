#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <vm/chapman.h>
#include "compiler.h"

const char program[] = "{ val z = 30; { val x = 40; 10 + x; val y = 50; x + y; x+z; }; }"; 

int main(void) {
    ch_program compiled_program;
    if(!ch_compile(program, sizeof(program), &compiled_program)) {
        printf("Failed to compile program\n");
        return 0;
    }

    ch_run(compiled_program);

    return 0;
}