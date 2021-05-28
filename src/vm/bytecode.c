#include "bytecode.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>

ch_string *ch_bytecode_load_string(const ch_program *program,
                                   ch_dataptr location) {
  uint32_t size = READ_U32(&program->start[location]);
  uint8_t *string_location = &program->start[location + sizeof(uint32_t)];

  return ch_string_load_raw(string_location, size);
}