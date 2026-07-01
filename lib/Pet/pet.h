#ifndef PET_H
#define PET_H

#include "../Config/scaffold_config.h"  // ENABLE_* feature switches — read this first.
#ifdef ENABLE_SOUND
#include "../Speaker/speaker_manager.h"
#endif
#include "../Display/screen_layout.h"  // for the MoodSprite enum returned by computeMood()

// PetState — the list of behaviours the pet can currently be doing.
// Only one state is active at a time.
enum PetState {
    STATE_IDLE,      // Default — pet is awake but doing nothing
    STATE_EATING,    // Triggered by feed() — pet is eating
    STATE_SLEEPING,  // Triggered by sleep() — pet is resting
    STATE_PLAYING,   // Triggered by play() — pet is exercising
    STATE_SICK,      // Entered automatically when sick stat is high — pet is unwell
    STATE_HEALING,   // Triggered by heal() — pet is receiving treatment
    STATE_BATHING,   // Triggered by bathe() — pet is being cleaned
    STATE_DRINKING,  // Triggered by drink() — pet is drinking
    STATE_DEAD       // Entered when any critical stat reaches a fatal level
};

class Pet {
private:
    // To add a NEW STAT — the extension seam (used by the Session 2 challenge): add the
    // field here + its getter/setter (below) + a DEFAULT_ constant; a decay rule in
    // TimerManager (time_manager.cpp); a RelevantStat value (screen_layout.h) + a
    // drawContextualStatBar case (display_manager.cpp); and a care action that restores
    // it (copy feed()).
    //
    // Pet condition values (0-100 scale)
    int fullness;     // 100 = full / well-fed, 0 = empty / starving
    int tired;        // 0 = not tired, 100 = exhausted
    int happy;        // 0 = sad, 100 = very happy
    int sick;         // 0 = healthy, 100 = very sick
    int sad;          // 0 = neutral, 100 = very sad
    int cleanliness;  // 0 = dirty, 100 = very clean
    int energised;    // 0 = no energy, 100 = full energy
    int hydration;    // 0 = dehydrated, 100 = well-hydrated

    PetState currentState;  // Which behaviour the pet is currently in

    const char* petName;  // Display name shown in the title zone; default is "Pixel"

    // Timestamps used to rate-limit the alerts — same millis() pattern as TimerManager.
    // The pet plays each alert at most once per interval (see updateState()).
    unsigned long lastFullnessAlertTime;
    unsigned long lastSicknessAlertTime;
    unsigned long lastThirstAlertTime;

    // The level at which each alert fires: fullness alerts when it drops this
    // LOW (the pet is getting hungry), sickness alerts when it climbs this HIGH.
    static const int FULLNESS_ALERT_THRESHOLD = 20;
    static const int SICKNESS_ALERT_THRESHOLD = 80;
    static const int HYDRATION_ALERT_THRESHOLD = 20;

    // How many milliseconds must pass between consecutive alerts.
    static const unsigned long FULLNESS_ALERT_INTERVAL   = 15000;
    static const unsigned long SICKNESS_ALERT_INTERVAL = 15000;
    static const unsigned long THIRST_ALERT_INTERVAL   = 15000;

    // constrainValue() — returns value forced into the legal 0..100 range.
    int constrainValue(int value) const;

public:
    // Starting values for a brand-new pet — used by the constructor and by
    // StorageManager::load() so both share a single source of truth.
    static const int DEFAULT_FULLNESS    = 90;
    static const int DEFAULT_TIRED       = 20;
    static const int DEFAULT_HAPPY       = 70;
    static const int DEFAULT_SICK        = 0;
    static const int DEFAULT_SAD         = 10;
    static const int DEFAULT_CLEANLINESS = 80;
    static const int DEFAULT_ENERGISED   = 80;
    static const int DEFAULT_HYDRATION   = 80;

    // Constructor
    Pet();

    // Getters for each condition
    int getFullness() const;
    int getTired() const;
    int getHappy() const;
    int getSick() const;
    int getSad() const;
    int getCleanliness() const;
    int getEnergised() const;
    int getHydration() const;

    // Setters for each condition
    void setFullness(int value);
    void setTired(int value);
    void setHappy(int value);
    void setSick(int value);
    void setSad(int value);
    void setCleanliness(int value);
    void setEnergised(int value);
    void setHydration(int value);

    // Utility methods for pet care
    void feed();           // Increases fullness, increases happy
    void sleep();          // Decreases tired, increases energised
    void play();           // Increases happy, increases tired, decreases energised
    void bathe();          // Increases cleanliness, decreases tired
    void heal();           // Decreases sick
    void drink();          // Increases hydration
    
    // Works out the pet's current visual mood from its stats.
    // Returns a MoodSprite (defined in ../Display/screen_layout.h); the
    // display uses it to pick the sprite picture and the mood word.
    MoodSprite computeMood() const;

    // Returns true if any stat has reached a fatal level (fullness=0, energy=0, happy=0, or hydration=0)
    bool isDead() const;

    // Returns the pet's display name for use in the title zone
    const char* getPetName() const;

    // Returns the current behavioural state of the pet
    PetState getState() const;

    // Returns true when the pet is in the dead state — lets callers ask the
    // game-over question without comparing against the STATE_DEAD enum value.
    bool isInDeadState() const;

    // Changes the pet's current behavioural state
    void setState(PetState newState);

    // Runs once per loop — checks the current state and applies any behaviour that
    // belongs to it. When sound is switched on it also plays the pet's own alert
    // and death sounds, so it takes the speaker; with sound off it takes nothing.
    void updateState(
        #ifdef ENABLE_SOUND
        SpeakerManager& speaker
        #endif
    );

    // Restores all stats to their starting values so the game can begin again.
    // When sound is on it also plays the restart fanfare, so it takes the speaker.
    void reset(
        #ifdef ENABLE_SOUND
        SpeakerManager& speaker
        #endif
    );

};

#endif
