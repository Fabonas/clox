# clox

A basic C implementation of the bytecode virtual machine for the **Lox** language from [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom.

This repo contains the core VM foundations: chunk-based bytecode storage, constant pools, line information, and a disassembler.

## Project structure

| File(s) | Description |
|---------|-------------|
| `chunk.c/h` | Bytecode chunk representation and constant pool |
| `common.h` | Shared macros and configuration |
| `debug.c/h` | Chunk disassembly helpers |
| `memory.c/h` | Memory allocation wrappers |
| `run.c/h` | VM execution helpers |
| `value.c/h` | Lox value representation |
| `main.c` | Small demo entry point |
| `Makefile` | Build and clean tasks |

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

## Cleaning up

```bash
make clean
```

## License

This is a learning project based on *Crafting Interpreters*.
