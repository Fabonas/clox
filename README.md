# clox

A C implementation of **Lox**, the language from
[Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom.
clox is a bytecode virtual machine: it compiles Lox source to a compact
bytecode chunk and runs it on a stack-based interpreter — the same strategy
used by Lua, Python, and the JVM.

This is an educational project, worked through chapter by chapter. It
currently implements everything through **Chapter 23 — Jumping Back and
Forth**.

---

## Current state

Done (Crafting Interpreters chapters 14–23):

- **Scanner** (ch 16) — hand-written lexer producing a token stream.
- **Bytecode chunks** (ch 14) — growable opcode array, a parallel array of
  source line numbers, and a constant pool indexed by a single byte (up to
  256 constants).
- **Virtual machine** (ch 15) — fetch-decode-execute loop over a value stack.
- **Compiler** (ch 17) — a single-pass Pratt parser that compiles
  expressions into bytecode.
- **Types of values** (ch 18) — `Value` is a tagged union of `bool`,
  `nil`, and `number`.
- **Strings** (ch 19) — heap-allocated `ObjString` values, string
  concatenation with `+`, and string interning so identical strings share a
  single object.
- **Hash tables** (ch 20) — open-addressed hash table with linear probing,
  tombstones for deletion, and FNV-1a hashing for string keys.
- **Global variables** (ch 21) — `var` declarations at the top level,
  assignment, and lookup via the `vm.globals` table.
- **Local variables** (ch 22) — block-scoped locals stored directly on the
  VM stack, shadowing, and a compile-time stack-slot allocator.
- **Statements** (ch 21–22) — `print`, expression statements, `var`
  declarations, and brace-delimited blocks with nested scope.
- **Control flow** (ch 23) — `if`/`else`, `while`, `for`, short-circuiting
  `and`/`or`, and the jump/loop bytecode instructions that implement them.

Everything builds cleanly with `make` and runs. The REPL reads whole
statements per line (use `;` and `{}` when needed).

Not yet done: functions, closures, resolvers, garbage collection, classes,
and instances (chapters 24–28).

## Quick start

```bash
make                 # debug build -> ./clox
make run             # build + launch the REPL
./clox script.lox    # run a file
make release         # optimized build
make SANITIZE=1      # AddressSanitizer + UBSan
make clean
```

## Examples

The REPL reads one statement per line. Multi-line statements must be on a
single line for now:

```
$ ./clox
> var x = 1 + 2 * 3;
> print x;
7
> var s = "hello, " + "world!";
> print s;
hello, world!
> {
.. var a = 10;
.. print a;
.. }
10
> print a;
Undefined variable 'a'.
[line 1] in script
> var i = 0; while (i < 3) { print i; i = i + 1; }
0
1
2
> for (var j = 0; j < 3; j = j + 1) print j;
0
1
2
```

## Architecture

```
source text -> [scanner] -> tokens -> [compiler] -> chunk -> [VM] -> result
```

`interpret(source)` compiles the source into a `Chunk`, hands it to the VM,
and `run()` fetches opcodes one at a time, pushing, popping, and combining
values on the stack until `OP_RETURN` halts.

## Project layout

```
main.{c,h}      entry point: REPL and file runner
scanner.{c,h}   lexer: source -> tokens
compiler.{c,h}  single-pass Pratt parser -> bytecode chunk
chunk.{c,h}     bytecode chunk: code, line info, constant pool
value.{c,h}     runtime Value (tagged bool/nil/number/obj union) and ValueArray
vm.{c,h}        stack-based interpreter loop + globals/strings tables
object.{c,h}    heap objects: Obj, ObjString, string interning
table.{c,h}     hash table for globals and string interning
debug.{c,h}     chunk disassembler + execution tracing
memory.{c,h}    single reallocate() allocator + grow macros
common.h        shared header: integer aliases, debug flags
Makefile        build / run / release / clean / backup targets
```

## Bytecode reference

| Opcode             | Operand | Stack effect            | Description                                 |
|--------------------|---------|-------------------------|---------------------------------------------|
| `OP_CONSTANT`      | index   | `-> value`              | push `constants[index]`                     |
| `OP_NIL`           | —       | `-> nil`                | push nil                                    |
| `OP_TRUE`          | —       | `-> true`               | push true                                   |
| `OP_FALSE`         | —       | `-> false`              | push false                                  |
| `OP_POP`           | —       | `a -> `                 | discard top value                           |
| `OP_GET_LOCAL`     | slot    | `-> local[slot]`        | push local variable by stack slot           |
| `OP_SET_LOCAL`     | slot    | `a -> a`                | store top into local stack slot             |
| `OP_GET_GLOBAL`    | index   | `-> value`              | look up global name in `vm.globals`         |
| `OP_DEFINE_GLOBAL` | index   | `a -> `                 | define/redefine global from top of stack    |
| `OP_SET_GLOBAL`    | index   | `a -> a`                | assign to existing global, error if unknown |
| `OP_EQUAL`         | —       | `a b -> a==b`           | structural equality (any types)             |
| `OP_GREATER`       | —       | `a b -> a>b`            | greater-than (numbers)                      |
| `OP_LESS`          | —       | `a b -> a<b`            | less-than (numbers)                         |
| `OP_ADD`           | —       | `a b -> a+b`            | add numbers or concatenate strings          |
| `OP_SUBTRACT`      | —       | `a b -> a-b`            | subtract (numbers)                          |
| `OP_MULTIPLY`      | —       | `a b -> a*b`            | multiply (numbers)                          |
| `OP_DIVIDE`        | —       | `a b -> a/b`            | divide (numbers)                            |
| `OP_NEGATE`        | —       | `a -> -a`               | numeric negation                            |
| `OP_NOT`           | —       | `a -> !a`               | logical not (falsey -> true)                |
| `OP_PRINT`         | —       | `a -> `                 | pop and print value with newline            |
| `OP_JUMP`          | offset  | `... -> ...`            | unconditional 16-bit forward jump           |
| `OP_JUMP_IF_FALSE` | offset  | `cond -> `              | pop condition; jump forward if falsey       |
| `OP_LOOP`          | offset  | `... -> ...`            | unconditional 16-bit backward jump          |
| `OP_RETURN`        | —       | `a -> `                 | pop and halt                                |

The compiler desugars the comparison operators that have no dedicated
opcode: `!=` emits `OP_EQUAL` then `OP_NOT`; `<=` emits `OP_LESS` then
`OP_NOT`; `>=` emits `OP_GREATER` then `OP_NOT`. `OP_EQUAL` accepts any
value types (different types are always unequal); `OP_GREATER`/`OP_LESS`
and numeric arithmetic opcodes require numbers and trap with a runtime error
otherwise. `OP_NOT` treats `nil` and `false` as falsey and everything else
(including `0`) as truthy.

Control flow is compiled with explicit 16-bit jump offsets. `OP_JUMP` is
used for the unconditional branch around an `else` block or out of a loop;
`OP_JUMP_IF_FALSE` implements conditional branches for `if` conditions and
the left-hand side of short-circuiting `and`/`or`; `OP_LOOP` jumps backward
to the top of a `while`/`for` body. `and` and `or` are compiled as
short-circuit jumps rather than stack operators.

## Debug tracing

`common.h` unconditionally `#define`s `DEBUG_PRINT_CODE` and
`DEBUG_TRACE_EXECUTION`. With tracing on, the compiler prints a disassembly
of the generated chunk and the VM prints the stack before each instruction:

```
== code ==
0000    1 OP_CONSTANT   0 '1'
0002    | OP_CONSTANT   1 '2'
0004    | OP_ADD
0005    1 OP_RETURN
     [ 3 ]
3
```

To silence it, comment out the two `#define`s in `common.h` and rebuild.
(`make release` adds `-DNDEBUG` but does **not** disable tracing.)

## Exit codes

| Code | Meaning        |
|------|----------------|
| 64   | usage error    |
| 65   | compile error  |
| 70   | runtime error  |
| 74   | I/O error      |

## Roadmap

- [x] Chunks, disassembler, and VM loop (ch 14–15)
- [x] Scanner (ch 16)
- [x] Compiling expressions (ch 17)
- [x] Types of values: bool, nil, numbers (ch 18)
- [x] Heap-allocated strings and string interning (ch 19)
- [x] Hash tables (ch 20)
- [x] Global variables (ch 21)
- [x] Local variables and block scope (ch 22)
- [x] Jumping back and forth: `if`/`else`, `while`, `for`, `and`/`or` (ch 23)
- [ ] Functions and closures (ch 24–25)
- [ ] Garbage collection, classes, instances, methods (ch 26–28)

## License

Educational implementation. See
[Crafting Interpreters](https://craftinginterpreters.com/) for its license
and attribution terms.
