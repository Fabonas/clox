/**
 * @file compiler.h
 *
 * Front-end entry point: source code to bytecode.
 *
 * The compiler turns a source string into bytecode in a chunk. Today this
 * is a placeholder that only tokenizes the source and prints the token
 * stream for inspection; it does not yet emit instructions. The VM's
 * `interpret()` calls `compile()` and currently assumes success.
 */

#ifndef clox_compiler_h

#define clox_compiler_h

#include "vm.h"

/**
 * Compile (currently: tokenize and print) Lox source.
 *
 * At present this initializes the scanner, reads tokens until EOF, and
 * prints them one per line for inspection — it does not emit bytecode. The
 * next development phase turns this into a real single-pass compiler that
 * emits instructions into a `Chunk`.
 *
 * @param `source`  NUL-terminated source text to compile.
 */
bool compile(const char* source, Chunk* chunk);

#endif
