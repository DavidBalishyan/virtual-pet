#include <unity.h>
#include <Arduino.h>
#include "../../lib/Display/animation_manager.h"

void setUp(void) {
    setMillis(0);
}

void tearDown(void) {}

void test_single_frame_never_advances(void) {
    AnimationManager anim(1);
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());

    anim.update();
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());

    setMillis(10000);
    anim.update();
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());
}

void test_starts_at_frame_zero(void) {
    AnimationManager anim(3);
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());
}

void test_advances_after_duration(void) {
    AnimationManager anim(2);
    // Frame duration = 200ms (default)
    setMillis(199);
    anim.update();
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());

    setMillis(200);
    anim.update();
    TEST_ASSERT_EQUAL_INT(1, anim.getCurrentFrame());
}

void test_wraps_around(void) {
    AnimationManager anim(2);
    setMillis(200);
    anim.update();
    TEST_ASSERT_EQUAL_INT(1, anim.getCurrentFrame());

    setMillis(400);
    anim.update();
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());

    setMillis(600);
    anim.update();
    TEST_ASSERT_EQUAL_INT(1, anim.getCurrentFrame());
}

void test_reset_goes_to_frame_zero(void) {
    AnimationManager anim(2);
    setMillis(200);
    anim.update();
    TEST_ASSERT_EQUAL_INT(1, anim.getCurrentFrame());

    anim.reset();
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());
}

void test_reset_updates_timestamp(void) {
    AnimationManager anim(2);
    setMillis(500);
    anim.reset();  // sets lastFrameAdvanceTime = 500

    setMillis(600);  // only 100ms later — should NOT advance
    anim.update();
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());

    setMillis(700);
    anim.update();
    TEST_ASSERT_EQUAL_INT(1, anim.getCurrentFrame());
}

void test_custom_frame_duration(void) {
    AnimationManager anim(2, 500);  // 500ms per frame
    setMillis(499);
    anim.update();
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());

    setMillis(500);
    anim.update();
    TEST_ASSERT_EQUAL_INT(1, anim.getCurrentFrame());
}

void test_no_divide_by_zero(void) {
    AnimationManager anim(0);
    anim.update();  // should not crash
    TEST_ASSERT_EQUAL_INT(0, anim.getCurrentFrame());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_single_frame_never_advances);
    RUN_TEST(test_starts_at_frame_zero);
    RUN_TEST(test_advances_after_duration);
    RUN_TEST(test_wraps_around);
    RUN_TEST(test_reset_goes_to_frame_zero);
    RUN_TEST(test_reset_updates_timestamp);
    RUN_TEST(test_custom_frame_duration);
    RUN_TEST(test_no_divide_by_zero);
    return UNITY_END();
}
