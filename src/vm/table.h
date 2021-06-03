#pragma once
#include "object.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  ch_string *key;
  ch_object value;
} ch_table_entry;

typedef struct {
  ch_table_entry *entries;
  uint32_t capacity;
  uint32_t size;
} ch_table;

void ch_table_create(ch_table *out_table);

void ch_table_free(ch_table *table);

bool ch_table_set(ch_table *table, ch_string *key, ch_object value);

ch_object *ch_table_get(ch_table *table, ch_string *key);

bool ch_table_delete(ch_table *table, ch_string *key);

ch_string* ch_table_find_string(ch_table* table, const char* value, size_t size, uint32_t hash);