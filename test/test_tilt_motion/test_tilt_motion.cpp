#include <unity.h>
#include <cmath>
#include "../../lib/Imu/tilt_motion.h"

void setUp(void) {}
void tearDown(void) {}

void test_constructor_centres_pet(void) {
    TiltMotion motion;
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetX());
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetY());
}

void test_zero_tilt_keeps_centred(void) {
    TiltMotion motion;
    // 100 frames of zero tilt should converge to (0, 0)
    for (int i = 0; i < 100; i++) {
        motion.update(0.0f, 0.0f);
    }
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetX());
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetY());
}

void test_positive_x_tilt_moves_right(void) {
    TiltMotion motion;
    // Feed a strong positive X tilt repeatedly to overcome smoothing
    for (int i = 0; i < 100; i++) {
        motion.update(0.5f, 0.0f);
    }
    TEST_ASSERT_TRUE(motion.getOffsetX() > 0);
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetY());
}

void test_negative_x_tilt_moves_left(void) {
    TiltMotion motion;
    for (int i = 0; i < 100; i++) {
        motion.update(-0.5f, 0.0f);
    }
    TEST_ASSERT_TRUE(motion.getOffsetX() < 0);
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetY());
}

void test_clamp_max_offset_x(void) {
    TiltMotion motion;
    // Extreme tilt should be clamped at MAX_OFFSET_X
    for (int i = 0; i < 200; i++) {
        motion.update(10.0f, 0.0f);
    }
    int offset = motion.getOffsetX();
    TEST_ASSERT_TRUE(offset <= 25);
    TEST_ASSERT_TRUE(offset > 20);
}

void test_clamp_max_offset_y(void) {
    TiltMotion motion;
    for (int i = 0; i < 200; i++) {
        motion.update(0.0f, 10.0f);
    }
    int offset = motion.getOffsetY();
    TEST_ASSERT_TRUE(offset <= 20);
    TEST_ASSERT_TRUE(offset > 15);
}

void test_smoothing_gradual(void) {
    TiltMotion motion;
    // Single frame of strong tilt should only move partway due to smoothing
    motion.update(1.0f, 0.0f);
    int first = motion.getOffsetX();
    // After many frames at same tilt, should be further
    for (int i = 0; i < 50; i++) {
        motion.update(1.0f, 0.0f);
    }
    int later = motion.getOffsetX();
    TEST_ASSERT_TRUE(later > first);
    TEST_ASSERT_TRUE(first < 25);
}

void test_recentres_when_tilt_removed(void) {
    TiltMotion motion;
    // Push pet right
    for (int i = 0; i < 100; i++) {
        motion.update(0.5f, 0.0f);
    }
    // Then remove tilt
    for (int i = 0; i < 150; i++) {
        motion.update(0.0f, 0.0f);
    }
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetX());
    TEST_ASSERT_EQUAL_INT(0, motion.getOffsetY());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_constructor_centres_pet);
    RUN_TEST(test_zero_tilt_keeps_centred);
    RUN_TEST(test_positive_x_tilt_moves_right);
    RUN_TEST(test_negative_x_tilt_moves_left);
    RUN_TEST(test_clamp_max_offset_x);
    RUN_TEST(test_clamp_max_offset_y);
    RUN_TEST(test_smoothing_gradual);
    RUN_TEST(test_recentres_when_tilt_removed);
    return UNITY_END();
}
