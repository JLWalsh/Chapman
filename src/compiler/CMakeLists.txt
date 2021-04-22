SET(COMPILER_SOURCES 
    main.c 
    token.c
    compiler.c
    error.c
    emit.c
)

add_executable(compiler ${COMPILER_SOURCES})

target_link_libraries(compiler PRIVATE vm)

add_custom_command(TARGET compiler PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/tests $<TARGET_FILE_DIR:compiler>/tests)