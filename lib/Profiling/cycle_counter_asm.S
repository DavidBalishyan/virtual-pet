/*
 * cycle_counter_asm.S - read the Xtensa CCOUNT register.
 *
 * uint32_t read_ccount(void);
 *
 * CCOUNT is the CPU cycle counter special register. C has no way to name it, so
 * reading it takes one instruction: `rsr` (read special register). This is the
 * point of the whole experiment - a real, standalone GNU assembly function that
 * does something C cannot.
 *
 * The ESP32 Arduino core is built with the Xtensa WINDOWED ABI, so this function
 * uses the windowed calling convention:
 *   - `entry` sets up the register window and a (minimum 16-byte) stack frame.
 *   - the return value goes in a2.
 *   - `retw.n` is the windowed return.
 * Getting the ABI wrong (missing `entry`, plain `ret`, bad frame size) crashes
 * or returns garbage - that is the main risk of the .S approach.
 *
 * The whole body is guarded by __XTENSA__ so that on the native/x86 host build
 * (the .S is preprocessed because of the capital ".S" extension) it produces an
 * empty object file instead of failing to assemble. The C++ fallback in
 * cycle_counter.cpp supplies read behaviour on that platform.
 */

#if defined(__XTENSA__)
    .text
    .align  4
    .global read_ccount
    .type   read_ccount, @function
read_ccount:
    entry   sp, 16          /* windowed-ABI prologue, minimum 16-byte frame */
    rsr.ccount a2           /* read CCOUNT into the return register a2       */
    retw.n                  /* windowed return                              */
    .size   read_ccount, . - read_ccount
#endif
