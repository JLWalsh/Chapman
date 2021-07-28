macro(ch_addtest name)
    add_executable(${name} ${name}.c ${ARGN})
    add_test(${name} ${name})
    
    message(STATUS "Creating test target ${name}.")
    target_link_libraries(${name} vm-static compiler Unity)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endmacro()

ch_addtest(tests_math)
ch_addtest(tests_parse)