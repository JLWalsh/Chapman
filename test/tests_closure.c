
#include <unity.h>
#include <stdbool.h>
#include <vm/chapman.h>
#include "utils.h"

void setUp(void) {}
void tearDown(void) {}

void test_closure_captures_parent_variables() {
    char program[] = "val x = 10; #closure() { return x; } return closure();";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(10, result.number_value);
}

void test_closure_modifies_parent_variables() {
    char program[] = "val x = 10; #closure() { x = 20; } closure(); return x;";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(20, result.number_value);
}

void test_closure_promotes_captured_variable_to_heap_when_parent_function_returns() {
    char program[] = "#enclosing() { val x = 10; #closure() { return x; } return closure; } val closure = enclosing(); return closure();";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(10, result.number_value);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_closure_captures_parent_variables);
    RUN_TEST(test_closure_modifies_parent_variables);
    RUN_TEST(test_closure_promotes_captured_variable_to_heap_when_parent_function_returns);

    return UNITY_END();
}