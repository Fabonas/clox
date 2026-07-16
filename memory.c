/**
 * @file memory.c
 * Introduction
 * ------------
 * Centralized dynamic memory management. Every growable array in clox routes
 * its allocations through `reallocate()`, making this the natural place to
 * add accounting, logging, or a future garbage collector.
 *
 * Implementation of the single allocation entry point.
 *
 * All dynamic memory in clox flows through `reallocate()`. Centralizing
 * allocation here means a future garbage collector or allocation profiler
 * can be wired in by changing this one function.
 */

#include "memory.h"

#include <stdlib.h>

#include "object.h"
#include "vm.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    (void)oldSize;

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if (result == NULL) {
        exit(1);
        return NULL;
    }

    return result;
}

static void freeObject(Obj* obj) {
    switch (obj->type) {
        case OBJ_STRING: {
            ObjString* string = (ObjString*)obj;
            FREE_ARRAY(char, string->chars, string->len + 1);
            FREE(ObjString, obj);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.objects;

    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}
