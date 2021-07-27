#include <unity.h>
#include <stdbool.h>

void setUp(void) {}
void tearDown(void) {}


void test_can_add_numbers() {
    char program[] =  "val x = 10; val y = 20;";
    TEST_ASSERT_FALSE(false);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_can_add_numbers);

    return UNITY_END();
}