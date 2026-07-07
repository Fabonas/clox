#ifndef clox_chunk_h

#define clox_chunk_h

#include <stdint.h>

#include "common.h"
#include "run.h"
#include "value.h"

typedef enum { OP_CONSTANT, OP_RETURN } OpCode;
typedef struct {
    int count;
    int capacity;
    uint8_t *code;
    ValueArray constants;
    RunArray runs;
    Run *lastRun;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);

#endif
