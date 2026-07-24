/**
 * @file vm.c
 * Introduction
 * ------------
 * The bytecode interpreter. It compiles source into a chunk, then runs a
 * fetch-decode-execute loop against a fixed-size value stack. Runtime errors
 * abort the current interpretation and report the offending source line.
 *
 * The bytecode interpreter loop.
 *
 * Execution model: `run()` is a fetch-decode-execute loop. Each iteration
 * reads one opcode byte from the chunk (via `READ_BYTE`) and dispatches on
 * it. Most opcodes operate on the value stack: `OP_CONSTANT` pushes, the
 * arithmetic/comparison opcodes pop two and push one, `OP_NEGATE` and
 * `OP_NOT` pop and push, and `OP_RETURN` halts. Variable opcodes read and
 * write locals by stack slot (`OP_GET_LOCAL`/`OP_SET_LOCAL`) or globals via
 * the `vm.globals` hash table (`OP_GET_GLOBAL`, `OP_DEFINE_GLOBAL`,
 * `OP_SET_GLOBAL`). `OP_PRINT` pops a value and prints it. The jump
 * instructions (`OP_JUMP`, `OP_JUMP_IF_FALSE`, and `OP_LOOP`) manipulate the
 * instruction pointer by a signed 16-bit offset to implement `if`/`else`,
 * `while`, `for`, and short-circuiting logical operators.
 *
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
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

VM vm;

static void resetStack() { vm.stackTop = vm.stack; }

static void runtimeError(const char *format, ...) {
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

void initVM() {
    resetStack();
    vm.objects = NULL;

    initTable(&vm.globals);
    initTable(&vm.strings);
}

void freeVM() {
    freeTable(&vm.strings);
    freeTable(&vm.globals);
    freeObjects();
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int dist) { return vm.stackTop[-1 - dist]; }

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    ObjString *b = AS_STRING(pop());
    ObjString *a = AS_STRING(pop());

    int   len   = a->len + b->len;
    char *chars = ALLOCATE(char, len + 1);

    memcpy(chars, a->chars, a->len);
    memcpy(chars + a->len, b->chars, b->len);

    chars[len] = '\0';

    ObjString *result = takeString(chars, len);
    push(OBJ_VAL(result));
}

#define READ_BYTE() (*vm.ip++)

#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define READ_SHORT() (vm.ip += 2, (u16)((vm.ip[-2] << 8) | vm.ip[-1]))

#define READ_STRING() AS_STRING(READ_CONSTANT())

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

static InterpretResult run() {
    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("     ");

        for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        u8 instruction;

        switch (instruction = READ_BYTE()) {
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

            case OP_ADD:
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());

                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operands must be two numbers or two string.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_DEFINE_GLOBAL: {
                ObjString *name = READ_STRING();
                Value      temp = NUMBER_VAL(1);

                if (tableGet(&vm.globals, name, &temp)) {
                    tableOverwrite(&vm.globals, name, peek(0));
                } else {
                    tableSet(&vm.globals, name, peek(0));
                }

                pop();
                break;
            }

            case OP_SET_LOCAL: {
                u8 slot        = READ_BYTE();
                vm.stack[slot] = peek(0);
                break;
            }

            case OP_SET_GLOBAL: {
                ObjString *name = READ_STRING();

                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

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

            case OP_POP:
                pop();
                break;

            case OP_GET_LOCAL: {
                u8 slot = READ_BYTE();
                push(vm.stack[slot]);
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString *name = READ_STRING();
                Value      value;

                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(value);
                break;
            }

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

            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }

            case OP_JUMP: {
                u16 offset = READ_SHORT();
                vm.ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                u16 offset = READ_SHORT();
                if (isFalsey(peek(0))) vm.ip += offset;
                break;
            }

            case OP_LOOP: {
                u16 offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }

            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }
}

#undef READ_STRING
#undef BINARY_OP
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT

InterpretResult interpret(const char *source) {
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
