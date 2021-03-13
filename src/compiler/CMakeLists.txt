SET(COMPILER_SOURCES 
    main.c 
    token.c
    compiler.c
    error.c
    emit.c
    hash.c
)

add_executable(compiler ${COMPILER_SOURCES})

target_link_libraries(compiler PRIVATE vm)