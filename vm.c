/**
 * @file vm.c
 *
 * The bytecode interpreter loop.
 *
 * Execution model: `run()` is a fetch-decode-execute loop. Each iteration
 * reads one opcode byte from the chunk (via `READ_BYTE`) and dispatches on
 * it. Most opcodes operate on the value stack: `OP_CONSTANT` pushes, the
 * arithmetic opcodes pop two and push one, `OP_NEGATE` pops and pushes, and
 * `OP_RETURN` pops and prints before halting.
 *
 * When `DEBUG_TRACE_EXECUTION` is defined, every iteration first dumps the
 * current stack contents and the disassembly of the instruction about to
 * run — extremely useful for tracing execution.
 *
 * NOTE: `interpret()` currently only calls the (stub) compiler and returns
 * `OK`. The `run()` loop is written but not yet wired in, because the
 * compiler does not yet produce a chunk. Once compilation emits code,
 * `interpret()` will install the chunk and call `run()`.
 */

#include "vm.h"

#include <stdio.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"

/**
 * The single global VM instance. A global keeps the code simple and matches
 * the structure of *Crafting Interpreters*; a later phase may pass it
 * around.
 */
VM vm;

/**
 * Reset the stack to empty.
 *
 * `stackTop` points at the next free slot, so an empty stack has
 * `stackTop == &stack[0]`.
 */
static void resetStack() { vm.stackTop = vm.stack; }

void initVM() { resetStack(); }

void freeVM() {}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

/**
 * The core interpreter loop.
 *
 * Uses local macros for fast instruction decoding; they are `#undef`'d at
 * the end to keep them scoped to this function.
 *
 * @return `INTERPRET_OK` when `OP_RETURN` is reached.
 */
static InterpretResult run() {
// Read and consume one byte of code (the next opcode or operand).
#define READ_BYTE() (*vm.ip++)
// Read a one-byte constant index and resolve it to the actual `Value`.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
// Pop `b`, pop `a`, push `(a op b)`. The do/while(0) wraps it as a single
// statement so it can be used safely inside a switch case.
#define BINARY_OP(op)     \
    do {                  \
        double b = pop(); \
        double a = pop(); \
        push(a op b);     \
    } while (false)

    for (;;) {
// When tracing is on, print the stack then disassemble the next instruction.
#ifdef DEBUG_TRACE_EXECUTION
        printf("     ");

        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
        u8 instruction;

        switch (instruction = READ_BYTE()) {
            case OP_ADD:
                BINARY_OP(+);
                break;

            case OP_SUBTRACT:
                BINARY_OP(-);
                break;

            case OP_MULTIPLY:
                BINARY_OP(*);
                break;

            case OP_DIVIDE:
                BINARY_OP(/);
                break;

            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NEGATE: {
                push(-pop());
                break;
            }

            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(const char* source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip    = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}
