#include <Arduino.h>
#include <Preferences.h>
#include "storage_manager.h"

// The NVS namespace. Groups all keys for this project under one label.
// Using a namespace means these keys will never collide with keys written by
// other libraries that also use the Preferences / NVS system.
const char* StorageManager::NAMESPACE = "virtual-pet";

// save()
// Opens the NVS namespace in read/write mode, writes all eight stat values
// plus the six timer timestamps as integers, then closes the handle.
// Calling end() commits the data to flash.
void StorageManager::save(const Pet& pet, const TimerManager& timers) {
#ifdef DEBUG
    Serial.println("StorageManager: saving pet stats to NVS...");
#endif

    Preferences prefs;
    prefs.begin(NAMESPACE, false);  // false = read/write mode

    prefs.putInt("fullness",      pet.getFullness());
    prefs.putInt("tired",       pet.getTired());
    prefs.putInt("happy",       pet.getHappy());
    prefs.putInt("sick",        pet.getSick());
    prefs.putInt("sad",         pet.getSad());
    prefs.putInt("cleanliness", pet.getCleanliness());
    prefs.putInt("energised",   pet.getEnergised());
    prefs.putInt("hydration",   pet.getHydration());
    prefs.putString("petName",  pet.getPetName());

    // Save timer timestamps so real-time decay can catch up after power-off.
    prefs.putInt("t_fullness",     timers.getLastFullnessDecayTime());
    prefs.putInt("t_happiness",    timers.getLastHappinessDecayTime());
    prefs.putInt("t_energy",       timers.getLastEnergyDrainTime());
    prefs.putInt("t_cleanliness",  timers.getLastCleanlinessDecayTime());
    prefs.putInt("t_sickness",     timers.getLastSicknessAccumulationTime());
    prefs.putInt("t_hydration",    timers.getLastHydrationDecayTime());

    prefs.end();

#ifdef DEBUG
    Serial.println("StorageManager: save complete.");
#endif
}

// load()
// Opens the NVS namespace in read-only mode and applies each saved stat to the
// pet via its setter. The second argument to getInt() is the default value used
// when no save data exists yet. These match the Pet constructor's starting values
// so a fresh device behaves identically to a newly constructed Pet object.
void StorageManager::load(Pet& pet, TimerManager& timers) {
#ifdef DEBUG
    Serial.println("StorageManager: loading pet stats from NVS...");
#endif

    Preferences prefs;
    prefs.begin(NAMESPACE, true);  // true = read-only mode

    pet.setFullness(     prefs.getInt("fullness",      Pet::DEFAULT_FULLNESS));
    pet.setTired(      prefs.getInt("tired",       Pet::DEFAULT_TIRED));
    pet.setHappy(      prefs.getInt("happy",       Pet::DEFAULT_HAPPY));
    pet.setSick(       prefs.getInt("sick",        Pet::DEFAULT_SICK));
    pet.setSad(        prefs.getInt("sad",         Pet::DEFAULT_SAD));
    pet.setCleanliness(prefs.getInt("cleanliness", Pet::DEFAULT_CLEANLINESS));
    pet.setEnergised(  prefs.getInt("energised",   Pet::DEFAULT_ENERGISED));
    pet.setHydration(  prefs.getInt("hydration",   Pet::DEFAULT_HYDRATION));

    // Read the pet name; if no key exists yet, fall back to the default.
    String savedName = prefs.getString("petName", "Pixel");
    pet.setPetName(savedName.c_str());

    // Load timer timestamps. Default 0 means "first boot", handled by TimerManager.
    timers.setLastFullnessDecayTime(           prefs.getInt("t_fullness",     0));
    timers.setLastHappinessDecayTime(          prefs.getInt("t_happiness",    0));
    timers.setLastEnergyDrainTime(             prefs.getInt("t_energy",       0));
    timers.setLastCleanlinessDecayTime(        prefs.getInt("t_cleanliness",  0));
    timers.setLastSicknessAccumulationTime(    prefs.getInt("t_sickness",     0));
    timers.setLastHydrationDecayTime(          prefs.getInt("t_hydration",    0));

    prefs.end();

#ifdef DEBUG
    Serial.print("  fullness: ");      Serial.println(pet.getFullness());
    Serial.print("  tired: ");       Serial.println(pet.getTired());
    Serial.print("  happy: ");       Serial.println(pet.getHappy());
    Serial.print("  sick: ");        Serial.println(pet.getSick());
    Serial.print("  sad: ");         Serial.println(pet.getSad());
    Serial.print("  cleanliness: "); Serial.println(pet.getCleanliness());
    Serial.print("  energised: ");   Serial.println(pet.getEnergised());
    Serial.print("  hydration: ");   Serial.println(pet.getHydration());
#endif
}

// clear()
// Opens the NVS namespace and erases all keys within it. After this call,
// the next load() will return the default starting values for every stat.
void StorageManager::clear() {
#ifdef DEBUG
    Serial.println("StorageManager: clearing all saved data from NVS.");
#endif
    Preferences prefs;
    prefs.begin(NAMESPACE, false);
    prefs.clear();
    prefs.end();
}
