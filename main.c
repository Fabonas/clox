#include "chunk.h"
#include "debug.h"

int main(int argc, char *argv[]) {
    Chunk chunk;
    initChunk(&chunk);

    // Test line encoding with multiple lines and runs.
    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 1);  // line 1: first instruction
    writeChunk(&chunk, constant, 1);     // line 1: still on same line

    constant = addConstant(&chunk, 3.4);
    writeChunk(&chunk, OP_CONSTANT, 2);  // line 2: new line
    writeChunk(&chunk, constant, 2);     // line 2: still on same line

    constant = addConstant(&chunk, 5.6);
    writeChunk(&chunk, OP_CONSTANT, 3);  // line 3
    writeChunk(&chunk, constant, 3);

    writeChunk(&chunk, OP_RETURN, 4);    // line 4

    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);
    return 0;
}
