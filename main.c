/**
 * @file main.c
 * Introduction
 * ------------
 * The command-line driver. It boots the VM, then either runs an interactive
 * REPL or reads a source file into memory and interprets it. Exit codes are
 * chosen to match the conventions used in *Crafting Interpreters*.
 *
 * Program entry point: REPL and file execution.
 *
 * clox runs in two modes:
 *   * no arguments   -> interactive REPL, reading one line at a time
 *   * one argument   -> run the given file as the program source
 *   * anything else  -> print usage and exit 64
 *
 * The REPL reads whole statements per line; use `;` and `{}` when needed.
 *
 * Exit codes follow the Unix convention: 64 for usage errors, 65 for compile
 * errors, 70 for runtime errors (mirroring *Crafting Interpreters*).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

static void repl() {
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    usize fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    usize bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char*           source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char* argv[]) {
    initVM();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}