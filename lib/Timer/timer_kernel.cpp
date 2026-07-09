#include "timer_kernel.h"

#if defined(__XTENSA__) && defined(USE_ASM_TIMER)
// Hand-written Xtensa implementation in timer_kernel_asm.S. extern "C" keeps the
// symbol unmangled so the assembly and this declaration name the same function.
extern "C" int timer_decay_ticks_asm(time_t now, time_t* last, time_t interval);
// The assembly loads/stores *last with a single 32-bit access, so it is only
// correct while time_t is 32 bits wide - which it is on the Xtensa (ILP32) target.
static_assert(sizeof(time_t) == 4, "asm timer kernel assumes a 32-bit time_t");
#endif

namespace timing {

int timerDecayTicks(time_t now, time_t* last, time_t interval) {
#if defined(__XTENSA__) && defined(USE_ASM_TIMER)
    // Device path: the arithmetic core runs as hand-written assembly.
    return timer_decay_ticks_asm(now, last, interval);
#else
    // C++ fallback. This is the reference behaviour: the host build always uses
    // it, and the device uses it too when USE_ASM_TIMER is off (for A/B testing).
    if (*last == 0) {          // first call: anchor the timer to "now", no ticks yet
        *last = now;
        return 0;
    }
    time_t elapsed = now - *last;
    if (elapsed < 0) {         // clock ran backwards (e.g. RTC reset): resync
        *last = now;
        return 0;
    }
    if (elapsed >= interval) {
        int ticks = (int)(elapsed / interval);
        *last = now - (elapsed % interval);  // keep the leftover so ticks never drift
        return ticks;
    }
    return 0;                   // less than one interval has passed
#endif
}

}  // namespace timing
