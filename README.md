# clox

A C implementation of the bytecode virtual machine for the **Lox** language from [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom.

This repo contains the core VM: chunk-based bytecode storage, a constant pool, line information, a disassembler, and a working stack-based interpreter.

## Project structure

| File(s) | Description |
|---------|-------------|
| `chunk.c/h` | Bytecode chunk representation and constant pool |
| `common.h` | Shared macros and configuration |
| `debug.c/h` | Chunk disassembly helpers |
| `memory.c/h` | Memory allocation wrappers |
| `types.h` | Common type aliases used by the VM |
| `value.c/h` | Lox value representation |
| `vm.c/h` | Stack-based bytecode interpreter |
| `main.c` | Demo entry point that builds and runs a sample chunk |
| `Makefile` | Build, run, and clean tasks |

## Building

```bash
make
```

Build an optimized release binary:

```bash
make release
```

Run with AddressSanitizer / UBSan:

```bash
make SANITIZE=1 run
```

## Running

```bash
make run
```

The current demo constructs a small bytecode chunk, disassembles it, and interprets it.

## Cleaning up

```bash
make clean
```

## License

This is a learning project based on *Crafting Interpreters*.
