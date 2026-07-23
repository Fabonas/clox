/**
 * @file table.h
 * Introduction
 * ------------
 * Public interface to clox's hash table. The VM uses two tables: one for
 * global variables (`vm.globals`) and one for interned strings
 * (`vm.strings`). Both rely on `ObjString` keys with precomputed hashes.
 *
 * Hash table API.
 *
 * The table is intentionally specialized to `ObjString` keys and `Value`
 * payloads. Callers initialize and free tables, get and set entries, delete
 * keys, and copy entries between tables. String interning uses
 * `tableFindString()` to locate an existing `ObjString` matching a character
 * buffer and hash.
 */

#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct {
    ObjString *key;
    Value      value;
} Entry;

typedef struct {
    int    count;
    int    capacity;
    Entry *entries;
} Table;

void       initTable(Table *table);
void       freeTable(Table *table);
bool       tableGet(Table *table, ObjString *key, Value *value);
bool       tableOverwrite(Table *table, ObjString *key, Value value);
bool       tableContains(Table *table, ObjString *key);
Entry     *tableGetEntry(Table *table, ObjString *key);
bool       tableSet(Table *table, ObjString *key, Value value);
bool       tableDelete(Table *table, ObjString *key);
void       tableAddAll(Table *from, Table *to);
ObjString *tableFindString(Table *table, const char *chars, int len, u32 hash);

#endif
