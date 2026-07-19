/**
 * @file vm.h
 * Introduction
 * ------------
 * Public interface to the virtual machine. Outside code initializes the VM,
 * pushes values if needed, and calls `interpret()` to compile and run a
 * program.
 *
 * The stack-based bytecode virtual machine.
 *
 * The VM owns a value stack (a fixed-size array used as a stack via a
 * `stackTop` pointer) and, while executing, references the current `Chunk`
 * and an instruction pointer (`ip`) into that chunk's code. Execution is a
 * simple fetch-decode-execute loop implemented in `vm.c`'s `run()`.
 *
 * The public API is tiny: init/free the VM, interpret a source string, and
 * push/pop values onto the stack.
 */

#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    u8*    ip;
    Value  stack[STACK_MAX];
    Value* stackTop;
    Table  strings;
    Obj*   objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

void initVM();

void freeVM();

/**
 * Compile then execute `source`.
 *
 * Compiles the source into a `Chunk`, installs it on the VM, and runs the
 * fetch-decode-execute loop. Compilation failure yields
 * `INTERPRET_COMPILE_ERROR`; a runtime type error during execution yields
 * `INTERPRET_RUNTIME_ERROR`; otherwise `INTERPRET_OK`.
 *
 * @param `source`  NUL-terminated source text to interpret.
 * @return        The outcome of interpretation.
 */
InterpretResult interpret(const char* source);

void push(Value value);

Value pop();

#endif
