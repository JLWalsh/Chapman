enable_testing()

cmake_minimum_required(VERSION 3.12)
project(Chapman)

add_subdirectory(src/vm)
add_subdirectory(src/compiler)
add_subdirectory(test)
add_subdirectory(lib)