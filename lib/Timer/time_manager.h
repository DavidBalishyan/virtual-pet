#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "../Pet/pet.h"

// ---------------------------------------------------------------
// TimerManager
//
// This class is responsible for all time-based changes to the pet.
// Instead of scattering timer logic across main.cpp, every "something
// happens automatically over time" rule lives here.
//
// How to use it:
//   1. Create one TimerManager in main.cpp (like the other managers).
//   2. Call timers.update(myPet) once per loop().
//   3. To add a new decay rule, add a method here and call it from update().
// ---------------------------------------------------------------
class TimerManager {
public:
    // Constructor — sets all timer timestamps to zero so they fire
    // immediately on the first loop().
    TimerManager();

    // update() — call this once every loop().
    // It checks each timer and applies the matching stat change to the pet
    // if enough time has passed.
    void update(Pet& pet);

private:
    // -----------------------------------------------------------------
    // Each timer needs one variable that remembers the last time it fired.
    // The type 'unsigned long' holds milliseconds without going negative.
    // -----------------------------------------------------------------

    // Tracks when fullness was last decreased.
    unsigned long lastFullnessDecayTime;

    // Tracks when happiness was last decreased.
    unsigned long lastHappinessDecayTime;

    // Tracks when energy was last decreased.
    unsigned long lastEnergyDrainTime;

    // Tracks when cleanliness was last decreased.
    unsigned long lastCleanlinessDecayTime;

    // Tracks when sickness was last increased.
    unsigned long lastSicknessAccumulationTime;

    // Tracks when hydration was last decreased.
    unsigned long lastHydrationDecayTime;

    // -----------------------------------------------------------------
    // Private helper methods — one per stat that changes automatically.
    // Keeping each rule in its own function makes them easy to find,
    // read, and modify independently.
    // -----------------------------------------------------------------

    // Decreases fullness by a fixed amount every few seconds.
    // The pet gets hungrier whether you interact with it or not.
    void applyFullnessDecay(Pet& pet);

    // Decreases happiness by a fixed amount every few seconds.
    // The pet gets sad if you ignore it.
    void applyHappinessDecay(Pet& pet);

    // Decreases energy by a fixed amount every few seconds.
    // The pet gets tired over time whether you interact with it or not.
    void applyEnergyDrain(Pet& pet);

    // Decreases cleanliness by a fixed amount every few seconds.
    // The pet gets dirty over time and needs bathing.
    void applyCleanlinessDecay(Pet& pet);

    // Increases sickness when cleanliness is dangerously low.
    // A dirty pet gradually becomes unwell — bathing prevents this.
    void applySicknessAccumulation(Pet& pet);

    // Decreases hydration by a fixed amount every few seconds.
    // The pet gets thirstier whether you interact with it or not.
    void applyHydrationDecay(Pet& pet);
};

#endif // TIME_MANAGER_H
