set(VM_SOURCE_FILES 
    chapman.c
    stack.c
    hash.c
    disassembler.c
    table.c
    bytecode.c
    object.c
)

# List of headers to be exported alongside the library
set(EXPORTED_HEADERS
    chapman.h
)

add_library(vm-static STATIC ${VM_SOURCE_FILES})
add_library(vm-shared SHARED ${VM_SOURCE_FILES})

target_include_directories(vm-static PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(vm-shared PUBLIC ${CMAKE_SOURCE_DIR}/src)