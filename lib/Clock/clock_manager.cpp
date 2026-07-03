#include <Arduino.h>
#include "M5StickCPlus2.h"
#include "clock_manager.h"

// Sets the RTC to a known starting date and time so the clock always shows
// something sensible and epoch calculations have a consistent reference.
// If the RTC was already keeping time from a previous session, this
// overwrites it - the foundation scope uses a hardcoded time. The stretch goal
// would detect a valid RTC and skip the set, or let the user set the time.
void ClockManager::begin() {
#ifdef DEBUG
    Serial.println("ClockManager: setting RTC to 2024-01-01 00:00:00");
#endif
    M5.Rtc.setTime(m5::rtc_time_t(STARTING_HOUR, STARTING_MINUTE, STARTING_SECOND));
    M5.Rtc.setDate(m5::rtc_date_t(STARTING_YEAR, STARTING_MONTH, STARTING_DAY, 1));
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

// Reads the current RTC date and time, converts to a Unix epoch timestamp.
// The BM8563 maintains its own time while the device is off (backup battery),
// so this value advances in real wall-clock time across reboots.
time_t ClockManager::getEpochSeconds() const {
    m5::rtc_time_t rtcTime = M5.Rtc.getTime();
    m5::rtc_date_t rtcDate = M5.Rtc.getDate();

    struct tm tm = {};
    tm.tm_year = rtcDate.year - 1900;
    tm.tm_mon  = rtcDate.month - 1;
    tm.tm_mday = rtcDate.date;
    tm.tm_hour = rtcTime.hours;
    tm.tm_min  = rtcTime.minutes;
    tm.tm_sec  = rtcTime.seconds;

    return mktime(&tm);
}
