set(VM_SOURCE_FILES 
    chapman.c
    stack.c
    hash.c
    disassembler.c
    table.c
)

add_library(vm ${VM_SOURCE_FILES})

target_include_directories(vm PUBLIC ${CMAKE_SOURCE_DIR}/src)