/**
 * @file memory.c
 *
 * Implementation of the single allocation entry point.
 *
 * All dynamic memory in clox flows through `reallocate()`. Centralizing
 * allocation here means a future garbage collector or allocation profiler
 * can be wired in by changing this one function.
 */

#include "memory.h"

#include <stdlib.h>

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    // oldSize is reserved for future GC accounting; currently unused.
    (void)oldSize;

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if (result == NULL) {
        // No recovery path: abort so callers never see a NULL.
        exit(1);
        return NULL;
    }

    return result;
}
