#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "../Pet/pet.h"
#include "../Timer/time_manager.h"

// StorageManager
// Handles saving and loading pet stats to NVS (Non-Volatile Storage) using the
// Arduino Preferences library. Stats written here survive a power-off or reset.
// Call load() once in setup() and save() whenever the user selects the Save action.
// Also persists TimerManager's RTC timestamps so real-time decay can catch up
// after the device has been powered off.
class StorageManager {
private:
    // The NVS namespace groups all virtual-pet keys together so they do not
    // clash with keys written by other libraries that share the same flash partition.
    // Limited to 15 characters by the ESP32 NVS implementation.
    static const char* NAMESPACE;

public:
    // Writes all eight pet stats and six timer timestamps to NVS so they
    // survive a power-off and the real-time clock catch-up works correctly.
    // Call this when the user confirms the Save action.
    void save(const Pet& pet, const TimerManager& timers);

    // Reads all eight pet stats and six timer timestamps from NVS and applies
    // them to the pet and timer manager. If no save data exists yet, each stat
    // falls back to the pet's default starting value and timestamps default to 0
    // (which the TimerManager initialises on first update).
    // Call this once in setup() before the main loop begins.
    void load(Pet& pet, TimerManager& timers);

    // Erases all saved stats and timestamps from NVS so the next load() returns
    // default values. Call this when the pet is reset after death so a fresh
    // game starts on the next boot.
    void clear();
};

#endif // STORAGE_MANAGER_H
