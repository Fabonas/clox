/**
 * @file common.h
 * Introduction
 * ------------
 * This is the shared foundation of the entire interpreter. Every translation
 * unit includes it first, so it is kept small and dependency-free: only the
 * most widely used standard headers, fixed-width integer aliases, and global
 * debug switches live here.
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

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

#define UINT8_COUNT (UINT8_MAX + 1)

typedef int8_t  i8;
typedef uint8_t u8;

typedef int16_t  i16;
typedef uint16_t u16;

typedef int32_t  i32;
typedef uint32_t u32;

typedef int64_t  i64;
typedef uint64_t u64;

typedef intptr_t  iptr;
typedef uintptr_t uptr;

typedef ptrdiff_t isize;
typedef size_t    usize;

#endif
