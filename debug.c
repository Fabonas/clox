/**
 * @file debug.c
 * Introduction
 * ------------
 * Human-readable bytecode rendering. The disassembler walks a `Chunk` and
 * prints each instruction with its offset, source line, opcode name, and any
 * constant-pool operand. It is used both interactively and by the VM's
 * execution tracer.
 *
 * Chunk disassembler implementation.
 *
 * Produces output like:
 *
 * ```
 * == chunk name ==
 * 0000 1234 OP_CONSTANT      0 '1.2'
 * 0002    | OP_RETURN
 * ```
 *
 * The line column shows the source line for the first instruction on a line
 * and `|` for subsequent instructions sharing that line. Instructions with
 * a constant-pool operand display the constant value; local-variable
 * instructions display the stack slot.
 */

#include "debug.h"

#include <stdint.h>
#include <stdio.h>

#include "chunk.h"
#include "value.h"

void disassembleChunk(Chunk *chunk, const char *name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int simpleInstruction(const char *name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int byteInstruction(const char *name, Chunk *chunk, int offset) {
    u8 slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

static int jumpInstruction(const char *name, int sign, Chunk *chunk,
                           int offset) {
    u16 jump = (u16)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];

    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);

    return offset + 3;
}

static int constantInstruction(const char *name, Chunk *chunk, int offset) {
    u8 constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

int disassembleInstruction(Chunk *chunk, int offset) {
    printf("%04d ", offset);

    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    u8 instruction = chunk->code[offset];

    switch (instruction) {
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);

        case OP_PRINT:
            return simpleInstruction("OP_PRINT", offset);

        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);

        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);

        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);

        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);

        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);

        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);

        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);

        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);

        case OP_POP:
            return simpleInstruction("OP_POP", offset);

        case OP_SET_LOCAL:
            return byteInstruction("OP_SET_LOCAL", chunk, offset);

        case OP_GET_LOCAL:
            return byteInstruction("OP_GET_LOCAL", chunk, offset);

        case OP_GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL", chunk, offset);

        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);

        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);

        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);

        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);

        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);

        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);

        case OP_SET_GLOBAL:
            return constantInstruction("OP_SET_GLOBAL", chunk, offset);

        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);

        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);

        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
