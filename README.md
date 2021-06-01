# Chapman
A strictly typed, dynamic language inspired by Lua and Javascript.

## Project Overview
The project's source code is split into two sub-projects, being the compiler and the virtual machine. The compiler can be found at `src/compiler`, and the vm can be found at `src/vm`.

### Requirements
- CMake
- clang-format
- GNU Make
- (Linux) gcc
- (MacOS) gcc or clang
- (Windows) mingw32 or mingw64

## Makefile Commands
Make is solely used to store util commands in this project.

### Build
```bash
make quickbuild
```
Make sure that you have a working CMake installation in your environment.

### Format
```bash
make format
```

## Special Thanks
While Chapman deviates from [munificient's](https://github.com/munificent) clox implementation, it was still a very useful resource whenever I wasn't too sure about what I was doing. [Go check out Crafting Interpreters!](http://www.craftinginterpreters.com/)