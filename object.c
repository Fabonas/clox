/**
 * @file object.c
 * Introduction
 * ------------
 * Heap object construction and object-specific behavior. Today this is limited
 * to strings: allocating the `ObjString` header, copying character data onto
 * the heap, and printing string values. Future object types will be added here.
 *
 * Heap object implementations.
 *
 * Object headers are allocated through `allocateObject()`, which sizes the
 * allocation for the concrete object type and tags it. Strings are built by
 * copying their payload into a fresh heap buffer and wrapping it in an
 * `ObjString`. Every created string is interned in `vm.strings` so that
 * identical string literals and concatenation results share a single
 * `ObjString` instance.
 */

#include "object.h"

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(usize size, ObjType type) {
    Obj* object  = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects   = object;

    return object;
}

static ObjString* allocateString(char* chars, int len, u32 hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->len       = len;
    string->chars     = chars;
    string->hash      = hash;
    tableSet(&vm.strings, string, NIL_VAL);

    return string;
}

static u32 hashString(const char* key, int len) {
    u32 hash = 2166136261u;

    for (int i = 0; i < len; i++) {
        hash ^= (u8)key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString* takeString(char* chars, int len) {
    u32        hash     = hashString(chars, len);
    ObjString* interned = tableFindString(&vm.strings, chars, len, hash);

    if (interned != NULL) {
        FREE_ARRAY(char, chars, len + 1);
        return interned;
    }

    return allocateString(chars, len, hash);
}

ObjString* copyString(const char* chars, int len) {
    char* heapChars = ALLOCATE(char, len + 1);
    memcpy(heapChars, chars, len);
    heapChars[len] = '\0';

    u32        hash     = hashString(chars, len);
    ObjString* interned = tableFindString(&vm.strings, chars, len, hash);

    if (interned != NULL) return interned;

    return allocateString(heapChars, len, hash);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}
