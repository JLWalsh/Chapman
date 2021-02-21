#pragma once
#include <stdint.h>
#include "ops.h"

#define CH_DATAPTR_NULL 0
#define CH_DATAPTR_MAX UINT32_MAX
typedef uint32_t ch_dataptr;

void ch_run(uint8_t* program);