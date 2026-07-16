/**
 * @file compiler.h
 * Introduction
 * ------------
 * Public interface to the compiler. The rest of the interpreter does not need
 * to know about parsers, precedence tables, or parse functions; it only needs
 * the single `compile()` entry point that turns source text into bytecode.
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

#include "object.h"
#include "vm.h"

bool compile(const char* source, Chunk* chunk);

#endif