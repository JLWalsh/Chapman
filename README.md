# Chapman
![Actions Status](https://github.com/JLwalsh/Chapman/actions/workflows/actions.yml/badge.svg)

A strictly typed, dynamic language inspired by Lua and Javascript.

## Quick Demo
Let's suppose that we wish to invoke the following program from a C or C++ codebase:
```chapman
#addNumber(x) {
    return x + 1337;
}

#main() {
    val number = 42;

    val shinyNewNumber = addNumber(number);

    // Print is a native function
    print(shinyNewNumber);
}
```

It can be done using Chapman's C API:
```c
#include <compiler.h>

// Here we define the native function (which is used in the program above)
void print(ch_context* vm, ch_argcount argcount) {
    ch_primitive number = ch_pop(vm);
    printf("Aaaand the number is: %f\n", number.number_value);
}

int main(void) {
    char* raw_program = /* load using filesystem, network, etc. */

    ch_program program;
    if(!ch_compile(raw_program, &program)) {
        printf("Oh noes! Looks like we've got compilation errors...\n");
        return -1;
    }

    ch_context vm = ch_newvm(program);

    // Here we bind the global "print" to the native function print 
    ch_addnative(&vm, print, "print");  

    // And finally, we invoke the program using the main function.
    ch_runfunction(&vm, "main");

    ch_freevm(&vm);

    return 0;
}
```

## Examples
Check out the [examples folder](/examples) for in-depth demos!

## Project Overview
The project's source code is split into two sub-projects, being the compiler and the virtual machine. The compiler can be found at `src/compiler`, and the vm can be found at `src/vm`.

### Requirements
- CMake
- clang-format
- (Linux) gcc
- (MacOS) gcc or clang
- (Windows) mingw32 or mingw64

### Running tests
In order to run our tests, we use Unity which is installed using a Git submodule. To install Unity, run the following command:
```
make setup
```

To run all tests, run the following command:
```
cd YOUR_BUILD_FOLDER/tests
ctest
```

## Special Thanks
While Chapman deviates from [munificient's](https://github.com/munificent) clox implementation, it was still a very useful resource whenever I wasn't too sure about what I was doing. [Go check out Crafting Interpreters!](http://www.craftinginterpreters.com/)
