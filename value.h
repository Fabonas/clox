/**
 * @file value.h
 * Introduction
 * ------------
 * Runtime value representation. A `Value` is a tagged union that can currently
 * hold booleans, nil, numbers, and heap objects. The macros in this header are
 * the only supported way to construct, test, and unwrap values.
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

typedef struct Obj       Obj;
typedef struct ObjString ObjString;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool   boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})

typedef struct {
    int    capacity;
    int    count;
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);

void initValueArray(ValueArray* array);

void writeValueArray(ValueArray* array, Value value);

void freeValueArray(ValueArray* array);

void printValue(Value value);

#endif