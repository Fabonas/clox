/**
 * @file common.h
 *
 * Shared definitions for the entire clox codebase.
 *
 * Every module includes this header. It pulls in the small set of standard
 * headers used across the project and defines fixed-width integer aliases so
 * the rest of the code can use short, explicit type names (`u8`, `i32`, …).
 * It also hosts compile-time configuration macros that toggle interpreter
 * features, such as execution tracing.
 *
 * Historical note: an earlier revision split these definitions into a
 * separate `types.h`. That file was folded into `common.h`, which now serves
 * as the single shared header for the project.
 */

#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * When defined, the VM prints the disassembly of every instruction it
 * executes, along with a snapshot of the value stack. Useful while debugging
 * the interpreter loop.
 */
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

/**
 * Fixed-width integer aliases. Using these keeps the bytecode and value
 * representations explicit about their sizes (opcodes are `u8`, constant
 * indices are `u8`) and avoids ambiguity with raw `int`.
 */
typedef int8_t  i8;
typedef uint8_t u8;

typedef int16_t  i16;
typedef uint16_t u16;

typedef int32_t  i32;
typedef uint32_t u32;

typedef int64_t  i64;
typedef uint64_t u64;

/**
 * Pointer-sized integers, useful for representing object sizes and pointer
 * arithmetic without losing precision on 64-bit hosts.
 */
typedef intptr_t  iptr;
typedef uintptr_t uptr;

typedef ptrdiff_t isize;
typedef size_t    usize;

#endif
