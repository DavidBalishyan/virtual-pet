#include <Arduino.h>
#include "pet.h"

// Pet()
// Constructs a brand-new pet in a healthy, awake-and-content starting state
// so the device looks alive the very first time it boots — the user should
// not have to do anything before the pet appears on screen with sensible
// stats. The starting values come from the DEFAULT_* constants in pet.h so
// the constructor, StorageManager::load(), and reset() can never drift out
// of agreement about what "a new pet" means.
//
// The colon-introduced list (`: fullness(...), tired(...), ...`) is a member
// initialiser list — it sets each field before the constructor body runs.
// It is the canonical C++ way to give member variables their initial values.
Pet::Pet()
    : fullness(DEFAULT_FULLNESS), tired(DEFAULT_TIRED), happy(DEFAULT_HAPPY),
      sick(DEFAULT_SICK), sad(DEFAULT_SAD), cleanliness(DEFAULT_CLEANLINESS),
      energised(DEFAULT_ENERGISED), currentState(STATE_IDLE),
      lastFullnessAlertTime(0), lastSicknessAlertTime(0) {
    petName = "Donald Trump";
}

// getPetName()
// Returns the pet's display name so the title zone can show it on screen.
const char* Pet::getPetName() const {
    return petName;
}

// Getters
// One read-only accessor per stat. The stat variables themselves are private,
// so this is the only way for other modules (DisplayManager, StorageManager,
// TimerManager) to see them. Keeping the variables private prevents an outside
// module from writing to them directly and bypassing the constrainValue()
// guarantee that all stats stay in 0..100. Anything that wants to *change* a
// stat must go through the matching setter below.
//
// The `const` at the end of each declaration is C++'s way of promising that
// the method does not modify the object — useful both as documentation and
// because the compiler enforces it.
int Pet::getFullness() const      { return fullness; }
int Pet::getTired() const       { return tired; }
int Pet::getHappy() const       { return happy; }
int Pet::getSick() const        { return sick; }
int Pet::getSad() const         { return sad; }
int Pet::getCleanliness() const { return cleanliness; }
int Pet::getEnergised() const   { return energised; }

// Setters
// Each setter stores the new value after passing it through constrainValue()
// so callers never have to constrain their own input — pet.setHappy(150) silently
// becomes 100, and pet.setHappy(-5) silently becomes 0. Constraining inside
// the setter (and not in a separate "constrainAll" sweep) means each setter
// only touches the one stat it owns.
void Pet::setFullness(int value)      { fullness      = constrainValue(value); }
void Pet::setTired(int value)       { tired       = constrainValue(value); }
void Pet::setHappy(int value)       { happy       = constrainValue(value); }
void Pet::setSick(int value)        { sick        = constrainValue(value); }
void Pet::setSad(int value)         { sad         = constrainValue(value); }
void Pet::setCleanliness(int value) { cleanliness = constrainValue(value); }
void Pet::setEnergised(int value)   { energised   = constrainValue(value); }

// Pet care actions
// Each action changes the pet's state, then adjusts several stats by routing
// every change through the matching setter. Going through the setters — even
// from inside the class — means the setters remain the only place where stat
// values are written. If we ever change how setters work (extra constraints,
// logging, change events), the action methods inherit those changes for free
// without needing to be edited.

void Pet::feed() {
    setState(STATE_EATING);                  // Signal that the pet is now eating
    setFullness(fullness + 30);              // Eating fills the pet up
    setHappy(happy + 10);                    // Slightly increase happiness
    setEnergised(energised - 5);             // Small energy cost
}

void Pet::sleep() {
    setState(STATE_SLEEPING);                // Signal that the pet is now sleeping
    setTired(tired - 40);                    // Reduce tiredness significantly
    setEnergised(energised + 30);            // Restore energy
    setFullness(fullness - 10);              // Sleeping burns through a little fullness
}

void Pet::play() {
    setState(STATE_PLAYING);                 // Signal that the pet is now playing
    setHappy(happy + 25);                    // Significantly increase happiness
    setTired(tired + 20);                    // Increase tiredness
    setEnergised(energised - 5);             // Use a little energy (kept low so repeated play is not an instant-kill)
    setFullness(fullness - 15);              // Playing burns off fullness (makes them hungrier)
}

void Pet::bathe() {
    setState(STATE_BATHING);                 // Signal that the pet is now being cleaned
    setCleanliness(cleanliness + 30);        // Improve cleanliness
    setTired(tired + 10);                    // Bathing is tiring
    setEnergised(energised - 10);            // Uses some energy
}

void Pet::heal() {
    setState(STATE_HEALING);                 // Signal that the pet is now receiving treatment
    setSick(sick - 50);                      // Reduce sickness
    setTired(tired + 20);                    // Medicine makes pet tired
    setHappy(happy - 5);                     // Unpleasant experience
}

// getState()
// Returns the behaviour the pet is currently in so other systems can react to it.
PetState Pet::getState() const {
    return currentState;
}

// isInDeadState()
// Returns true when the state machine is in STATE_DEAD. main.cpp uses this to
// decide between the death screen and the normal update tick, without needing to
// know the PetState enum values.
bool Pet::isInDeadState() const {
    return currentState == STATE_DEAD;
}

// setState()
// Changes the pet's current behaviour. All state transitions go through here
// so there is always one place to look when debugging unexpected state changes.
void Pet::setState(PetState newState) {
    currentState = newState;
}

// updateState()
// Runs once per loop. Checks the current state and applies any behaviour that
// belongs to it, and plays the pet's own alert and death sounds through the
// given speaker. Add new states here as the program grows.
void Pet::updateState(
    #ifdef ENABLE_SOUND
    SpeakerManager& speaker
    #endif
) {
    // Death overrides every other state — if any critical stat is fatal, stop here.
    if (isDead()) {
        #ifdef ENABLE_SOUND
        // The death sound plays only on the first frame of death (when transitioning
        // from a living state) so it sounds exactly once per death.
        if (currentState != STATE_DEAD) {
            speaker.playDeathSound();
        }
        #endif
        setState(STATE_DEAD);
        return;
    }

    #ifdef ENABLE_SOUND
    // The alerts exist only to make a sound, so the whole block lives behind the
    // sound switch. Check whether a low-fullness (hunger) alert is due — same
    // millis() pattern as TimerManager — then play it and reset the timer.
    if (fullness <= FULLNESS_ALERT_THRESHOLD) {
        if (millis() - lastFullnessAlertTime >= FULLNESS_ALERT_INTERVAL) {
            speaker.playHungerAlertSound();
            lastFullnessAlertTime = millis();
        }
    }

    // Check whether a sickness alert is due — same pattern as the fullness alert above.
    if (sick >= SICKNESS_ALERT_THRESHOLD) {
        if (millis() - lastSicknessAlertTime >= SICKNESS_ALERT_INTERVAL) {
            speaker.playSicknessAlertSound();
            lastSicknessAlertTime = millis();
        }
    }
    #endif

    switch (currentState) {
        case STATE_IDLE:
            // If the sick stat is dangerously high, transition to sick automatically
            if (sick >= 50) {
                setState(STATE_SICK);
            }
            break;

        case STATE_EATING:
            // Eating is handled instantly by feed() — return to idle
            setState(STATE_IDLE);
            break;

        case STATE_SLEEPING:
            // Sleeping is handled instantly by sleep() — return to idle
            setState(STATE_IDLE);
            break;

        case STATE_PLAYING:
            // Playing is handled instantly by play() — return to idle
            setState(STATE_IDLE);
            break;

        case STATE_SICK:
            // Pet stays sick until heal() is called — no automatic return to idle
            break;

        case STATE_HEALING:
            // Healing is handled instantly by heal() — return to idle
            setState(STATE_IDLE);
            break;

        case STATE_BATHING:
            // Bathing is handled instantly by bathe() — return to idle
            setState(STATE_IDLE);
            break;

        case STATE_DEAD:
            // Nothing to do here — main.cpp handles the death screen and restart button.
            break;
    }
}

// isDead()
// Checks whether any critical stat has reached a fatal level.
// Fullness at 0 means the pet has starved. Energy or happiness at 0 means
// the pet has given up. Any one of these ends the pet's life.
bool Pet::isDead() const {
    if (fullness <= 0) {
        return true;
    }
    if (energised <= 0) {
        return true;
    }
    if (happy <= 0) {
        return true;
    }
    return false;
}

// reset()
// Restores all stats to the same starting values used in the constructor.
// Uses the DEFAULT_* constants so the constructor, StorageManager::load(),
// and reset() all share one source of truth — change a starting value in
// pet.h and every code path agrees.
// Called when the user chooses to restart after the pet has died.
void Pet::reset(
    #ifdef ENABLE_SOUND
    SpeakerManager& speaker
    #endif
) {
    fullness      = DEFAULT_FULLNESS;
    tired       = DEFAULT_TIRED;
    happy       = DEFAULT_HAPPY;
    sick        = DEFAULT_SICK;
    sad         = DEFAULT_SAD;
    cleanliness = DEFAULT_CLEANLINESS;
    energised   = DEFAULT_ENERGISED;
    currentState = STATE_IDLE;  // Clear death state so the next updateState() starts fresh

    // Clear the alert timers so no leftover rate-limit state carries into the new life.
    lastFullnessAlertTime  = 0;
    lastSicknessAlertTime = 0;

    #ifdef ENABLE_SOUND
    // Play the restart fanfare. The pet owns this lifecycle sound, just like its
    // death and alert sounds.
    speaker.playResetSound();
    #endif
}

// computeMood()
// Maps the pet's current stats to one of the four visual moods, using a fixed
// priority order so the sprite always shows the single most important thing.
// We check the worst problems first: being unwell matters more than being
// hungry (low fullness), which matters more than being happy. The first rule
// that matches wins and we return straight away; if none match, it is neutral.
//
// Read this as a ladder of "if this is true, stop here": as soon as one
// condition is met its mood is returned, so the rules below it never run.
// To change the priority order, reorder these blocks. To add a new mood, add
// a value to MoodSprite (in screen_layout.h) and a new block here.
//
// Note: the four moods deliberately do not cover every danger. A pet can die
// from low happiness or low energy (see isDead()), but neither has its own
// mood here, so the face stays NEUTRAL as those stats fall. Adding a "sad" or
// "tired" mood for those is a natural extension exercise.
MoodSprite Pet::computeMood() const {
    #ifdef ENABLE_MOOD_SPRITES
    if (sick > 50) {
        return MOOD_UNWELL;
    }
    if (fullness < 30) {
        return MOOD_HUNGRY;
    }
    if (happy > 70) {
        return MOOD_HAPPY;
    }
    #endif
    // With mood sprites off, the pet has only one face: every state is NEUTRAL.
    return MOOD_NEUTRAL;
}

// constrainValue()
// Takes a single number and returns it forced into the legal 0..100 range.
// Called only by the setters. Every other route to stat mutation (the action
// methods feed/sleep/play/bathe/heal) goes through the setters, so this one
// helper is enough to guarantee no stat ever escapes 0..100.
//
// Returns the constrained value rather than modifying in place, so each
// setter reads as one self-contained line: hand in a number, store the
// constrained number back.
int Pet::constrainValue(int value) const {
    if (value < 0)   { return 0; }
    if (value > 100) { return 100; }
    return value;
}
