/*
 * timer_kernel_asm.S - the TimerManager decay-tick arithmetic, in Xtensa assembly.
 *
 *   int timer_decay_ticks_asm(time_t now, time_t* last, time_t interval);
 *
 * This is the C++ timing::timerDecayTicks() logic (see timer_kernel.cpp) rewritten
 * by hand. It is a leaf function using the Xtensa WINDOWED ABI:
 *   - `entry` sets up the register window / stack frame.
 *   - incoming args land in a2 (now), a3 (last ptr), a4 (interval).
 *   - the result (tick count) is returned in a2.
 *   - `retw.n` is the windowed return.
 * It uses the ESP32's hardware integer divide (quos/rems); the C++ fallback in
 * timer_kernel.cpp covers any target without those instructions.
 *
 * The whole body is guarded so that on the host (native/x86) build - where the .S
 * is preprocessed because of the capital ".S" extension - it produces an empty
 * object file, and the C++ fallback is used instead. On device the same fallback
 * is used whenever USE_ASM_TIMER is left undefined.
 */

#if defined(__XTENSA__) && defined(USE_ASM_TIMER)
    .text
    .align  4
    .global timer_decay_ticks_asm
    .type   timer_decay_ticks_asm, @function
timer_decay_ticks_asm:
    entry   sp, 16
    l32i    a5, a3, 0           /* a5 = *last                                  */
    beqz    a5, .Lresync        /* first call (*last == 0) -> resync, return 0 */
    sub     a6, a2, a5          /* a6 = elapsed = now - *last                  */
    bltz    a6, .Lresync        /* clock went backwards -> resync, return 0    */
    blt     a6, a4, .Lnone      /* elapsed < interval -> return 0, *last kept  */
    quos    a7, a6, a4          /* a7 = ticks     = elapsed / interval         */
    rems    a8, a6, a4          /* a8 = remainder = elapsed % interval         */
    sub     a9, a2, a8          /* a9 = now - remainder                        */
    s32i    a9, a3, 0           /* *last = now - remainder                     */
    mov     a2, a7              /* return ticks                                */
    retw.n

.Lresync:
    s32i    a2, a3, 0           /* *last = now                                 */
.Lnone:
    movi    a2, 0               /* return 0                                    */
    retw.n
    .size   timer_decay_ticks_asm, . - timer_decay_ticks_asm
#endif
