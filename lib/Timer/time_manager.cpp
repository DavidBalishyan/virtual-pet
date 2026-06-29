#include <Arduino.h>
#include "time_manager.h"

// Uncomment the next line to build the QUICK-TEST version, where the pet's stats
// decay fast enough to reach a fatal level in about a minute. Leave it commented
// for the real, balanced behaviour. (See the two sets of values below.)
#define FAST_TEST

// ---------------------------------------------------------------------------
// STAT BALANCE — two complete sets of decay rates.
//
// The pet has three fatal stats: fullness, energy and happiness (all die at 0).
// These constants decide how fast each one moves, and therefore how
// long the pet survives if you ignore it. Getting them right is "balancing" how
// the pet behaves — too fast and the pet is impossible to keep alive, too slow
// and nothing you do seems to matter.
//
// We keep TWO sets and pick one with the FAST_TEST switch at the top of this file:
//
//   * The SHIPPED set (the #else branch) is the real, balanced behaviour. An
//     untouched pet starves in about 8 minutes — fullness is tuned to empty first,
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
// by changing these intervals. This is a compile-time choice — only ONE set ends
// up in the firmware — not a setting that can change while the program runs.
//
// In BOTH sets every stat moves by 1 point per interval, so each interval reads
// directly as "lose 1 point every N milliseconds" and the time to cross the full
// 0–100 range is simply (100 × interval).
// ---------------------------------------------------------------------------
#ifdef FAST_TEST

// --- QUICK-TEST set: the pet starves in under a minute (fullness empties first). ---
const unsigned long FULLNESS_DECAY_INTERVAL = 600;         // fullness -1 every 0.6 s (empties first)
const int FULLNESS_DECAY_AMOUNT = 1;
const unsigned long HAPPINESS_DECAY_INTERVAL = 1000;        // happiness -1 every 1 s
const int HAPPINESS_DECAY_AMOUNT = 1;
const unsigned long ENERGY_DRAIN_INTERVAL = 1000;           // energy -1 every 1 s
const int ENERGY_DRAIN_AMOUNT = 1;
const unsigned long CLEANLINESS_DECAY_INTERVAL = 1500;      // cleanliness -1 every 1.5 s (secondary)
const int CLEANLINESS_DECAY_AMOUNT = 1;
const unsigned long SICKNESS_ACCUMULATION_INTERVAL = 1500;  // sickness +1 every 1.5 s (secondary)
const int SICKNESS_ACCUMULATION_AMOUNT = 1;

#else

// --- SHIPPED set: the pet starves in about 8 minutes (fullness empties first). ---
const unsigned long FULLNESS_DECAY_INTERVAL = 6000;        // fullness -1 every 6 s (empties first)
const int FULLNESS_DECAY_AMOUNT = 1;
const unsigned long HAPPINESS_DECAY_INTERVAL = 9000;        // happiness -1 every 9 s
const int HAPPINESS_DECAY_AMOUNT = 1;
const unsigned long ENERGY_DRAIN_INTERVAL = 8000;           // energy -1 every 8 s
const int ENERGY_DRAIN_AMOUNT = 1;
// Cleanliness and sickness are secondary: neither is directly fatal. Cleanliness
// only matters once it drops below CLEANLINESS_DANGER_THRESHOLD, which then lets
// sickness build up. They are tuned slightly slower than the three fatal stats.
const unsigned long CLEANLINESS_DECAY_INTERVAL = 10000;     // cleanliness -1 every 10 s
const int CLEANLINESS_DECAY_AMOUNT = 1;
const unsigned long SICKNESS_ACCUMULATION_INTERVAL = 10000; // sickness +1 every 10 s
const int SICKNESS_ACCUMULATION_AMOUNT = 1;

#endif

// Cleanliness must fall below this value before sickness starts accumulating.
const int CLEANLINESS_DANGER_THRESHOLD = 30;


// Constructor — initialise all timestamps to 0.
// Setting them to 0 means the first check in update() will always find
// that "enough time has passed", so the first decay fires immediately.
TimerManager::TimerManager()
    : lastFullnessDecayTime(0),
      lastHappinessDecayTime(0),
      lastEnergyDrainTime(0),
      lastCleanlinessDecayTime(0),
      lastSicknessAccumulationTime(0) {
}


// update() — the single function main.cpp calls every loop().
// It delegates each timed job to its own private helper method.
// To add a new automatic stat change, add a method and call it here.
void TimerManager::update(Pet& pet) {
    applyFullnessDecay(pet);
    applyHappinessDecay(pet);
    applyEnergyDrain(pet);
    applyCleanlinessDecay(pet);
    applySicknessAccumulation(pet);
}


// applyFullnessDecay()
// Checks whether FULLNESS_DECAY_INTERVAL milliseconds have passed since
// fullness was last decreased. If yes, decreases fullness and resets the timer.
// The pet gets hungrier (less full) over time whether you feed it or not.
void TimerManager::applyFullnessDecay(Pet& pet) {
    unsigned long currentTime = millis();

    if (currentTime - lastFullnessDecayTime > FULLNESS_DECAY_INTERVAL) {
        pet.setFullness(pet.getFullness() - FULLNESS_DECAY_AMOUNT);
        lastFullnessDecayTime = currentTime;
    }
}


// applyHappinessDecay()
// Checks whether HAPPINESS_DECAY_INTERVAL milliseconds have passed since
// happiness was last decreased. If yes, decreases happiness and resets the timer.
void TimerManager::applyHappinessDecay(Pet& pet) {
    unsigned long currentTime = millis();

    if (currentTime - lastHappinessDecayTime > HAPPINESS_DECAY_INTERVAL) {
        pet.setHappy(pet.getHappy() - HAPPINESS_DECAY_AMOUNT);
        lastHappinessDecayTime = currentTime;
    }
}


// applyEnergyDrain()
// Checks whether ENERGY_DRAIN_INTERVAL milliseconds have passed since
// energy was last decreased. If yes, decreases energy and resets the timer.
void TimerManager::applyEnergyDrain(Pet& pet) {
    unsigned long currentTime = millis();

    if (currentTime - lastEnergyDrainTime > ENERGY_DRAIN_INTERVAL) {
        pet.setEnergised(pet.getEnergised() - ENERGY_DRAIN_AMOUNT);
        lastEnergyDrainTime = currentTime;
    }
}


// applyCleanlinessDecay()
// Checks whether CLEANLINESS_DECAY_INTERVAL milliseconds have passed since
// cleanliness was last decreased. If yes, decreases cleanliness and resets the timer.
// The pet gets dirty over time — bathing is the only way to keep it clean.
void TimerManager::applyCleanlinessDecay(Pet& pet) {
    unsigned long currentTime = millis();

    if (currentTime - lastCleanlinessDecayTime > CLEANLINESS_DECAY_INTERVAL) {
        pet.setCleanliness(pet.getCleanliness() - CLEANLINESS_DECAY_AMOUNT);
        lastCleanlinessDecayTime = currentTime;
    }
}


// applySicknessAccumulation()
// Increases sick only when cleanliness has fallen below CLEANLINESS_DANGER_THRESHOLD.
// A dirty pet gradually becomes unwell — the user must bathe it to stop this.
void TimerManager::applySicknessAccumulation(Pet& pet) {
    unsigned long currentTime = millis();

    if (pet.getCleanliness() < CLEANLINESS_DANGER_THRESHOLD) {
        if (currentTime - lastSicknessAccumulationTime > SICKNESS_ACCUMULATION_INTERVAL) {
            pet.setSick(pet.getSick() + SICKNESS_ACCUMULATION_AMOUNT);
            lastSicknessAccumulationTime = currentTime;
        }
    }
}
