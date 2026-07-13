/**
 * @file chunk.h
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

/**
 * The set of opcodes understood by the VM. Each is a single byte. Some
 * opcodes (`OP_CONSTANT`) are followed by an operand byte in the code
 * stream.
 */
typedef enum {
    OP_CONSTANT, /**< push `constants[index]` (index in next byte) */
    OP_RETURN,   /**< pop and print the top of the stack, halt execution */
    OP_NEGATE,   /**< pop `a`, push `-a` */
    OP_ADD,      /**< pop `b`, pop `a`, push `a + b` */
    OP_SUBTRACT, /**< pop `b`, pop `a`, push `a - b` */
    OP_MULTIPLY, /**< pop `b`, pop `a`, push `a * b` */
    OP_DIVIDE,   /**< pop `b`, pop `a`, push `a / b` */
} OpCode;

/**
 * A compiled bytecode chunk plus its constant pool and line metadata.
 */
typedef struct {
    int        count;     /**< number of bytes currently in use */
    int        capacity;  /**< allocated byte capacity of code/lines */
    u8*        code;      /**< the bytecode */
    ValueArray constants; /**< the constant pool */
    int*       lines;     /**< source line per code byte */
} Chunk;

/**
 * Set a chunk to the empty, unallocated state.
 *
 * @param `chunk`  Chunk to initialize. The first `writeChunk` triggers the
 *               initial allocation.
 */
void initChunk(Chunk* chunk);

/**
 * Release everything a chunk owns, then re-init so it can be reused.
 *
 * @param `chunk`  Chunk to free.
 */
void freeChunk(Chunk* chunk);

/**
 * Append one byte of code (an opcode or operand) tagged with its source
 * line.
 *
 * Grows both `code` and `lines` together so they stay index-aligned.
 *
 * @param `chunk`  Chunk to mutate.
 * @param `byte`   Opcode or operand byte to append.
 * @param `line`   1-based source line this byte originates from.
 */
void writeChunk(Chunk* chunk, u8 byte, int line);

/**
 * Append `value` to the constant pool and return its index.
 *
 * The compiler then emits `OP_CONSTANT` followed by this index. Because
 * only a single operand byte is used, a single chunk can hold at most 256
 * distinct constants.
 *
 * @param `chunk`  Chunk whose constant pool to extend.
 * @param `value`  Value to intern.
 * @return       Index of the new constant in the pool.
 */
int addConstant(Chunk* chunk, Value value);

#endif
