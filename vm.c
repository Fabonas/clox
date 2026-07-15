/**
 * @file vm.c
 *
 * The bytecode interpreter loop.
 *
 * Execution model: `run()` is a fetch-decode-execute loop. Each iteration
 * reads one opcode byte from the chunk (via `READ_BYTE`) and dispatches on
 * it. Most opcodes operate on the value stack: `OP_CONSTANT` pushes, the
 * arithmetic/comparison opcodes pop two and push one, `OP_NEGATE` and
 * `OP_NOT` pop and push, and `OP_RETURN` pops and prints before halting.
 * Operand types are checked at runtime; a mismatch raises a runtime error
 * via `runtimeError()` and aborts interpretation.
 *
 * When `DEBUG_TRACE_EXECUTION` is defined, every iteration first dumps the
 * current stack contents and the disassembly of the instruction about to
 * run — extremely useful for tracing execution.
 */

#include "vm.h"

#include <stdarg.h>
#include <stdbool.h>
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

/**
 * Report a runtime error: print a `printf`-style message to `stderr`
 * followed by the offending source line, then reset the stack. The caller is
 * expected to `return INTERPRET_RUNTIME_ERROR;` immediately after.
 *
 * @param `format`  `printf`-style format string.
 * @param `...`     Format arguments.
 */
static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputs("\n", stderr);

    usize instruction = vm.ip - vm.chunk->code - 1;
    int   line        = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

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
 * Peek at a value on the stack without popping it.
 *
 * @param `dist`  Distance below the top (0 is the top, 1 the one below, …).
 * @return       The value `dist` slots below the top of the stack.
 */
static Value peek(int dist) { return vm.stackTop[-1 - dist]; }

/**
 * Lox truthiness. `nil` and `false` are falsey; every other value
 * (including `0`) is truthy. Used by `OP_NOT` (and later by `and`/`or`/`if`).
 *
 * @param `value`  Value to test.
 * @return       `true` if `value` is falsey.
 */
static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
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
// Pop `b`, pop `a`, push `valueType(a op b)`. Both operands must be numbers;
// otherwise raise a runtime error and abort. The do/while(0) wraps it as a
// single statement so it can be used safely inside a switch case. `valueType`
// selects how to box the result (`NUMBER_VAL` for arithmetic, `BOOL_VAL` for
// comparisons).
#define BINARY_OP(valueType, op)                          \
    do {                                                  \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers.");    \
            return INTERPRET_RUNTIME_ERROR;               \
        }                                                 \
        double b = AS_NUMBER(pop());                      \
        double a = AS_NUMBER(pop());                      \
        push(valueType(a op b));                          \
    } while (false)

    for (;;) {
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
                BINARY_OP(NUMBER_VAL, +);
                break;

            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;

            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;

            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;
            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;

            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_NIL:
                push(NIL_VAL);
                break;

            case OP_TRUE:
                push(BOOL_VAL(true));
                break;

            case OP_FALSE:
                push(BOOL_VAL(false));
                break;

            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_NEGATE: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(NUMBER_VAL(-AS_NUMBER(pop())));
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
