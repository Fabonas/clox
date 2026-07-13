/**
 * @file memory.h
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

/**
 * Grow a capacity using the classic "start at 8, then double" rule.
 *
 * Starting at 8 avoids a wasteful chain of tiny reallocations for small
 * arrays, while doubling gives amortized O(1) appends.
 *
 * @param `capacity`  Current capacity (element count), may be 0.
 * @return          New, larger capacity.
 */
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

/**
 * Resize an existing allocation from `oldCount` elements to `newCount`
 * elements of the given type.
 *
 * Returns the new pointer (which may differ).
 *
 * @param `type`      Element type.
 * @param `pointer`   Existing buffer (may be `NULL`).
 * @param `oldCount`  Current element count.
 * @param `newCount`  Desired element count.
 * @return          Pointer to the resized block.
 */
#define GROW_ARRAY(type, pointer, oldCount, newCount)     \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
                      sizeof(type) * (newCount))

/**
 * Free an allocation of `oldCount` elements of the given type.
 *
 * Implemented as a `reallocate()`-to-zero, which calls `free()` under the
 * hood.
 *
 * @param `type`      Element type.
 * @param `pointer`   Buffer to free (may become `NULL`).
 * @param `oldCount`  Element count being released.
 */
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * The single allocation entry point.
 *
 * One function handles allocate, grow, shrink, and free:
 *
 * ```
 * pointer  oldSize  newSize  behaviour
 * -------  -------  -------  -------------------------------------
 * NULL     0        n        allocate a new n-byte block
 * ptr      o        n        grow/shrink the block to n bytes
 * ptr      o        0        free the block, return NULL
 * ```
 *
 * On allocation failure the process is terminated. This is deliberately
 * coarse-grained: clox does not attempt graceful recovery from OOM.
 *
 * @param `pointer`  Existing block (`NULL` for a fresh allocation).
 * @param `oldSize`  Previous size in bytes (reserved for future GC accounting;
 *                 currently unused).
 * @param `newSize`  Desired size in bytes; 0 means free.
 * @return         Pointer to the (re)allocated block, or `NULL` when freed.
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif
