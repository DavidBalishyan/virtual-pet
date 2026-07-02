#ifndef SPEAKER_MANAGER_H
#define SPEAKER_MANAGER_H

// SpeakerManager
// Wraps M5.Speaker to play short buzzer melodies for key pet events.
// Each public method corresponds to one in-game event — feed, play, sleep, etc.
// Call init() once in setup(), then call the appropriate play method when an event occurs.
class SpeakerManager {
public:
    // Sets the speaker volume to a comfortable level for the built-in buzzer.
    void init();

    // Plays a short ascending two-note melody to signal that the pet has been fed.
    void playFeedSound();

    // Plays a bouncy three-note ascending melody to signal that the pet is playing.
    void playPlaySound();

    // Plays a gentle three-note descending melody to signal that the pet is going to sleep.
    void playSleepSound();

    // Plays a three-note bouncing melody to signal that the pet has been bathed.
    void playBatheSound();

    // Plays a four-note rising resolution to signal that the pet has been healed.
    void playHealSound();

    // Plays a four-note descending sad melody when the pet has died.
    void playDeathSound();

    // Plays a four-note ascending fanfare when the pet is reset and ready to play again.
    void playResetSound();

    // Plays a two-beep urgent warning when the pet's fullness is dangerously low.
    void playHungerAlertSound();

    // Plays a two-beep warning at a lower pitch when the pet's sickness is dangerously high.
    void playSicknessAlertSound();

    // Plays a two-beep rising warning when the pet's hydration is dangerously low.
    void playThirstAlertSound();

    // Plays a short two-note ascending chime to confirm that the pet's stats have been saved.
    void playSaveSound();

    // Plays a short gurgling two-note sound for the drink action.
    void playDrinkSound();
};

#endif // SPEAKER_MANAGER_H
