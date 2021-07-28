#include <unity.h>
#include <stdbool.h>
#include <vm/chapman.h>
#include "utils.h"

void setUp(void) {}
void tearDown(void) {}

void test_can_add_positive_numbers() {
    char program[] =  "val x = 10; val y = 20; return x + y;";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(30, result.number_value);
}

void test_can_add_negative_numbers() {
    char program[] =  "val x = -27; val y = -43; return x + y;";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(-70, result.number_value);
}

void test_can_add_numbers_with_different_signs() {
    char program[] =  "val x = 42; val y = -42; return x + y;";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(0, result.number_value);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_can_add_positive_numbers);
    RUN_TEST(test_can_add_negative_numbers);
    RUN_TEST(test_can_add_numbers_with_different_signs);

    return UNITY_END();
}