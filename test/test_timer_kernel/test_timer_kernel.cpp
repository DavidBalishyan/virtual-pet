#include <unity.h>
#include <Arduino.h>
#include "../../lib/Timer/timer_kernel.h"

// These run on the native/x86 host, where neither __XTENSA__ nor USE_ASM_TIMER is
// defined, so they exercise the C++ fallback in timer_kernel.cpp. That fallback is
// the reference the assembly must match exactly, so these also pin down the
// contract the Xtensa routine has to satisfy on the device.

void setUp(void) {}

void tearDown(void) {}

// First call anchors *last to now and reports no ticks.
void test_first_call_anchors_and_returns_zero(void) {
    time_t last = 0;
    int ticks = timing::timerDecayTicks(1000, &last, 6);
    TEST_ASSERT_EQUAL_INT(0, ticks);
    TEST_ASSERT_EQUAL_INT(1000, (int)last);
}

// Less than one interval elapsed: no ticks, timer left untouched.
void test_sub_interval_no_ticks(void) {
    time_t last = 1000;
    int ticks = timing::timerDecayTicks(1005, &last, 6);  // 5 < 6
    TEST_ASSERT_EQUAL_INT(0, ticks);
    TEST_ASSERT_EQUAL_INT(1000, (int)last);  // unchanged
}

// Exactly one interval: one tick, timer advances by the interval.
void test_single_tick(void) {
    time_t last = 1000;
    int ticks = timing::timerDecayTicks(1006, &last, 6);
    TEST_ASSERT_EQUAL_INT(1, ticks);
    TEST_ASSERT_EQUAL_INT(1006, (int)last);
}

// Catch-up: many intervals at once (e.g. powered off), and the leftover remainder
// is preserved so ticks do not drift.
void test_multiple_ticks_keeps_remainder(void) {
    time_t last = 1000;
    int ticks = timing::timerDecayTicks(1020, &last, 6);  // 20 / 6 = 3 rem 2
    TEST_ASSERT_EQUAL_INT(3, ticks);
    TEST_ASSERT_EQUAL_INT(1018, (int)last);  // now - remainder = 1020 - 2
}

// Clock ran backwards (RTC reset): resync to now, report no ticks.
void test_backwards_clock_resyncs(void) {
    time_t last = 5000;
    int ticks = timing::timerDecayTicks(1000, &last, 6);
    TEST_ASSERT_EQUAL_INT(0, ticks);
    TEST_ASSERT_EQUAL_INT(1000, (int)last);
}

// Two back-to-back catch-ups stay drift-free: the preserved remainder means the
// tick boundary is honoured across calls.
void test_no_drift_across_calls(void) {
    time_t last = 0;
    timing::timerDecayTicks(100, &last, 6);                 // anchor at 100
    int t1 = timing::timerDecayTicks(109, &last, 6);        // 9/6 = 1 rem 3 -> last 106
    int t2 = timing::timerDecayTicks(112, &last, 6);        // 6/6 = 1 rem 0 -> last 112
    TEST_ASSERT_EQUAL_INT(1, t1);
    TEST_ASSERT_EQUAL_INT(1, t2);
    TEST_ASSERT_EQUAL_INT(112, (int)last);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_first_call_anchors_and_returns_zero);
    RUN_TEST(test_sub_interval_no_ticks);
    RUN_TEST(test_single_tick);
    RUN_TEST(test_multiple_ticks_keeps_remainder);
    RUN_TEST(test_backwards_clock_resyncs);
    RUN_TEST(test_no_drift_across_calls);
    return UNITY_END();
}
