#include "../minunit.h"

#include "test_chunk.c"
#include "test_guide.c"

int main(void) {
    MU_RUN_SUITE(chunk_suite);
    MU_RUN_SUITE(guide_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
