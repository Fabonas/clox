/**
 * @file value.c
 *
 * Growable array of Lox values and value printing.
 *
 * `ValueArray` is a thin wrapper around the `memory.h` growth macros. The
 * chunk module uses it to store the constant pool: every `OP_CONSTANT`
 * instruction indexes into a `ValueArray` by a single-byte index.
 */

#include "value.h"

#include <stdio.h>

#include "memory.h"

void initValueArray(ValueArray* array) {
    array->values   = NULL;
    array->capacity = 0;
    array->count    = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values =
            GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value) { printf("%g", value); }
