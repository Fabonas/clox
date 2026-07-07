#include "../minunit.h"
#include "../value.h"

/*
 * This file is a short guide to writing tests with minunit for clox.
 *
 * A test is a static void function with no arguments, declared with
 * MU_TEST(name).  Inside the test you call one or more assertions.
 * If an assertion fails, the test stops and the runner counts it as a
 * failure.  If all assertions pass, the test passes.
 */

/* mu_check(expression) fails when the expression is false. */
MU_TEST(test_guide_check) {
    mu_check(1 + 1 == 2);
}

/* mu_assert(expression, "message") fails with a custom message. */
MU_TEST(test_guide_assert) {
    mu_assert(2 + 2 == 4, "basic arithmetic should work");
}

/* mu_assert_int_eq(expected, actual) compares two integers. */
MU_TEST(test_guide_int_eq) {
    mu_assert_int_eq(42, 40 + 2);
}

/* mu_assert_double_eq(expected, actual) compares two doubles. */
MU_TEST(test_guide_double_eq) {
    mu_assert_double_eq(3.14159, 3.14159);
}

/* mu_assert_string_eq(expected, actual) compares two C strings. */
MU_TEST(test_guide_string_eq) {
    mu_assert_string_eq("hello", "hello");
}

/* In this clox implementation Value is currently a typedef for double. */
MU_TEST(test_guide_value) {
    Value value = 123.456;
    mu_assert_double_eq(123.456, value);
}

/*
 * A suite groups related tests.  Declare it with MU_TEST_SUITE(name)
 * and register each test with MU_RUN_TEST(test_name).  The runner in
 * test_main.c will execute every suite you add there.
 */
MU_TEST_SUITE(guide_suite) {
    MU_RUN_TEST(test_guide_check);
    MU_RUN_TEST(test_guide_assert);
    MU_RUN_TEST(test_guide_int_eq);
    MU_RUN_TEST(test_guide_double_eq);
    MU_RUN_TEST(test_guide_string_eq);
    MU_RUN_TEST(test_guide_value);
}
