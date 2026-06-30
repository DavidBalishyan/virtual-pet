# Unit Tests

Run with:

```
pio test -e native
```

## What and why

Three test suites cover the hardware-independent modules:

| Suite | Tests | What it covers |
|---|---|---|
| test_tilt_motion | 8 | Smoothing, clamping, self-recentre |
| test_animation_manager | 8 | Frame cycling, wrap-around, reset |
| test_pet | 35 | Stats, actions, state machine, moods, death, reset |

Each suite compiles the **real source files** from `lib/` (not mocks), so
you're testing the actual implementation that runs on the device.

## Why test/stubs/Arduino.h exists

The source files in `lib/` include `<Arduino.h>` for things like `millis()`,
`constrain()`, and `round()`. The real `<Arduino.h>` only compiles for the
ESP32 CPU. It has hardware register definitions, HAL headers, and inline
assembly that an x86 PC cannot build.

`test/stubs/Arduino.h` replaces it when running tests on your PC. It
provides only the few things the logic actually needs:

- **`millis()`** driven by a controllable counter (`setMillis()`) so timing
  is deterministic in tests
- **`constrain()`** the same macro the real Arduino.h defines
- **`<cmath>`** for `round()` and other math
- **`Serial`** a no-op so debug prints compile but produce nothing

The stub gets picked up because `-I test/stubs` is in the native build flags,
so `#include <Arduino.h>` finds `test/stubs/Arduino.h` and never looks for
the real (missing) one.

## Adding a new test

1. Create `test/test_your_module/test_your_module.cpp`.
2. Include the header and its `.cpp` directly:
   ```cpp
   #include "../../lib/YourModule/your_module.h"
   ```
3. If your module calls M5 hardware functions, put a stub class declaration
   in `test/stubs/`.
4. Run with `pio test -e native`.
