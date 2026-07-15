# clox

A C implementation of **Lox**, the language from
[Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom.
clox is a bytecode virtual machine: it compiles Lox source to a compact
bytecode chunk and runs it on a stack-based interpreter — the same strategy
used by Lua, Python, and the JVM.

This is an educational project, worked through chapter by chapter. It
currently implements everything through **Chapter 18 — Types of
Values**.

---

## Current state

Done (Crafting Interpreters chapters 14–18):

- **Scanner** (ch 16) — hand-written lexer producing a token stream.
- **Bytecode chunks** (ch 14) — growable opcode array, a parallel array of
  source line numbers, and a constant pool indexed by a single byte (up to
  256 constants).
- **Virtual machine** (ch 15) — fetch-decode-execute loop over a value stack.
- **Compiler** (ch 17) — a single-pass Pratt parser that compiles
  **expressions** into bytecode, then hands the chunk to the VM to run.
  Supports numbers, the literals `true`/`false`/`nil`, unary `-` and `!`,
  the binary arithmetic `+ - * /`, comparisons `< <= > >=`, equality
  `== !=`, and parenthesized grouping.
- **Types of values** (ch 18) — `Value` is a tagged union of `bool`,
  `nil`, and `number`. Arithmetic and comparison operands are type-checked
  at runtime; mismatched types raise a runtime error. Equality uses
  structural comparison that treats different types as unequal.

Everything builds cleanly with `make` and runs. The REPL evaluates one
expression per line.

Not yet done: `print`/`var`/control-flow statements, strings and other
heap objects, functions, closures, and classes.

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

The REPL evaluates one expression per line. With debug tracing on (the
default — see below) the VM prints a disassembly trace; the result is the
final line:

```
$ ./clox
> 1 + 2 * 3
7
> -2 * (3 + 4)
-14
> 1 < 2
true
> !nil
true
> 1 == true
false
```

## Architecture

```
source text -> [scanner] -> tokens -> [compiler] -> chunk -> [VM] -> result
```

`interpret(source)` compiles the source into a `Chunk`, hands it to the VM,
and `run()` fetches opcodes one at a time, pushing, popping, and combining
values on the stack until `OP_RETURN` halts and prints the top value.

## Project layout

```
main.c          entry point: REPL and file runner
scanner.{c,h}   lexer: source -> tokens
compiler.{c,h}  single-pass Pratt parser -> bytecode chunk
chunk.{c,h}     bytecode chunk: code, line info, constant pool
value.{c,h}     runtime Value (tagged bool/nil/number union) and ValueArray
vm.{c,h}        stack-based interpreter loop
debug.{c,h}     chunk disassembler + execution tracing
memory.{c,h}    single reallocate() allocator + grow macros
common.h        shared header: integer aliases, debug flags
Makefile        build / run / release / clean / backup targets
```

## Bytecode reference

| Opcode         | Operand | Stack effect       | Description                          |
|----------------|---------|--------------------|--------------------------------------|
| `OP_CONSTANT`  | index   | `-> value`         | push `constants[index]`             |
| `OP_NIL`       | —       | `-> nil`           | push nil                             |
| `OP_TRUE`      | —       | `-> true`          | push true                            |
| `OP_FALSE`     | —       | `-> false`         | push false                           |
| `OP_EQUAL`     | —       | `a b -> a==b`      | structural equality (any types)     |
| `OP_GREATER`   | —       | `a b -> a>b`       | greater-than (numbers)              |
| `OP_LESS`      | —       | `a b -> a<b`       | less-than (numbers)                  |
| `OP_ADD`       | —       | `a b -> a+b`       | add (numbers)                        |
| `OP_SUBTRACT`  | —       | `a b -> a-b`       | subtract (numbers)                   |
| `OP_MULTIPLY`  | —       | `a b -> a*b`       | multiply (numbers)                   |
| `OP_DIVIDE`    | —       | `a b -> a/b`       | divide (numbers)                     |
| `OP_NEGATE`    | —       | `a -> -a`          | numeric negation                     |
| `OP_NOT`       | —       | `a -> !a`          | logical not (falsey -> true)         |
| `OP_RETURN`    | —       | `a -> ` (prints)   | pop, print, and halt                 |

The compiler desugars the comparison operators that have no dedicated
opcode: `!=` emits `OP_EQUAL` then `OP_NOT`; `<=` emits `OP_GREATER` then
`OP_NOT`; `>=` emits `OP_LESS` then `OP_NOT`. `OP_EQUAL` accepts any value
types (different types are always unequal); `OP_GREATER`/`OP_LESS` and the
arithmetic opcodes require numbers and trap with a runtime error otherwise.
`OP_NOT` treats `nil` and `false` as falsey and everything else (including
`0`) as truthy.

## Debug tracing

`common.h` unconditionally `#define`s `DEBUG_PRINT_CODE` and
`DEBUG_TRACE_EXECUTION`. With tracing on, the VM prints the stack and the
disassembly of each instruction before executing it:

```
     [ 1 ]
0000    1 OP_CONSTANT   0 '1'
0002    | OP_CONSTANT   1 '2'
     [ 1 ][ 2 ]
0006    | OP_MULTIPLY
     [ 1 ][ 6 ]
0007    | OP_ADD
     [ 7 ]
0008    2 OP_RETURN
7
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
- [ ] Objects and strings (ch 19+)
- [ ] Statements: `print`, `var`, control flow (ch 21–23)
- [ ] Functions, closures, and classes (ch 24–28)

## License

Educational implementation. See
[Crafting Interpreters](https://craftinginterpreters.com/) for its license
and attribution terms.
