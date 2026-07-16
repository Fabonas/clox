/**
 * @file chunk.c
 * Introduction
 * ------------
 * This module owns the `Chunk` value type: a growable bytecode buffer paired
 * with its constant pool and source-line metadata. Chunks are the compiler's
 * output and the VM's input, so every byte of code and every constant flows
 * through the functions here before execution begins.
 *
 * Bytecode chunk construction and constant pool management.
 *
 * These functions let the compiler emit code byte-by-byte while transparently
 * growing the code and lines buffers, and let it intern constants into the
 * pool, receiving back the single-byte index used by `OP_CONSTANT`.
 */

#include "chunk.h"

#include <stdint.h>
#include <stdlib.h>

#include "memory.h"
#include "value.h"

void initChunk(Chunk* chunk) {
    chunk->count    = 0;
    chunk->capacity = 0;
    chunk->code     = NULL;
    chunk->lines    = NULL;
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(u8, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, u8 byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(u8, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines =
            GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count]  = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}