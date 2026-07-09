#include <Arduino.h>
#include "time_manager.h"
#include "timer_kernel.h"  // decay-tick arithmetic (assembly on device, C++ fallback)

// Uncomment the next line to build the QUICK-TEST version, where the pet's stats
// decay fast enough to reach a fatal level in about a minute. Leave it commented
// for the real, balanced behaviour. (See the two sets of values below.)
#define FAST_TEST

// ---------------------------------------------------------------------------
// STAT BALANCE. Two complete sets of decay rates.
//
// The pet has four fatal stats: fullness, energy, happiness and hydration (all die at 0).
// These constants decide how fast each one moves, and therefore how
// long the pet survives if you ignore it. Getting them right is "balancing" how
// the pet behaves. Too fast and the pet is impossible to keep alive, too slow
// and nothing you do seems to matter.
//
// We keep TWO sets and pick one with the FAST_TEST switch at the top of this file:
//
//   * The SHIPPED set (the #else branch) is the real, balanced behaviour. An
//     untouched pet starves in about 8 minutes. Fullness is tuned to empty first,
//     before happiness or energy, so the on-screen bar is what drives the death.
//     Long enough that caring for it feels meaningful, short enough to demo.
//
//   * The QUICK-TEST set (the #ifdef FAST_TEST branch) speeds everything up so
//     the pet reaches a fatal stat in about a minute. Uncomment the
//     "#define FAST_TEST" line above when you are developing and don't want to
//     wait ten minutes to watch the pet die; comment it out again before shipping.
//
// Both sets are kept on purpose: comparing the two numbers side by side is the
// clearest way to see how the same program can behave completely differently just
// by changing these intervals. This is a compile-time choice: only ONE set ends
// up in the firmware, not a setting that can change while the program runs.
//
// In BOTH sets every stat moves by 1 point per interval, so each interval reads
// directly as "lose 1 point every N milliseconds" and the time to cross the full
// 0-100 range is simply (100 × interval).
//
// NOTE: Intervals are now in SECONDS (not milliseconds) because the RTC epoch
// clock has second granularity. The numeric values are the original ms values
// divided by 1000.
// ---------------------------------------------------------------------------
#ifdef FAST_TEST

// --- QUICK-TEST set: the pet starves or dehydrates in under a minute (fullness empties first). ---
const time_t FULLNESS_DECAY_INTERVAL = 1;            // fullness -1 every 1 s (empties first)
const int FULLNESS_DECAY_AMOUNT = 1;
const time_t HAPPINESS_DECAY_INTERVAL = 1;            // happiness -1 every 1 s
const int HAPPINESS_DECAY_AMOUNT = 1;
const time_t ENERGY_DRAIN_INTERVAL = 1;               // energy -1 every 1 s
const int ENERGY_DRAIN_AMOUNT = 1;
const time_t CLEANLINESS_DECAY_INTERVAL = 2;          // cleanliness -1 every 2 s (secondary)
const int CLEANLINESS_DECAY_AMOUNT = 1;
const time_t SICKNESS_ACCUMULATION_INTERVAL = 2;      // sickness +1 every 2 s (secondary)
const int SICKNESS_ACCUMULATION_AMOUNT = 1;
const time_t HYDRATION_DECAY_INTERVAL = 1;            // hydration -1 every 1 s (third fatal)
const int HYDRATION_DECAY_AMOUNT = 1;

#else

// --- SHIPPED set: the pet starves or dehydrates in about 8 minutes (fullness empties first). ---
const time_t FULLNESS_DECAY_INTERVAL = 6;             // fullness -1 every 6 s (empties first)
const int FULLNESS_DECAY_AMOUNT = 1;
const time_t HAPPINESS_DECAY_INTERVAL = 9;            // happiness -1 every 9 s
const int HAPPINESS_DECAY_AMOUNT = 1;
const time_t ENERGY_DRAIN_INTERVAL = 8;               // energy -1 every 8 s
const int ENERGY_DRAIN_AMOUNT = 1;
// Cleanliness and sickness are secondary: neither is directly fatal. Cleanliness
// only matters once it drops below CLEANLINESS_DANGER_THRESHOLD, which then lets
// sickness build up. They are tuned slightly slower than the three fatal stats.
const time_t CLEANLINESS_DECAY_INTERVAL = 10;         // cleanliness -1 every 10 s
const int CLEANLINESS_DECAY_AMOUNT = 1;
const time_t SICKNESS_ACCUMULATION_INTERVAL = 10;     // sickness +1 every 10 s
const int SICKNESS_ACCUMULATION_AMOUNT = 1;
// Hydration is as fatal as fullness: it kills when it reaches 0.
// Tuned to empty about 1.5× slower than fullness so the drink action
// is important but not quite as urgent as feeding.
const time_t HYDRATION_DECAY_INTERVAL = 9;            // hydration -1 every 9 s
const int HYDRATION_DECAY_AMOUNT = 1;

#endif

// Cleanliness must fall below this value before sickness starts accumulating.
const int CLEANLINESS_DANGER_THRESHOLD = 30;


// Constructor: initialise all timestamps to 0.
// Setting them to 0 signals "not yet initialised" to the apply* helpers,
// which set them to the current time on the first update() call.
TimerManager::TimerManager()
    : lastFullnessDecayTime(0),
      lastHappinessDecayTime(0),
      lastEnergyDrainTime(0),
      lastCleanlinessDecayTime(0),
      lastSicknessAccumulationTime(0),
      lastHydrationDecayTime(0) {
}


// update(): the single function main.cpp calls every loop().
// It delegates each timed job to its own private helper method.
// To add a new automatic stat change, add a method and call it here.
void TimerManager::update(Pet& pet, time_t currentEpochSecs) {
    applyFullnessDecay(pet, currentEpochSecs);
    applyHappinessDecay(pet, currentEpochSecs);
    applyEnergyDrain(pet, currentEpochSecs);
    applyCleanlinessDecay(pet, currentEpochSecs);
    applySicknessAccumulation(pet, currentEpochSecs);
    applyHydrationDecay(pet, currentEpochSecs);
}


// applyFullnessDecay()
// Checks whether FULLNESS_DECAY_INTERVAL seconds have passed since
// fullness was last decreased. If yes, decreases fullness and resets the timer.
// The pet gets hungrier (less full) over time whether you feed it or not.
void TimerManager::applyFullnessDecay(Pet& pet, time_t currentTime) {
    int ticks = timing::timerDecayTicks(currentTime, &lastFullnessDecayTime, FULLNESS_DECAY_INTERVAL);
    if (ticks > 0) {
        pet.setFullness(pet.getFullness() - FULLNESS_DECAY_AMOUNT * ticks);
    }
}


// applyHappinessDecay()
// Checks whether HAPPINESS_DECAY_INTERVAL seconds have passed since
// happiness was last decreased. If yes, decreases happiness and resets the timer.
void TimerManager::applyHappinessDecay(Pet& pet, time_t currentTime) {
    int ticks = timing::timerDecayTicks(currentTime, &lastHappinessDecayTime, HAPPINESS_DECAY_INTERVAL);
    if (ticks > 0) {
        pet.setHappy(pet.getHappy() - HAPPINESS_DECAY_AMOUNT * ticks);
    }
}


// applyEnergyDrain()
// Checks whether ENERGY_DRAIN_INTERVAL seconds have passed since
// energy was last decreased. If yes, decreases energy and resets the timer.
void TimerManager::applyEnergyDrain(Pet& pet, time_t currentTime) {
    int ticks = timing::timerDecayTicks(currentTime, &lastEnergyDrainTime, ENERGY_DRAIN_INTERVAL);
    if (ticks > 0) {
        pet.setEnergised(pet.getEnergised() - ENERGY_DRAIN_AMOUNT * ticks);
    }
}


// applyCleanlinessDecay()
// Checks whether CLEANLINESS_DECAY_INTERVAL seconds have passed since
// cleanliness was last decreased. If yes, decreases cleanliness and resets the timer.
// The pet gets dirty over time. Bathing is the only way to keep it clean.
void TimerManager::applyCleanlinessDecay(Pet& pet, time_t currentTime) {
    int ticks = timing::timerDecayTicks(currentTime, &lastCleanlinessDecayTime, CLEANLINESS_DECAY_INTERVAL);
    if (ticks > 0) {
        pet.setCleanliness(pet.getCleanliness() - CLEANLINESS_DECAY_AMOUNT * ticks);
    }
}


// applySicknessAccumulation()
// Increases sick only when cleanliness has fallen below CLEANLINESS_DANGER_THRESHOLD.
// A dirty pet gradually becomes unwell. The user must bathe it to stop this.
void TimerManager::applySicknessAccumulation(Pet& pet, time_t currentTime) {
    // First-run anchoring happens unconditionally (even while clean), matching the
    // original, so the timer is never left at 0 waiting for the pet to get dirty.
    if (lastSicknessAccumulationTime == 0) {
        lastSicknessAccumulationTime = currentTime;
        return;
    }
    // Sickness only builds up while the pet is dirty. When it is clean the kernel
    // is not called, so the timer stays frozen exactly as before.
    if (pet.getCleanliness() < CLEANLINESS_DANGER_THRESHOLD) {
        int ticks = timing::timerDecayTicks(currentTime, &lastSicknessAccumulationTime,
                                            SICKNESS_ACCUMULATION_INTERVAL);
        if (ticks > 0) {
            pet.setSick(pet.getSick() + SICKNESS_ACCUMULATION_AMOUNT * ticks);
        }
    }
}


// applyHydrationDecay()
// Checks whether HYDRATION_DECAY_INTERVAL seconds have passed since
// hydration was last decreased. If yes, decreases hydration and resets the timer.
// The pet gets thirstier over time whether you give it water or not.
void TimerManager::applyHydrationDecay(Pet& pet, time_t currentTime) {
    int ticks = timing::timerDecayTicks(currentTime, &lastHydrationDecayTime, HYDRATION_DECAY_INTERVAL);
    if (ticks > 0) {
        pet.setHydration(pet.getHydration() - HYDRATION_DECAY_AMOUNT * ticks);
    }
}


// ---------------------------------------------------------------------------
// Getters: return each timer's last-fire timestamp.
// ---------------------------------------------------------------------------
time_t TimerManager::getLastFullnessDecayTime() const { return lastFullnessDecayTime; }
time_t TimerManager::getLastHappinessDecayTime() const { return lastHappinessDecayTime; }
time_t TimerManager::getLastEnergyDrainTime() const { return lastEnergyDrainTime; }
time_t TimerManager::getLastCleanlinessDecayTime() const { return lastCleanlinessDecayTime; }
time_t TimerManager::getLastSicknessAccumulationTime() const { return lastSicknessAccumulationTime; }
time_t TimerManager::getLastHydrationDecayTime() const { return lastHydrationDecayTime; }

// ---------------------------------------------------------------------------
// Setters: restore each timer's last-fire timestamp from saved data.
// ---------------------------------------------------------------------------
void TimerManager::setLastFullnessDecayTime(time_t t) { lastFullnessDecayTime = t; }
void TimerManager::setLastHappinessDecayTime(time_t t) { lastHappinessDecayTime = t; }
void TimerManager::setLastEnergyDrainTime(time_t t) { lastEnergyDrainTime = t; }
void TimerManager::setLastCleanlinessDecayTime(time_t t) { lastCleanlinessDecayTime = t; }
void TimerManager::setLastSicknessAccumulationTime(time_t t) { lastSicknessAccumulationTime = t; }
void TimerManager::setLastHydrationDecayTime(time_t t) { lastHydrationDecayTime = t; }