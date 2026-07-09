#include <Arduino.h>
#include "M5StickCPlus2.h"
#include "speaker_manager.h"

// init()
// Prepares the built-in buzzer for playback. Must be called once in setup()
// after M5.begin() has already initialised the hardware.
void SpeakerManager::init() {
    M5.Speaker.setVolume(128);
}

// playFeedSound()
// Two ascending notes (C5 then E5) give a quick "happy eating" cue.
// The short durations keep the sound snappy so it doesn't feel slow.
void SpeakerManager::playFeedSound() {
    M5.Speaker.tone(523, 120);  // C5
    delay(140);
    M5.Speaker.tone(659, 180);  // E5
    delay(200);
    M5.Speaker.stop();
}

// playPlaySound()
// Three ascending notes (C5, G5, C6) give an energetic, bouncy feel.
// The upward leap to the octave reinforces the sense of active movement.
void SpeakerManager::playPlaySound() {
    M5.Speaker.tone(523, 100);  // C5
    delay(120);
    M5.Speaker.tone(784, 100);  // G5
    delay(120);
    M5.Speaker.tone(1047, 180); // C6
    delay(200);
    M5.Speaker.stop();
}

// playSleepSound()
// Three descending notes (A4, F4, D4) create a gentle lullaby-like descent.
// The longer duration on the final note lets it fade naturally.
void SpeakerManager::playSleepSound() {
    M5.Speaker.tone(440, 150);  // A4
    delay(180);
    M5.Speaker.tone(349, 150);  // F4
    delay(180);
    M5.Speaker.tone(294, 250);  // D4
    delay(280);
    M5.Speaker.stop();
}

// playBatheSound()
// Three notes that climb then return (C5, G5, C5) mimic a playful splash.
// The return to the root note gives it a rounded, bubbly character.
void SpeakerManager::playBatheSound() {
    M5.Speaker.tone(523, 100);  // C5
    delay(120);
    M5.Speaker.tone(784, 100);  // G5
    delay(120);
    M5.Speaker.tone(523, 180);  // C5
    delay(200);
    M5.Speaker.stop();
}

// playHealSound()
// Four notes rising from G4 to E5. A classic "resolution" pattern.
// The ascending line sounds like something being restored to full health.
void SpeakerManager::playHealSound() {
    M5.Speaker.tone(392, 100);  // G4
    delay(120);
    M5.Speaker.tone(440, 100);  // A4
    delay(120);
    M5.Speaker.tone(523, 100);  // C5
    delay(120);
    M5.Speaker.tone(659, 220);  // E5
    delay(240);
    M5.Speaker.stop();
}

// playDeathSound()
// Four notes descending from A4 to A3. A slow, sad farewell.
// The longer note durations make it feel heavy and final.
void SpeakerManager::playDeathSound() {
    M5.Speaker.tone(440, 200);  // A4
    delay(240);
    M5.Speaker.tone(349, 200);  // F4
    delay(240);
    M5.Speaker.tone(294, 200);  // D4
    delay(240);
    M5.Speaker.tone(220, 400);  // A3
    delay(440);
    M5.Speaker.stop();
}

// playResetSound()
// Four notes rising from C5 to C6. A triumphant "new life" fanfare.
// The jump to the octave at the end feels like a fresh start.
void SpeakerManager::playResetSound() {
    M5.Speaker.tone(523, 100);  // C5
    delay(120);
    M5.Speaker.tone(659, 100);  // E5
    delay(120);
    M5.Speaker.tone(784, 100);  // G5
    delay(120);
    M5.Speaker.tone(1047, 250); // C6
    delay(280);
    M5.Speaker.stop();
}

// playHungerAlertSound()
// Two short high beeps at A5. An urgent, attention-grabbing warning.
// The gap between beeps makes the double-beep pattern immediately recognisable
// as a different class of sound from the one-shot action melodies.
void SpeakerManager::playHungerAlertSound() {
    M5.Speaker.tone(880, 100);  // A5
    delay(120);
    M5.Speaker.stop();
    delay(80);
    M5.Speaker.tone(880, 100);  // A5
    delay(120);
    M5.Speaker.stop();
}

// playSicknessAlertSound()
// Two short beeps at D4. A lower, more ominous pitch than the fullness alert
// so the user can tell the two warnings apart by ear without looking at the screen.
void SpeakerManager::playSicknessAlertSound() {
    M5.Speaker.tone(294, 100);  // D4
    delay(120);
    M5.Speaker.stop();
    delay(80);
    M5.Speaker.tone(294, 100);  // D4
    delay(120);
    M5.Speaker.stop();
}

// playSaveSound()
// Two ascending notes (C5 then E5) give a soft, confident "data saved" chime.
// Intentionally shorter and quieter-feeling than the care action melodies so it
// does not interrupt the game feel. It is a background confirmation, not a celebration.
void SpeakerManager::playSaveSound() {
    M5.Speaker.tone(523, 80);   // C5
    delay(100);
    M5.Speaker.tone(659, 150);  // E5
    delay(180);
    M5.Speaker.stop();
}

// playThirstAlertSound()
// Two rising beeps (G5 then C6): a bright, insistent warning that the pet needs water.
// Higher in pitch than the hunger alert so the user can distinguish them by ear.
void SpeakerManager::playThirstAlertSound() {
    M5.Speaker.tone(784, 100);  // G5
    delay(120);
    M5.Speaker.stop();
    delay(80);
    M5.Speaker.tone(1047, 100); // C6
    delay(120);
    M5.Speaker.stop();
}

// playDrinkSound()
// A quick descending two-note "glug" (E5 then C5) to suggest drinking.
void SpeakerManager::playDrinkSound() {
    M5.Speaker.tone(659, 100);  // E5
    delay(120);
    M5.Speaker.tone(523, 150);  // C5
    delay(180);
    M5.Speaker.stop();
}

// =========================================================================
// Non-blocking song player
// =========================================================================
//
// The event jingles above run start-to-finish with blocking delay(), which is
// fine for a half-second cue. A full song is several seconds, so blocking would
// freeze input and rendering. Instead we store each song as an array of notes and
// advance ONE note per loop() tick using millis(): startSong() plays the first
// note, updateSong() moves to the next note once the current one's time is up.
//
// AUTHORING FORMAT (how to write a song)
// Each note is {pitch, length}: a NOTE_* frequency (or REST for silence) plus a
// LENGTH in sixteenth-notes — QUARTER, EIGHTH, HALF, DOTTED_QUARTER, ... The real
// time per note is derived from the song's tempo (bpm) when it plays, so you write
// music in note values, not milliseconds. To add a song: build a SongNote[] table,
// then add one row to SONGS below (its position is the id the dashboard sends).
namespace {
    // --- Pitches (Hz, equal temperament, A4 = 440). 'S' in a name = sharp. ---
    constexpr uint16_t REST = 0;  // silence

    constexpr uint16_t NOTE_G3 = 196, NOTE_A3 = 220, NOTE_B3 = 247;

    constexpr uint16_t NOTE_C4 = 262, NOTE_CS4 = 277, NOTE_D4 = 294, NOTE_DS4 = 311,
                       NOTE_E4 = 330, NOTE_F4 = 349, NOTE_FS4 = 370, NOTE_G4 = 392,
                       NOTE_GS4 = 415, NOTE_A4 = 440, NOTE_AS4 = 466, NOTE_B4 = 494;

    constexpr uint16_t NOTE_C5 = 523, NOTE_CS5 = 554, NOTE_D5 = 587, NOTE_DS5 = 622,
                       NOTE_E5 = 659, NOTE_F5 = 698, NOTE_FS5 = 740, NOTE_G5 = 784,
                       NOTE_GS5 = 831, NOTE_A5 = 880, NOTE_AS5 = 932, NOTE_B5 = 988;

    constexpr uint16_t NOTE_C6 = 1047, NOTE_CS6 = 1109, NOTE_D6 = 1175, NOTE_E6 = 1319,
                       NOTE_F6 = 1397, NOTE_G6 = 1568;

    // --- Rhythm values, in sixteenth-notes (a quarter note = 4 sixteenths). ---
    constexpr uint16_t SIXTEENTH = 1, EIGHTH = 2, DOTTED_EIGHTH = 3, QUARTER = 4,
                       DOTTED_QUARTER = 6, HALF = 8, DOTTED_HALF = 12, WHOLE = 16;

    // "Twinkle, Twinkle, Little Star" — the first two lines.
    const SongNote SONG_TWINKLE[] = {
        {NOTE_C5, QUARTER}, {NOTE_C5, QUARTER}, {NOTE_G5, QUARTER}, {NOTE_G5, QUARTER},
        {NOTE_A5, QUARTER}, {NOTE_A5, QUARTER}, {NOTE_G5, HALF},
        {NOTE_F5, QUARTER}, {NOTE_F5, QUARTER}, {NOTE_E5, QUARTER}, {NOTE_E5, QUARTER},
        {NOTE_D5, QUARTER}, {NOTE_D5, QUARTER}, {NOTE_C5, HALF},
    };

    // Beethoven's "Ode to Joy" — the main theme.
    const SongNote SONG_ODE[] = {
        {NOTE_E5, QUARTER}, {NOTE_E5, QUARTER}, {NOTE_F5, QUARTER}, {NOTE_G5, QUARTER},
        {NOTE_G5, QUARTER}, {NOTE_F5, QUARTER}, {NOTE_E5, QUARTER}, {NOTE_D5, QUARTER},
        {NOTE_C5, QUARTER}, {NOTE_C5, QUARTER}, {NOTE_D5, QUARTER}, {NOTE_E5, QUARTER},
        {NOTE_E5, DOTTED_QUARTER}, {NOTE_D5, EIGHTH}, {NOTE_D5, HALF},
    };

    // The opening bars of the "Super Mario Bros." overworld theme.
    const SongNote SONG_MARIO[] = {
        {NOTE_E5, EIGHTH}, {NOTE_E5, EIGHTH}, {REST, EIGHTH},   {NOTE_E5, EIGHTH},
        {REST, EIGHTH},    {NOTE_C5, EIGHTH}, {NOTE_E5, EIGHTH}, {REST, EIGHTH},
        {NOTE_G5, QUARTER},{REST, QUARTER},   {NOTE_G4, QUARTER},{REST, QUARTER},
        {NOTE_C5, QUARTER},{REST, EIGHTH},    {NOTE_G4, QUARTER},{REST, EIGHTH},
        {NOTE_E4, QUARTER},{REST, EIGHTH},    {NOTE_A4, DOTTED_EIGHTH}, {NOTE_B4, DOTTED_EIGHTH},
        {NOTE_A4, DOTTED_EIGHTH}, {NOTE_G4, DOTTED_EIGHTH}, {NOTE_E5, DOTTED_EIGHTH}, {NOTE_G5, DOTTED_EIGHTH},
        {NOTE_A5, QUARTER},{NOTE_F5, DOTTED_EIGHTH}, {NOTE_G5, DOTTED_EIGHTH}, {REST, EIGHTH},
        {NOTE_E5, DOTTED_EIGHTH}, {NOTE_C5, DOTTED_EIGHTH}, {NOTE_D5, DOTTED_EIGHTH}, {NOTE_B4, QUARTER},
    };

    // Beethoven's "Für Elise" (Bagatelle No. 25, WoO 59) — the well-known A theme,
    // stated twice. A longer, chromatic melody that shows off the fuller note set.
    const SongNote SONG_FUR_ELISE[] = {
        {NOTE_E5, EIGHTH}, {NOTE_DS5, EIGHTH}, {NOTE_E5, EIGHTH}, {NOTE_DS5, EIGHTH},
        {NOTE_E5, EIGHTH}, {NOTE_B4, EIGHTH}, {NOTE_D5, EIGHTH}, {NOTE_C5, EIGHTH},
        {NOTE_A4, QUARTER}, {REST, EIGHTH},
        {NOTE_C4, EIGHTH}, {NOTE_E4, EIGHTH}, {NOTE_A4, EIGHTH}, {NOTE_B4, QUARTER}, {REST, EIGHTH},
        {NOTE_E4, EIGHTH}, {NOTE_GS4, EIGHTH}, {NOTE_B4, EIGHTH}, {NOTE_C5, QUARTER}, {REST, EIGHTH},
        {NOTE_E4, EIGHTH},
        {NOTE_E5, EIGHTH}, {NOTE_DS5, EIGHTH}, {NOTE_E5, EIGHTH}, {NOTE_DS5, EIGHTH},
        {NOTE_E5, EIGHTH}, {NOTE_B4, EIGHTH}, {NOTE_D5, EIGHTH}, {NOTE_C5, EIGHTH},
        {NOTE_A4, QUARTER}, {REST, EIGHTH},
        {NOTE_C4, EIGHTH}, {NOTE_E4, EIGHTH}, {NOTE_A4, EIGHTH}, {NOTE_B4, QUARTER}, {REST, EIGHTH},
        {NOTE_E4, EIGHTH}, {NOTE_C5, EIGHTH}, {NOTE_B4, EIGHTH}, {NOTE_A4, QUARTER},
    };

    // Registry of songs. The order here IS the id the dashboard sends, so keep it
    // in sync with the buttons in lib/Wireless/dashboard.html. bpm is the tempo in
    // quarter-notes per minute.
    struct SongDef {
        const char*     name;
        const SongNote* notes;
        int             length;
        uint16_t        bpm;
    };
    // Length is computed from the table size so it never drifts out of sync.
    #define SONG_ENTRY(name, table, bpm) {name, table, (int)(sizeof(table) / sizeof(SongNote)), bpm}
    const SongDef SONGS[] = {
        SONG_ENTRY("Twinkle",    SONG_TWINKLE,   120),
        SONG_ENTRY("Ode to Joy", SONG_ODE,       100),
        SONG_ENTRY("Mario",      SONG_MARIO,     200),
        SONG_ENTRY("Fur Elise",  SONG_FUR_ELISE, 120),
    };
    #undef SONG_ENTRY
    const int SONG_COUNT = (int)(sizeof(SONGS) / sizeof(SongDef));

    // Gap subtracted from each tone so repeated same-pitch notes are audibly
    // separated. The note's full duration still governs when the next one starts.
    constexpr uint16_t NOTE_GAP_MS = 30;
}

int SpeakerManager::songCount() {
    return SONG_COUNT;
}

const char* SpeakerManager::songName(int id) {
    if (id < 0 || id >= SONG_COUNT) return "";
    return SONGS[id].name;
}

void SpeakerManager::startSong(int id) {
    if (id < 0 || id >= SONG_COUNT) return;
    songNotes  = SONGS[id].notes;
    songLength = SONGS[id].length;
    songIndex  = 0;
    // One quarter-note lasts 60000/bpm ms, and a quarter is 4 sixteenths, so one
    // sixteenth is 15000/bpm ms. Every note's real duration is its length in
    // sixteenths times this. Guard against a zero bpm just in case.
    uint16_t bpm = SONGS[id].bpm;
    msPerSixteenth = bpm > 0 ? 15000UL / bpm : 100;
    songActive = true;
    playCurrentNote();
}

void SpeakerManager::stopSong() {
    songActive = false;
    M5.Speaker.stop();
}

bool SpeakerManager::isSongPlaying() const {
    return songActive;
}

void SpeakerManager::updateSong() {
    if (!songActive) return;
    // Still within the current note's time slot? Let it keep sounding.
    unsigned long slotMs = songNotes[songIndex].length * msPerSixteenth;
    if (millis() - noteStartMs < slotMs) return;

    songIndex++;
    if (songIndex >= songLength) {
        stopSong();  // reached the end
        return;
    }
    playCurrentNote();
}

void SpeakerManager::playCurrentNote() {
    noteStartMs = millis();
    uint16_t freq = songNotes[songIndex].freq;
    if (freq == REST) {
        M5.Speaker.stop();  // a rest: silence for this note's duration
        return;
    }
    unsigned long slotMs = songNotes[songIndex].length * msPerSixteenth;
    // Sound slightly shorter than the slot so consecutive notes don't blur together.
    M5.Speaker.tone(freq, slotMs > NOTE_GAP_MS ? slotMs - NOTE_GAP_MS : slotMs);
}
