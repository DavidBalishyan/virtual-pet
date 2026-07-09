#include "cycle_counter.h"

#if defined(__XTENSA__)
// Defined in cycle_counter_asm.S. extern "C" keeps the name unmangled so the
// assembly symbol and this declaration refer to the same thing.
extern "C" uint32_t read_ccount(void);
#endif

namespace profiling {

uint32_t readCycleCount() {
#if defined(__XTENSA__)
    // Real hardware cycle counter, read by one `rsr` instruction in assembly.
    return read_ccount();
#else
    // Host / native build: no portable hardware cycle counter is reachable, and
    // the test Arduino stub has no micros(). A monotonic counter keeps the API
    // usable and unit-testable; it counts calls, not real CPU cycles.
    static uint32_t fake = 0;
    return ++fake;
#endif
}

uint32_t cyclesSince(uint32_t start, uint32_t end) {
    // Unsigned subtraction is wrap-safe: if CCOUNT wrapped once between the two
    // reads, (end - start) still yields the correct elapsed count as long as the
    // interval is shorter than one full 2^32 wrap.
    return end - start;
}

}  // namespace profiling
