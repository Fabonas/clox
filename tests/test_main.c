#include <stdio.h>
#include "../minunit.h"

int minunit_tests_run = 0;
int minunit_tests_passed = 0;
int minunit_tests_failed = 0;
const char *minunit_last_message = NULL;

const char *run_chunk_tests(void);

int main(void) {
    run_chunk_tests();

    MU_REPORT();
}
