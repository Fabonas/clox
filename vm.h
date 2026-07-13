/**
 * @file vm.h
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
#include "value.h"

/**
 * Maximum number of values that can live on the stack at once. A fixed-size
 * stack keeps allocation out of the hot path; exceeding it is a (future)
 * runtime error.
 */
#define STACK_MAX 256

/**
 * The interpreter's runtime state.
 */
typedef struct {
    Chunk* chunk;             /**< bytecode currently being executed */
    u8*    ip;                /**< instruction pointer into `chunk->code` */
    Value  stack[STACK_MAX];  /**< the value stack */
    Value* stackTop;          /**< points just past the top value */
} VM;

/**
 * Outcome of interpreting a program. `INTERPRET_OK` means success; the two
 * error variants distinguish front-end (compile) and back-end (runtime)
 * failures.
 */
typedef enum {
    INTERPRET_OK,             /**< program ran successfully */
    INTERPRET_COMPILE_ERROR,  /**< compilation failed */
    INTERPRET_RUNTIME_ERROR,  /**< execution failed at runtime */
} InterpretResult;

/**
 * Initialize the VM's runtime state (resets the stack).
 */
void initVM();

/**
 * Tear down VM state.
 *
 * Nothing to clean up yet; reserved for future GC/object teardown.
 */
void freeVM();

/**
 * Compile then execute `source`.
 *
 * Today only the (stub) compilation step runs; `run()` is not yet invoked.
 * Once the compiler emits a chunk this will install it on the VM and call
 * `run()`, translating its result into an `InterpretResult`.
 *
 * @param `source`  NUL-terminated source text to interpret.
 * @return        `INTERPRET_OK` on success (currently always), or an error
 *                variant once compile/runtime failures are wired in.
 */
InterpretResult interpret(const char* source);

/**
 * Push a value onto the stack.
 *
 * @param `value`  Value to push.
 */
void push(Value value);

/**
 * Pop and return the top value from the stack.
 *
 * @return The value that was on top of the stack.
 */
Value pop();

#endif
