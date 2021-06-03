// Taken from https://craftinginterpreters.com/hash-tables.html
#include "table.h"
#include <stdlib.h>
#include <string.h>

#define TABLE_MAX_LOAD 0.75
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)

static ch_table_entry *find_entry(ch_table_entry *entries, uint32_t capacity,
                                  ch_string *key) {
  uint32_t index = key->hash & (capacity - 1);
  ch_table_entry *tombstone = NULL;
  for (;;) {
    ch_table_entry *entry = &entries[index];
    if (entry->key == NULL) {
      if (IS_NULL(entry->value)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL) tombstone = entry;
      }
    } else if (entry->key == key) {
      return entry;
    }

    index = (index + 1) & (capacity - 1);
  }
}

static void adjust_capacity(ch_table *table, uint32_t capacity) {
  ch_table_entry *entries =
      (ch_table_entry *)malloc(capacity * sizeof(ch_table_entry));
  for (uint32_t i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = MAKE_NULL();
  }

  table->size = 0;
  for (uint32_t i = 0; i < table->capacity; i++) {
    ch_table_entry *entry = &table->entries[i];

    if (entry->key == NULL) continue;

    ch_table_entry *destination = find_entry(entries, capacity, entry->key);
    destination->key = entry->key;
    destination->value = entry->value;
    table->size++;
  }

  free(table->entries);

  table->entries = entries;
  table->capacity = capacity;
}

bool ch_table_set(ch_table *table, ch_string *key, ch_object value) {
  if (table->size + 1 > table->capacity * TABLE_MAX_LOAD) {
    uint32_t capacity = GROW_CAPACITY(table->capacity);
    adjust_capacity(table, capacity);
  }

  ch_table_entry *entry = find_entry(table->entries, table->capacity, key);
  bool is_new = entry->key == NULL;

  if (is_new && IS_NULL(entry->value)) table->size++;

  entry->key = key;
  entry->value = value;

  return is_new;
}

ch_object *ch_table_get(ch_table *table, ch_string *key) {
  if (table->size == 0) return NULL;

  ch_table_entry *entry = find_entry(table->entries, table->capacity, key);
  if (entry->key == NULL) return NULL;

  return &entry->value;
}

bool ch_table_delete(ch_table *table, ch_string *key) {
  if (table->size == 0) return false;

  ch_table_entry *entry = find_entry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  entry->key = NULL;
  entry->value = MAKE_BOOLEAN(true);

  return true;
}

ch_string* ch_table_find_string(ch_table* table, const char* value, size_t size, uint32_t hash) {
  if (table->size == 0) return NULL;

  uint32_t index = hash & (table->capacity - 1);
  for (;;) {
    ch_table_entry* entry = &table->entries[index];
    if (entry->key == NULL) {

      if (IS_NULL(entry->value)) return NULL;
    } else if (entry->key->size == size &&
        entry->key->hash == hash &&
        memcmp(entry->key->value, value, size) == 0) {
      return entry->key;
    }

    index = (index + 1) & (table->capacity - 1);
  }
}

void ch_table_create(ch_table *out_table) {
  out_table->capacity = 0;
  out_table->size = 0;
  out_table->entries = NULL;
}

void ch_table_free(ch_table *table) {
  free(table->entries);
  ch_table_create(table);
}
