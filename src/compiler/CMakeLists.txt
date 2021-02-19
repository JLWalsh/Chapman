SET(COMPILER_SOURCES main.c)

add_executable(compiler ${COMPILER_SOURCES})

target_link_libraries(compiler PRIVATE vm)