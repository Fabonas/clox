/**
 * @file object.h
 * Introduction
 * ------------
 * Header for clox's heap-allocated objects. It defines the object base type,
 * the string specialization, and the macros used by `value.h` to test and
 * unwrap object payloads.
 *
 * Heap object type system.
 *
 * All heap objects share a leading `Obj` header that stores the object's
 * runtime type tag and a `next` pointer for the VM's object list. Concrete
 * object types such as `ObjString` embed this header as their first member,
 * allowing a `Value`'s `Obj*` payload to be cast to the appropriate subtype
 * once its type has been checked. Strings also store their length and a
 * precomputed hash so they can be used as hash-table keys.
 */

#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType     type;
    struct Obj* next;
};

struct ObjString {
    Obj   obj;
    int   len;
    char* chars;
    u32   hash;
};

ObjString* takeString(char* chars, int len);
ObjString* copyString(const char* chars, int len);

void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
