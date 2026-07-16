/**
 * @file value.c
 * Introduction
 * ------------
 * Implementation of the growable `ValueArray` and value formatting/equality.
 * These helpers sit at the boundary between the VM's runtime values and the
 * C standard library.
 *
 * Growable array of Lox values and value printing.
 *
 * `ValueArray` is a thin wrapper around the `memory.h` growth macros. The
 * chunk module uses it to store the constant pool: every `OP_CONSTANT`
 * instruction indexes into a `ValueArray` by a single-byte index.
 */

#include "value.h"

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"

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

void printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;

        case VAL_NIL:
            printf("nil");
            break;

        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;

        case VAL_OBJ:
            printObject(value);
            break;
    }
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);

        case VAL_NIL:
            return true;

        case VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);

        case VAL_OBJ: {
            ObjString* aString = AS_STRING(a);
            ObjString* bString = AS_STRING(b);

            return aString->len == bString->len &&
                   memcmp(aString->chars, bString->chars, aString->len) == 0;
        }

        default:
            return false;
    }
}