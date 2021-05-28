#include "object.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>

ch_string* ch_string_load_raw(uint8_t* string_ptr, uint32_t size) {
    ch_string* string = (ch_string*) malloc(sizeof(string));
    // +1 for null byte
    string->value = (char*) malloc(size + 1);
    string->size = size + 1;

    memcpy(string->value, string_ptr, size);
    string->value[size] = '\0';

    string->hash = ch_hash_string(string->value, string->size);

    return string;
}

void ch_string_free(ch_string* string) {
    free(string->value);
    free(string);
}