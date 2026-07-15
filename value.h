/**
 * @file value.h
 *
 * Lox's runtime value representation.
 *
 * `Value` is a tagged union: a `ValueType` tag plus a payload union holding
 * the C value for each type Lox currently supports — `bool`, `nil`, and
 * `number` (IEEE-754 `double`). This is the representation introduced in
 * chapter 18 of *Crafting Interpreters*; later chapters extend the union
 * with heap objects (strings, closures, …).
 *
 * A family of macros (`IS_*`/`AS_*`/`*_VAL`) wraps the common type-test,
 * unwrap, and construction operations so call sites stay readable and the
 * representation can evolve behind them.
 *
 * `ValueArray` is the growable array used by chunks to store their constant
 * pool and by the VM for various dynamic sequences.
 */

#ifndef clox_value_h
#define clox_value_h

#include <stdint.h>

#include "common.h"

/**
 * The runtime type tag carried by every `Value`. One variant per Lox type
 * supported so far.
 */
typedef enum {
    VAL_BOOL,   /**< `true` / `false` */
    VAL_NIL,    /**< the singleton `nil` value */
    VAL_NUMBER, /**< IEEE-754 `double` */
} ValueType;

/**
 * A Lox value: a type tag (`type`) plus a payload union (`as`) holding the
 * underlying C value. Only the union arm matching the tag is meaningful;
 * access the correct arm with the `AS_*` macros, never by reading `as`
 * directly.
 */
typedef struct {
    ValueType type; /**< discriminates which `as` arm is valid */
    union {
        bool   boolean; /**< valid when `type == VAL_BOOL` */
        double number;  /**< valid when `type == VAL_NUMBER` (also used as a
                         zeroed placeholder for `VAL_NIL`) */
    } as;
} Value;

/** Type-test macros: true when `value` carries the named type. */
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

/** Unwrap macros: extract the C payload. Only call the one matching the tag. */
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

/** Construction macros: build a `Value` of the named type from a C value. */
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

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
 * Structural value equality. Two values are equal only when their type tags
 * match and their payloads compare equal; values of different types are
 * always unequal (so `1 == true` is false). Used by the VM's `OP_EQUAL`.
 *
 * @param `a`  First value.
 * @param `b`  Second value.
 * @return    `true` if `a` and `b` are equal.
 */
bool valuesEqual(Value a, Value b);

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
 * Print a value's textual form: `true`/`false` for booleans, `nil` for nil,
 * and numbers via `%g` (which trims trailing zeros and avoids scientific
 * notation for "small" magnitudes).
 *
 * @param `value`  Value to print.
 */
void printValue(Value value);

#endif
