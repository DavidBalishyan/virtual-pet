#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <time.h>
#include "../Pet/pet.h"

// ---------------------------------------------------------------
// TimerManager
//
// This class is responsible for all time-based changes to the pet.
// Instead of scattering timer logic across main.cpp, every "something
// happens automatically over time" rule lives here.
//
// Uses the BM8563 RTC's epoch time (seconds) instead of millis(), so
// stat decay continues in real wall-clock time even when the device
// is off. On boot, any elapsed time while powered off is caught up
// by applying multiple decay ticks at once.
//
// How to use it:
//   1. Create one TimerManager in main.cpp (like the other managers).
//   2. Call timers.update(myPet, petClock.getEpochSeconds()) once per loop().
//   3. To add a new decay rule, add a method here and call it from update().
// ---------------------------------------------------------------
class TimerManager {
public:
    // Constructor: sets all timer timestamps to zero so they are
    // initialised to the current time on the first update() call.
    TimerManager();

    // update(): call this once every loop().
    // currentEpochSecs comes from ClockManager::getEpochSeconds().
    // It checks each timer and applies the matching stat change to the pet
    // if enough time has passed.
    void update(Pet& pet, time_t currentEpochSecs);

    // Getters: used by StorageManager to persist timer positions to NVS.
    time_t getLastFullnessDecayTime() const;
    time_t getLastHappinessDecayTime() const;
    time_t getLastEnergyDrainTime() const;
    time_t getLastCleanlinessDecayTime() const;
    time_t getLastSicknessAccumulationTime() const;
    time_t getLastHydrationDecayTime() const;

    // Setters: used by StorageManager to restore timer positions from NVS.
    void setLastFullnessDecayTime(time_t t);
    void setLastHappinessDecayTime(time_t t);
    void setLastEnergyDrainTime(time_t t);
    void setLastCleanlinessDecayTime(time_t t);
    void setLastSicknessAccumulationTime(time_t t);
    void setLastHydrationDecayTime(time_t t);

private:
    // -----------------------------------------------------------------
    // Each timer needs one variable that remembers the last time it fired.
    // The type 'time_t' holds Unix epoch seconds (int32_t on ESP32).
    // -----------------------------------------------------------------

    // Tracks when fullness was last decreased.
    time_t lastFullnessDecayTime;

    // Tracks when happiness was last decreased.
    time_t lastHappinessDecayTime;

    // Tracks when energy was last decreased.
    time_t lastEnergyDrainTime;

    // Tracks when cleanliness was last decreased.
    time_t lastCleanlinessDecayTime;

    // Tracks when sickness was last increased.
    time_t lastSicknessAccumulationTime;

    // Tracks when hydration was last decreased.
    time_t lastHydrationDecayTime;

    // -----------------------------------------------------------------
    // Private helper methods. One per stat that changes automatically.
    // Keeping each rule in its own function makes them easy to find,
    // read, and modify independently.
    // -----------------------------------------------------------------

    // Decreases fullness by a fixed amount every few seconds.
    // The pet gets hungrier whether you interact with it or not.
    void applyFullnessDecay(Pet& pet, time_t currentTime);

    // Decreases happiness by a fixed amount every few seconds.
    // The pet gets sad if you ignore it.
    void applyHappinessDecay(Pet& pet, time_t currentTime);

    // Decreases energy by a fixed amount every few seconds.
    // The pet gets tired over time whether you interact with it or not.
    void applyEnergyDrain(Pet& pet, time_t currentTime);

    // Decreases cleanliness by a fixed amount every few seconds.
    // The pet gets dirty over time and needs bathing.
    void applyCleanlinessDecay(Pet& pet, time_t currentTime);

    // Increases sickness when cleanliness is dangerously low.
    // A dirty pet gradually becomes unwell. Bathing prevents this.
    void applySicknessAccumulation(Pet& pet, time_t currentTime);

    // Decreases hydration by a fixed amount every few seconds.
    // The pet gets thirstier whether you interact with it or not.
    void applyHydrationDecay(Pet& pet, time_t currentTime);
};

#endif // TIME_MANAGER_H