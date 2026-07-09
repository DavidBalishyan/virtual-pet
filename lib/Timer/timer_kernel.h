#ifndef TIMER_KERNEL_H
#define TIMER_KERNEL_H

#include <time.h>

// ---------------------------------------------------------------------------
// timer_kernel: the arithmetic core shared by every decay rule in TimerManager.
//
// All six apply* helpers in time_manager.cpp repeat the same integer routine:
// work out how many whole `interval`-second ticks have elapsed since a timer
// last fired, advance the timer past those ticks, and hand back the count so
// the caller can apply that many points of decay. That is pure integer maths
// (subtract, compare, divide, modulo), which makes it the one part of the Timer
// worth writing in assembly.
//
// On the ESP32 (Xtensa) this dispatches to a hand-written assembly routine
// (timer_kernel_asm.S) when USE_ASM_TIMER is defined; otherwise, and always on
// the host, it uses the C++ fallback below. The two implementations are kept
// deliberately identical so the flag can be flipped to A/B compare them on the
// same device using the cycle counter (see lib/Profiling).
// ---------------------------------------------------------------------------

namespace timing {

// Returns how many whole `interval` ticks have elapsed since *last, and advances
// *last past them (to now minus the leftover remainder). Returns 0 without
// touching *last when less than one interval has passed. On the first call
// (*last == 0) or if the clock ran backwards (now < *last), it resynchronises
// *last to now and returns 0. `interval` must be positive.
int timerDecayTicks(time_t now, time_t* last, time_t interval);

}  // namespace timing

#endif  // TIMER_KERNEL_H
