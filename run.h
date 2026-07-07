#ifndef clox_run_h
#define clox_run_h

typedef struct {
    int line;
    int count;
} Run;

typedef struct {
    int capacity;
    int count;
    Run *runs;
} RunArray;

void initRunArray(RunArray *runs);
void freeRunArray(RunArray *runs);
Run *insertLine(int line, RunArray *runArray, Run *lastRun);

#endif
