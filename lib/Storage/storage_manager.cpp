#include <Arduino.h>
#include <Preferences.h>
#include "storage_manager.h"

// The NVS namespace — groups all keys for this project under one label.
// Using a namespace means these keys will never collide with keys written by
// other libraries that also use the Preferences / NVS system.
const char* StorageManager::NAMESPACE = "virtual-pet";

// save()
// Opens the NVS namespace in read/write mode, writes all seven stat values
// as integers, then closes the handle. Calling end() commits the data to flash.
void StorageManager::save(const Pet& pet) {
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

    prefs.end();

#ifdef DEBUG
    Serial.println("StorageManager: save complete.");
#endif
}

// load()
// Opens the NVS namespace in read-only mode and applies each saved stat to the
// pet via its setter. The second argument to getInt() is the default value used
// when no save data exists yet — these match the Pet constructor's starting values
// so a fresh device behaves identically to a newly constructed Pet object.
void StorageManager::load(Pet& pet) {
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

    prefs.end();

#ifdef DEBUG
    Serial.print("  fullness: ");      Serial.println(pet.getFullness());
    Serial.print("  tired: ");       Serial.println(pet.getTired());
    Serial.print("  happy: ");       Serial.println(pet.getHappy());
    Serial.print("  sick: ");        Serial.println(pet.getSick());
    Serial.print("  sad: ");         Serial.println(pet.getSad());
    Serial.print("  cleanliness: "); Serial.println(pet.getCleanliness());
    Serial.print("  energised: ");   Serial.println(pet.getEnergised());
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
