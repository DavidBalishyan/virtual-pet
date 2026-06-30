#ifndef ARDUINO_H
#define ARDUINO_H

#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstring>

using boolean = bool;
using byte = uint8_t;

// Controllable millis() — shared state across all TUs via C++17 inline variable.
inline unsigned long _test_millis = 0;
inline unsigned long millis() { return _test_millis; }
inline void setMillis(unsigned long v) { _test_millis = v; }

// Arduino-compatible constrain — macro (like the real Arduino one), so it
// works with mixed types (float + int) without template deduction failures.
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// GPIO constants
const int INPUT = 0;
const int LOW = 0;

inline void delay(unsigned long ms) {}
inline void pinMode(int pin, int mode) {}
inline int digitalRead(int pin) { return 0; }

// Minimal Serial stub — enough to satisfy DEBUG code that prints.
class SerialStub {
public:
    void begin(unsigned long) {}
    void print(const char*) {}
    void println(const char*) {}
    void printf(const char*, ...) {}
    void print(int) {}
    void println(int) {}
};
inline SerialStub Serial;

#endif // ARDUINO_H
