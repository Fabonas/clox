#ifndef clox_minunit_h
#define clox_minunit_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern int minunit_tests_run;
extern int minunit_tests_passed;
extern int minunit_tests_failed;
extern const char *minunit_last_message;

#define MU_RUN_TEST(test) do {                                              \
    const char *message = test();                                           \
    minunit_tests_run++;                                                    \
    if (message == NULL) {                                                  \
        minunit_tests_passed++;                                             \
        printf("  PASS: %s\n", #test);                                       \
    } else {                                                                \
        minunit_tests_failed++;                                             \
        printf("  FAIL: %s\n    %s\n", #test, message);                     \
    }                                                                       \
} while (0)

#define MU_ASSERT(message, test) do {                                       \
    if (!(test)) {                                                          \
        return (message);                                                   \
    }                                                                       \
} while (0)

#define MU_ASSERT_INT_EQ(expected, actual) do {                             \
    int mu_expected = (expected);                                           \
    int mu_actual = (actual);                                               \
    if (mu_expected != mu_actual) {                                         \
        static char buf[256];                                               \
        snprintf(buf, sizeof(buf),                                          \
                 "%s:%d: expected %d, got %d",                              \
                 __FILE__, __LINE__, mu_expected, mu_actual);               \
        return buf;                                                         \
    }                                                                       \
} while (0)

#define MU_ASSERT_UINT8_EQ(expected, actual) do {                           \
    unsigned int mu_expected = (expected);                                  \
    unsigned int mu_actual = (actual);                                      \
    if (mu_expected != mu_actual) {                                         \
        static char buf[256];                                               \
        snprintf(buf, sizeof(buf),                                          \
                 "%s:%d: expected %u, got %u",                              \
                 __FILE__, __LINE__, mu_expected, mu_actual);               \
        return buf;                                                         \
    }                                                                       \
} while (0)

#define MU_ASSERT_DOUBLE_EQ(expected, actual) do {                          \
    double mu_expected = (expected);                                        \
    double mu_actual = (actual);                                            \
    if (fabs(mu_expected - mu_actual) > 0.0001) {                           \
        static char buf[256];                                               \
        snprintf(buf, sizeof(buf),                                          \
                 "%s:%d: expected %f, got %f",                              \
                 __FILE__, __LINE__, mu_expected, mu_actual);               \
        return buf;                                                         \
    }                                                                       \
} while (0)

#define MU_ASSERT_PTR_EQ(expected, actual) do {                             \
    const void *mu_expected = (expected);                                   \
    const void *mu_actual = (actual);                                       \
    if (mu_expected != mu_actual) {                                         \
        static char buf[256];                                               \
        snprintf(buf, sizeof(buf),                                          \
                 "%s:%d: expected %p, got %p",                              \
                 __FILE__, __LINE__, mu_expected, mu_actual);               \
        return buf;                                                         \
    }                                                                       \
} while (0)

#define MU_ASSERT_PTR_NULL(actual) do {                                     \
    const void *mu_actual = (actual);                                       \
    if (mu_actual != NULL) {                                                \
        static char buf[256];                                               \
        snprintf(buf, sizeof(buf),                                          \
                 "%s:%d: expected NULL, got %p",                            \
                 __FILE__, __LINE__, mu_actual);                            \
        return buf;                                                         \
    }                                                                       \
} while (0)

#define MU_ASSERT_PTR_NOT_NULL(actual) do {                                 \
    const void *mu_actual = (actual);                                       \
    if (mu_actual == NULL) {                                                \
        static char buf[256];                                               \
        snprintf(buf, sizeof(buf),                                          \
                 "%s:%d: expected non-NULL pointer",                        \
                 __FILE__, __LINE__);                                       \
        return buf;                                                         \
    }                                                                       \
} while (0)

#define MU_REPORT() do {                                                    \
    printf("\n");                                                           \
    printf("Tests run:    %d\n", minunit_tests_run);                         \
    printf("Tests passed: %d\n", minunit_tests_passed);                      \
    printf("Tests failed: %d\n", minunit_tests_failed);                      \
    return (minunit_tests_failed == 0) ? 0 : 1;                             \
} while (0)

#endif
