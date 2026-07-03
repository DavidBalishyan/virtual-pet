#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#include <time.h>

// Wraps the M5.Rtc API to provide the current time for display on the Stats
// screen. The M5StickC Plus 2 has a BM8563 RTC on its I2C bus, already
// initialised by M5.begin(). This class gives it a known starting time and
// exposes read-only getters.
class ClockManager {
private:
    static const int STARTING_HOUR   = 12;
    static const int STARTING_MINUTE = 0;
    static const int STARTING_SECOND = 0;
    static const int STARTING_YEAR   = 2026;
    static const int STARTING_MONTH  = 1;
    static const int STARTING_DAY    = 1;

public:
    // Sets the RTC to a known starting date and time (2024-01-01 00:00:00).
    // Must be called once in setup() after M5.begin().
    void begin();

    // Reads the current time from the RTC and writes the hours and minutes
    // into the given out-parameters. Values are in 24-hour format (0-23).
    void getCurrentTime(int& hours, int& minutes) const;

    // Reads the current RTC date and time and returns it as a Unix epoch
    // timestamp (seconds since 1970-01-01). Survives reboots because the
    // BM8563 has its own backup battery.
    time_t getEpochSeconds() const;
};

#endif // CLOCK_MANAGER_H
