#include "hash.h"

/*
    For clearer definitions of m(odulo) and p(ower), see: https://cp-algorithms.com/string/string-hashing.html
*/
const uint32_t p = 67;
const uint32_t m = 1e9 + 9;

uint32_t ch_hashstring(char* string, size_t size) {
    uint32_t hash = 0;
    for(size_t i = 0; i < size; i++) {
        hash += (string[i] * (p ^ i)) % m;
    }

    return hash;
}