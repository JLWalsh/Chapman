macro(ch_addtest name)
    add_executable(${name} ${name}.c ${ARGN})
    add_test(${name} ${name})
    
    message(STATUS "Creating test target ${name}.")
    target_link_libraries(${name} compiler Unity)
endmacro()

ch_addtest(tests_math)