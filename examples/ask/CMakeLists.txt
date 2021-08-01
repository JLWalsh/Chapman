add_executable(ask src/main.c)

target_link_libraries(ask PRIVATE compiler)

# Copy ask.ch into the build directory
add_custom_command(TARGET ask PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/src/ask.ch $<TARGET_FILE_DIR:ask>/ask.ch)