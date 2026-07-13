/**
 * @file value.h
 *
 * Lox's runtime value representation.
 *
 * Lox is currently dynamically typed with a single numeric type: double
 * precision floating point. `Value` is therefore just a `double` typedef.
 * This is the simplest possible representation, used in the early chapters
 * of *Crafting Interpreters*. Later chapters replace it with a tagged union
 * to support bools, nil, objects, and so on; the `Value`/`ValueArray` API
 * here is designed so that swap can be done without touching call sites.
 *
 * `ValueArray` is the growable array used by chunks to store their constant
 * pool and by the VM for various dynamic sequences.
 */

#ifndef clox_value_h
#define clox_value_h

#include "common.h"

/**
 * A Lox value. Today this is a plain IEEE-754 `double`; when richer types
 * are added this typedef will become a tagged union.
 */
typedef double Value;

/**
 * A growable array of `Value`s.
 *
 * `capacity` is the allocated slot count, `count` is the number currently
 * in use, and `values` points at the heap buffer.
 */
typedef struct {
    int    capacity;
    int    count;
    Value* values;
} ValueArray;

/**
 * Initialize an array to the empty, unallocated state.
 *
 * @param `array`  Array to initialize. The first write allocates the initial
 *               capacity (see `GROW_CAPACITY`).
 */
void initValueArray(ValueArray* array);

/**
 * Append a value, growing the backing buffer (doubling capacity) when full.
 *
 * @param `array`  Array to mutate.
 * @param `value`  Value to append.
 */
void writeValueArray(ValueArray* array, Value value);

/**
 * Release the buffer and reset to the empty state so the array can be
 * reused or go out of scope safely.
 *
 * @param `array`  Array to free.
 */
void freeValueArray(ValueArray* array);

/**
 * Print a value using `%g`, which trims trailing zeros and avoids
 * scientific notation for "small" magnitudes — giving Lox numbers a natural
 * look.
 *
 * @param `value`  Value to print.
 */
void printValue(Value value);

#endif
