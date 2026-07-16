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
 * `ObjString`.
 */

#include "object.h"

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "memory.h"
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

static ObjString* allocateString(char* chars, int len) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->len       = len;
    string->chars     = chars;

    return string;
}

ObjString* takeString(char* chars, int len) {
    return allocateString(chars, len);
}

ObjString* copyString(const char* chars, int len) {
    char* heapChars = ALLOCATE(char, len + 1);
    memcpy(heapChars, chars, len);
    heapChars[len] = '\0';

    return allocateString(heapChars, len);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}
