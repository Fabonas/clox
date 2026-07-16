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
 *   * `code`      — the raw bytecode, a sequence of opcodes (and, for some
 *                    opcodes, a trailing operand byte that indexes the
 *                    constant pool).
 *   * `lines`     — the source line number for each byte of code, used only
 *                    by the disassembler to attribute instructions to source
 *                    lines. Keeping it separate from `code` keeps the hot
 *                    interpreter loop cache-friendly.
 *   * `constants` — the constant pool (a `ValueArray`). `OP_CONSTANT` refers
 *                    to entries here by index.
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
<<<<<<< HEAD
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
=======
    OP_CONSTANT, /**< push `constants[index]` (index in next byte) */
    OP_NIL,      /**< push `nil` */
    OP_TRUE,     /**< push `true` */
    OP_FALSE,    /**< push `false` */
    OP_EQUAL,    /**< pop `b`, pop `a`, push `a == b` */
    OP_GREATER,  /**< pop `b`, pop `a`, push `a > b` */
    OP_LESS,     /**< pop `b`, pop `a`, push `a < b` */
    OP_RETURN,   /**< pop and print the top of the stack, halt execution */
    OP_NEGATE,   /**< pop `a`, push `-a` */
    OP_ADD,      /**< pop `b`, pop `a`, push `a + b` */
    OP_SUBTRACT, /**< pop `b`, pop `a`, push `a - b` */
    OP_MULTIPLY, /**< pop `b`, pop `a`, push `a * b` */
    OP_DIVIDE,   /**< pop `b`, pop `a`, push `a / b` */
    OP_NOT,      /**< pop `a`, push `!a` (true if `a` is falsey) */
>>>>>>> 14ae8ba (Implemented Chapter 18: Types of Values)
} OpCode;

typedef struct {
    int        count;
    int        capacity;
    u8*        code;
    ValueArray constants;
    int*       lines;
} Chunk;

void initChunk(Chunk* chunk);

void freeChunk(Chunk* chunk);

void writeChunk(Chunk* chunk, u8 byte, int line);

int addConstant(Chunk* chunk, Value value);

#endif