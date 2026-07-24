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
 * The compiler turns a source string into bytecode in a chunk. It implements a
 * single-pass Pratt parser (chapter 17) that compiles expressions, statements,
 * variable declarations (chapters 21–22), and control-flow statements
 * (chapter 23). On success the chunk is ready for the VM; on failure it
 * returns `false` and the caller discards the chunk.
 */

#ifndef clox_compiler_h

#define clox_compiler_h

#include "object.h"
#include "vm.h"

bool compile(const char* source, Chunk* chunk);

#endif