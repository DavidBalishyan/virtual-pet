#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

// Wraps the M5.Rtc API to provide the current time for display on the Stats
// screen. The M5StickC Plus 2 has a BM8563 RTC on its I2C bus, already
// initialised by M5.begin(). This class gives it a known starting time and
// exposes a read-only getter.
class ClockManager {
private:
    static const int STARTING_HOUR   = 12;
    static const int STARTING_MINUTE = 0;

public:
    // Sets the RTC to a known starting time (12:00).
    // Must be called once in setup() after M5.begin().
    void begin();

    // Reads the current time from the RTC and writes the hours and minutes
    // into the given out-parameters. Values are in 24-hour format (0-23).
    void getCurrentTime(int& hours, int& minutes) const;
};

#endif // CLOCK_MANAGER_H
