/**
 * @file debug.h
 * Introduction
 * ------------
 * Public disassembler interface. Debugging a bytecode interpreter is much
 * easier when you can see the instruction stream; these two functions produce
 * the formatted views used by the rest of the tool chain.
 *
 * Bytecode disassembly helpers.
 *
 * The disassembler renders a `Chunk` as human-readable text: each
 * instruction on its own line, with its byte offset, source line, opcode
 * name, and (for `OP_CONSTANT`) the constant value. It is used both
 * interactively (for inspecting compiled output) and by the VM when
 * `DEBUG_TRACE_EXECUTION` is on.
 */

#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);

int disassembleInstruction(Chunk* chunk, int offset);

#endif