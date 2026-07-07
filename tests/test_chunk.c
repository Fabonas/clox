#include "../minunit.h"
#include "../chunk.h"

static const char *test_init_chunk_clears_fields(void) {
    Chunk chunk;
    initChunk(&chunk);

    MU_ASSERT_INT_EQ(0, chunk.count);
    MU_ASSERT_INT_EQ(0, chunk.capacity);
    MU_ASSERT_PTR_NULL(chunk.code);
    MU_ASSERT_INT_EQ(0, chunk.constants.count);
    MU_ASSERT_INT_EQ(0, chunk.runs.count);

    freeChunk(&chunk);
    return NULL;
}

static const char *test_write_chunk_increases_count(void) {
    Chunk chunk;
    initChunk(&chunk);

    writeChunk(&chunk, OP_RETURN, 1);
    MU_ASSERT_INT_EQ(1, chunk.count);

    writeChunk(&chunk, OP_CONSTANT, 1);
    MU_ASSERT_INT_EQ(2, chunk.count);

    freeChunk(&chunk);
    return NULL;
}

static const char *test_write_chunk_stores_byte(void) {
    Chunk chunk;
    initChunk(&chunk);

    writeChunk(&chunk, OP_RETURN, 1);
    MU_ASSERT_UINT8_EQ(OP_RETURN, chunk.code[0]);

    freeChunk(&chunk);
    return NULL;
}

static const char *test_add_constant_stores_value(void) {
    Chunk chunk;
    initChunk(&chunk);

    int index = addConstant(&chunk, 3.14);
    MU_ASSERT_INT_EQ(0, index);
    MU_ASSERT_INT_EQ(1, chunk.constants.count);
    MU_ASSERT_DOUBLE_EQ(3.14, chunk.constants.values[0]);

    freeChunk(&chunk);
    return NULL;
}

static const char *test_chunk_grows_when_full(void) {
    Chunk chunk;
    initChunk(&chunk);

    for (int i = 0; i < 10; i++) {
        writeChunk(&chunk, OP_RETURN, i + 1);
    }

    MU_ASSERT_INT_EQ(10, chunk.count);
    MU_ASSERT("capacity should be >= count", chunk.capacity >= chunk.count);
    MU_ASSERT_PTR_NOT_NULL(chunk.code);

    freeChunk(&chunk);
    return NULL;
}

const char *run_chunk_tests(void) {
    printf("\nchunk.c tests:\n");
    MU_RUN_TEST(test_init_chunk_clears_fields);
    MU_RUN_TEST(test_write_chunk_increases_count);
    MU_RUN_TEST(test_write_chunk_stores_byte);
    MU_RUN_TEST(test_add_constant_stores_value);
    MU_RUN_TEST(test_chunk_grows_when_full);
    return NULL;
}
