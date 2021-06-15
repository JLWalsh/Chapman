#pragma once
#include <stdint.h>

#define CH_DATAPTR_NULL 0
#define CH_DATAPTR_MAX UINT32_MAX

// By convention, ch_dataptr is serialized in little endian format
#define READ_U32(bytes_ptr)                                                    \
  ((ch_dataptr)((bytes_ptr)[0] | (bytes_ptr)[1] << 8 | (bytes_ptr)[2] << 16 |  \
                (bytes_ptr)[3] << 24))
typedef uint32_t ch_dataptr;

// To convert types without changing bits (which can occur using a regular cast)
#define U32_TO_JMPPTR(u32) ((union { ch_jmpptr i; uint32_t z; }) { .z = u32}).i
#define JMPPTR_TO_U32(jmpptr) ((union { ch_jmpptr i; uint32_t z; }) { .i = ptr}).z
#define READ_JMPPTR(bytes_ptr)                                                    \
  ((ch_jmpptr)((bytes_ptr)[0] | (bytes_ptr)[1] << 8 | (bytes_ptr)[2] << 16 |  \
                (bytes_ptr)[3] << 24))
typedef int32_t ch_jmpptr;

#define READ_ARGCOUNT(argcount_ptr) ((ch_argcount)(argcount_ptr)[0])
#define CH_ARGCOUNT_MAX 255
typedef uint8_t ch_argcount;
