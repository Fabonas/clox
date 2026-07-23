/**
 * @file chunk.h
 * Introduction
 * ------------
 * This header defines the `Chunk` type, the unit of compiled code in clox. A
 * chunk bundles raw bytecode, a constant pool, and per-byte line information
 * so that the VM can execute a program and the debugger can map instructions
 * back to source locations.
 *
 * Bytecode chunks: the VM's program representation.
 *
 * A `Chunk` is the compiled output the VM executes. It holds three parallel
 * arrays of equal length:
 *
 *   * `code`      — the raw bytecode, a sequence of opcodes (and, for some,
 *                    trailing operand bytes that index the constant pool or
 *                    name a local stack slot).
 *   * `lines`     — the source line number for each byte of code, used only
 *                    by the disassembler to attribute instructions to source
 *                    lines. Keeping it separate from `code` keeps the hot
 *                    interpreter loop cache-friendly.
 *   * `constants` — the constant pool (a `ValueArray`). `OP_CONSTANT` and
 *                    global-variable opcodes refer to entries here by index.
 *
 * `count`/`capacity` use the standard growable-array pattern from
 * `memory.h`.
 */

#ifndef clox_chunk_h

#define clox_chunk_h

#include <stdint.h>

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_LOCAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,
    OP_NEGATE,
    OP_PRINT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
} OpCode;

typedef struct {
    int        count;
    int        capacity;
    u8        *code;
    ValueArray constants;
    int       *lines;
} Chunk;

void initChunk(Chunk *chunk);

void freeChunk(Chunk *chunk);

void writeChunk(Chunk *chunk, u8 byte, int line);

int addConstant(Chunk *chunk, Value value);

#endif
