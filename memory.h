/**
 * @file memory.h
 * Introduction
 * ------------
 * Allocation macros and the single reallocation primitive. The macros hide
 * the size arithmetic and the "start at 8, then double" growth policy so that
 * containers can grow their buffers without repeating boilerplate.
 *
 * Dynamic memory management primitives.
 *
 * clox grows many of its data structures (bytecode chunks, value arrays, the
 * call stack) by over-allocating capacity and doubling when full. This header
 * provides the three macros every growable container uses, all of which route
 * through a single `reallocate()` so the whole allocator can be instrumented
 * or replaced in one place.
 *
 * Design: clox does not handle allocation failure granularly. `reallocate()`
 * aborts the process on OOM, which keeps the rest of the code free of
 * per-call error handling. This is the approach taken by *Crafting
 * Interpreters* and is acceptable for a learning interpreter.
 */

#ifndef clox_memory_h
#define clox_memory_h

#include <stddef.h>

#include "common.h"
#include "object.h"

#define ALLOCATE(type, count) (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount)     \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
                      sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void  freeObjects();

#endif
