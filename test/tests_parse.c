
#include <unity.h>
#include <stdbool.h>
#include <vm/chapman.h>
#include "utils.h"

void setUp(void) {}
void tearDown(void) {}

void test_can_parse_number_without_decimals() {
    char program[] =  "val x = 122; return x;";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(122, result.number_value);
}

void test_can_parse_number_with_decimals() {
    char program[] =  "val x = 10.1234; return x;";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_NUMBER, result.type);
    TEST_ASSERT_EQUAL(10.1234, result.number_value);
}

void test_cannot_parse_number_with_missing_decimals() {
    char program[] =  "val x = -22.; return x;";

    bool compiles = doescompile(program);

    TEST_ASSERT_FALSE(compiles);
}

void test_can_parse_string() {
    char program[] =  "val x = \"Hello, world!\"; return x;";

    ch_primitive result = run(program);

    TEST_ASSERT_EQUAL(PRIMITIVE_OBJECT, result.type);
    char parsed_string[] = "Hello, world!";
    TEST_ASSERT_EQUAL_CHAR_ARRAY(AS_STRING(result.object_value)->value, parsed_string, sizeof(parsed_string) - 1);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_can_parse_number_without_decimals);
    RUN_TEST(test_can_parse_number_with_decimals);
    RUN_TEST(test_cannot_parse_number_with_missing_decimals);
    RUN_TEST(test_can_parse_string);

    return UNITY_END();
}