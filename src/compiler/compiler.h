#pragma once
#include "token.h"
#include "emit.h"
#include <vm/chapman.h>
#include <stdint.h>

#define MAX_FUNCTION_BLOBS 256

typedef struct {
    ch_lexeme name;
    uint8_t scope_id;
} ch_local;

typedef struct ch_scope {
    ch_local locals[UINT8_MAX];

    uint8_t locals_size;
} ch_scope;

/*
    A blobscope is used to scope nested function's bytecode so that
    its bytecode ends up in a single section.

    For example, the bytecode of this function:

    #a() {
        work1();
        #b() {
            work2();
        }
        b();
        work3();
    }

    Will end up being laid out like this:
    #a:
      work1();
      b();
      work3();
    #b:
      work2();


    When parsing b, the compiler will create a new blob specifically for it,
    so that all the bytecode being emitted in b ends up in a single blob.
    Whenever the parser leaves a function, it will merge its own bytecode blob
    with its children's blobs, to create a singular big blob.
    So when the parser leaves b, it will "check in" b's blob into a's list of blobs.
    If b had children, it would still only "check in" a single blob into a, since
    it would've merged its own blobs.
*/

typedef struct {
    // Blob 0 is reserved for the current function
    ch_blob bytecode_blobs[MAX_FUNCTION_BLOBS];
    uint16_t num_blobs;

    struct ch_blobscope* parent;
} ch_blobscope;

typedef struct {
    ch_token_state token_state;
    ch_token previous;
    ch_token current;

    ch_scope scope;

    bool is_panic;
    bool has_errors;

    ch_blobscope* blobscope;
    ch_blob data_blob;
} ch_compilation;

bool ch_compile(const uint8_t* program, size_t program_size, ch_program* output);