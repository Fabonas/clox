# clox

A basic C implementation of the bytecode virtual machine for the **Lox** language from [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom.

This repo contains the core VM foundations: chunk-based bytecode storage, constant pools, line information, a disassembler, and a small test harness.

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
| `Makefile` | Build, test, and clean tasks |
| `tests/` | Unit tests using `minunit.h` |

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

## Testing

```bash
make test
```

## Cleaning up

```bash
make clean
```

## License

This is a learning project based on *Crafting Interpreters*.
