#include "hash.h"

const uint32_t fnv1a_prime = 0x01000193;
const uint32_t fnv1a_seed = 0x811C9DC5;

uint32_t ch_hash_string(const char *string, size_t n) {
  uint32_t hash = fnv1a_seed;

  while (n--) {
    hash = (*string ^ hash) * fnv1a_prime;
    string++;
  }

  return hash;
}