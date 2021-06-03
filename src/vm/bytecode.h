#pragma once
#include "chapman.h"
#include <stdint.h>

#define READ_U32(bytes_ptr)                                                    \
  ((ch_dataptr)((bytes_ptr)[0] | (bytes_ptr)[1] << 8 | (bytes_ptr)[2] << 16 |  \
                (bytes_ptr)[3] << 24))

ch_string *ch_bytecode_load_string(const ch_context *context, ch_dataptr location);