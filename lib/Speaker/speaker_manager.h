#ifndef SPEAKER_MANAGER_H
#define SPEAKER_MANAGER_H

#include <stdint.h>

// One note of a song. freq is the tone frequency in Hz (0 means a rest/silence).
// length is the note's musical duration measured in SIXTEENTH-notes (a quarter
// note is 4, an eighth 2, a dotted quarter 6, ...). The real time each note takes
// is derived from the song's tempo (bpm) at playback, so the same table plays at
// the right speed regardless of tempo. Songs are plain arrays of these — a "song"
// is just data. See the note/rhythm constants and the SONGS table in the .cpp.
struct SongNote {
    uint16_t freq;
    uint16_t length;  // duration in sixteenth-notes
};

// SpeakerManager
// Wraps M5.Speaker to play short buzzer melodies for key pet events.
// Each public method corresponds to one in-game event. Feed, play, sleep, etc.
// Call init() once in setup(), then call the appropriate play method when an event occurs.
//
// It also hosts a small NON-BLOCKING song player (startSong/updateSong/stopSound)
// used by the phone dashboard. Unlike the event jingles above — which run to
// completion with blocking delay() — a song can be several seconds long, so it is
// advanced one note per loop() tick instead of freezing the pet.
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

    // --- Non-blocking song player (driven from the phone dashboard) ---

    // How many built-in songs are available to choose from.
    static int songCount();

    // Display name of song `id` (0 .. songCount()-1), or "" if the id is invalid.
    static const char* songName(int id);

    // Begins playing song `id` from the start. Non-blocking: the first note starts
    // immediately, and updateSong() plays the rest over the following loop ticks.
    // An out-of-range id is ignored. Starting a song replaces any song in progress.
    void startSong(int id);

    // Immediately stops any song in progress and silences the speaker.
    void stopSong();

    // True while a song is actively playing.
    bool isSongPlaying() const;

    // Advances the current song based on how much time has elapsed. Call once per
    // loop() tick. Does nothing when no song is playing.
    void updateSong();

private:
    // Plays the note at songIndex and stamps noteStartMs. Rests (freq 0) just
    // silence the speaker for the note's duration.
    void playCurrentNote();

    const SongNote* songNotes  = nullptr;  // notes of the song currently playing
    int             songLength = 0;        // number of notes in songNotes
    int             songIndex  = 0;        // index of the note currently sounding
    unsigned long   noteStartMs = 0;       // millis() when the current note began
    unsigned long   msPerSixteenth = 0;    // real time of one sixteenth-note, from tempo
    bool            songActive  = false;   // true between startSong() and the end
};

#endif // SPEAKER_MANAGER_H
