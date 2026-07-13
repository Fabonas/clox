/**
 * @file debug.h
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

/**
 * Disassemble an entire chunk.
 *
 * Prints a `== name ==` header, then every instruction in order. The loop
 * advances `offset` past operands by reassigning it to the return value of
 * `disassembleInstruction()`.
 *
 * @param `chunk`  Chunk to dump.
 * @param `name`   Label printed in the header line.
 */
void disassembleChunk(Chunk* chunk, const char* name);

/**
 * Disassemble the single instruction at `offset`.
 *
 * Prints the byte offset, source line (or `|` for continuation), opcode
 * name, and operand detail, then returns the offset of the next instruction
 * (offset + 1 for operand-less opcodes, +2 for those with a one-byte
 * operand).
 *
 * @param `chunk`   Chunk containing the instruction.
 * @param `offset`  Byte offset of the instruction to disassemble.
 * @return        Byte offset of the following instruction.
 */
int disassembleInstruction(Chunk* chunk, int offset);

#endif
