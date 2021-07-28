SET(COMPILER_SOURCES 
    token.c
    compiler.c
    error.c
    emit.c
)

add_executable(runcompiler main.c ${COMPILER_SOURCES})
add_library(compiler SHARED ${COMPILER_SOURCES})

target_link_libraries(runcompiler PRIVATE vm-static)
target_link_libraries(compiler PUBLIC vm-static)
target_include_directories(compiler PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Copy any files in the `tests` directory. These files are Chapman programs that are used during development to test out the compiler & vm
add_custom_command(TARGET runcompiler PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/tests $<TARGET_FILE_DIR:compiler>/tests)