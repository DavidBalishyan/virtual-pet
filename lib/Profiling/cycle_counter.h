#ifndef CYCLE_COUNTER_H
#define CYCLE_COUNTER_H

#include <cstdint>

// ---------------------------------------------------------------------------
// cycle_counter: read the CPU's cycle counter for cheap on-device profiling.
//
// On the ESP32 (Xtensa LX6) the CPU has a hardware register, CCOUNT, that ticks
// up once every clock cycle. There is no C keyword for it, so the only way to
// read it is a single assembly instruction (see cycle_counter_asm.S). That is
// the whole reason this module exists: it is a genuine "you need assembly for
// this" case, unlike arithmetic the compiler already handles well.
//
// CCOUNT is a plain 32-bit counter, so it wraps back to 0 roughly every 2^32
// cycles (~18 seconds at 240 MHz). cyclesSince() handles a single wrap for you
// with ordinary unsigned subtraction, so any interval shorter than one wrap
// (e.g. one render frame) measures correctly.
//
// On the host (unit tests, x86) there is no portable equivalent, so the build
// falls back to a monotonic stand-in. It counts calls, not real cycles: enough
// to keep the API usable and testable, but only the device reports true cycles.
// ---------------------------------------------------------------------------

namespace profiling {

// Current value of the CPU cycle counter. Ticks once per CPU clock; wraps at 2^32.
uint32_t readCycleCount();

// Cycles elapsed from start to end. Correct across a single 2^32 wrap because
// the subtraction is done in unsigned arithmetic.
uint32_t cyclesSince(uint32_t start, uint32_t end);

}  // namespace profiling

#endif  // CYCLE_COUNTER_H
