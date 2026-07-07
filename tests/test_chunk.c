#include "../minunit.h"
#include "../chunk.h"

MU_TEST(test_init_chunk_clears_fields) {
    Chunk chunk;
    initChunk(&chunk);

    mu_assert_int_eq(0, chunk.count);
    mu_assert_int_eq(0, chunk.capacity);
    mu_check(chunk.code == NULL);
    mu_assert_int_eq(0, chunk.constants.count);
    mu_assert_int_eq(0, chunk.runs.count);

    freeChunk(&chunk);
}

MU_TEST(test_write_chunk_increases_count) {
    Chunk chunk;
    initChunk(&chunk);

    writeChunk(&chunk, OP_RETURN, 1);
    mu_assert_int_eq(1, chunk.count);

    writeChunk(&chunk, OP_CONSTANT, 1);
    mu_assert_int_eq(2, chunk.count);

    freeChunk(&chunk);
}

MU_TEST(test_write_chunk_stores_byte) {
    Chunk chunk;
    initChunk(&chunk);

    writeChunk(&chunk, OP_RETURN, 1);
    mu_assert_int_eq(OP_RETURN, chunk.code[0]);

    freeChunk(&chunk);
}

MU_TEST(test_add_constant_stores_value) {
    Chunk chunk;
    initChunk(&chunk);

    int index = addConstant(&chunk, 3.14);
    mu_assert_int_eq(0, index);
    mu_assert_int_eq(1, chunk.constants.count);
    mu_assert_double_eq(3.14, chunk.constants.values[0]);

    freeChunk(&chunk);
}

MU_TEST(test_chunk_grows_when_full) {
    Chunk chunk;
    initChunk(&chunk);

    for (int i = 0; i < 10; i++) {
        writeChunk(&chunk, OP_RETURN, i + 1);
    }

    mu_assert_int_eq(10, chunk.count);
    mu_check(chunk.capacity >= chunk.count);
    mu_check(chunk.code != NULL);

    freeChunk(&chunk);
}

MU_TEST_SUITE(chunk_suite) {
    MU_RUN_TEST(test_init_chunk_clears_fields);
    MU_RUN_TEST(test_write_chunk_increases_count);
    MU_RUN_TEST(test_write_chunk_stores_byte);
    MU_RUN_TEST(test_add_constant_stores_value);
    MU_RUN_TEST(test_chunk_grows_when_full);
}
