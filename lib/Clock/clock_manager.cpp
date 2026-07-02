#include <Arduino.h>
#include "M5StickCPlus2.h"
#include "clock_manager.h"

// Sets the RTC to a known starting time so the clock always shows something
// sensible. If the RTC was already keeping time from a previous session, this
// overwrites it - the foundation scope uses a hardcoded time. The stretch goal
// would detect a valid RTC and skip the set, or let the user set the time.
void ClockManager::begin() {
#ifdef DEBUG
    Serial.println("ClockManager: setting RTC to 12:00:00");
#endif
    M5.Rtc.setTime(m5::rtc_time_t(STARTING_HOUR, STARTING_MINUTE, 0));
}

// Reads hours and minutes from the BM8563 RTC via the M5.Rtc API.
// Writes them into the caller's int references so DisplayManager can use them
// directly without knowing what an m5::rtc_time_t is.
void ClockManager::getCurrentTime(int& hours, int& minutes) const {
    m5::rtc_time_t rtcTime = M5.Rtc.getTime();

    hours   = rtcTime.hours;
    minutes = rtcTime.minutes;

    // Clamp to valid range in case the RTC returns uninitialised data.
    if (hours   < 0 || hours   > 23) { hours   = 12; }
    if (minutes < 0 || minutes > 59) { minutes =  0; }
}
