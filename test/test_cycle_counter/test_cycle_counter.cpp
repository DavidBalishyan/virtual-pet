#include <unity.h>
#include <Arduino.h>
#include "../../lib/Profiling/cycle_counter.h"

// These tests run on the native/x86 host, where __XTENSA__ is NOT defined, so
// they exercise the portable fallback in cycle_counter.cpp and the wrap-safe
// arithmetic in cyclesSince(). They cannot assert real CPU-cycle values (there
// is no CCOUNT on the host); the true cycle counts only appear on the device.

void setUp(void) {}

void tearDown(void) {}

// The fallback counter must strictly increase on every call.
void test_read_is_monotonic(void) {
    uint32_t a = profiling::readCycleCount();
    uint32_t b = profiling::readCycleCount();
    uint32_t c = profiling::readCycleCount();
    TEST_ASSERT_TRUE(b > a);
    TEST_ASSERT_TRUE(c > b);
}

void test_cyclesSince_basic(void) {
    TEST_ASSERT_EQUAL_UINT32(150u, profiling::cyclesSince(100u, 250u));
}

// The load-bearing assertion: unsigned subtraction stays correct across a single
// 2^32 wrap. This holds on any platform, so it is a legitimate host test.
void test_cyclesSince_wraps(void) {
    TEST_ASSERT_EQUAL_UINT32(0x1Fu, profiling::cyclesSince(0xFFFFFFF0u, 0x0000000Fu));
}

void test_cyclesSince_zero(void) {
    TEST_ASSERT_EQUAL_UINT32(0u, profiling::cyclesSince(12345u, 12345u));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_read_is_monotonic);
    RUN_TEST(test_cyclesSince_basic);
    RUN_TEST(test_cyclesSince_wraps);
    RUN_TEST(test_cyclesSince_zero);
    return UNITY_END();
}
