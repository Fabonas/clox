#include "run.h"
#include "memory.h"
#include <stdlib.h>

void initRunArray(RunArray *array) {
    array->runs = NULL;
    array->capacity = 0;
    array->count = 0;
}

void freeRunArray(RunArray *array) {
    FREE_ARRAY(Run, array->runs, array->capacity);
    initRunArray(array);
}

Run *insertLine(int line, RunArray *runArray, Run *lastRun) {
    if (lastRun != NULL && lastRun->line == line) {
        lastRun->count++;
        return lastRun;
    }

    if (runArray->runs == NULL || runArray->capacity < runArray->count + 1) {
        int oldCapacity = runArray->capacity;
        runArray->capacity = GROW_CAPACITY(oldCapacity);
        runArray->runs = GROW_ARRAY(Run, runArray->runs, oldCapacity, runArray->capacity);
    }

    Run run = {.count = 1, .line = line};

    runArray->runs[runArray->count] = run;
    runArray->count++;
    return &runArray->runs[runArray->count - 1];
}
