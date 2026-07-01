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
// Two ascending notes — C5 then E5 — give a quick "happy eating" cue.
// The short durations keep the sound snappy so it doesn't feel slow.
void SpeakerManager::playFeedSound() {
    M5.Speaker.tone(523, 120);  // C5
    delay(140);
    M5.Speaker.tone(659, 180);  // E5
    delay(200);
    M5.Speaker.stop();
}

// playPlaySound()
// Three ascending notes — C5, G5, C6 — give an energetic, bouncy feel.
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
// Three descending notes — A4, F4, D4 — create a gentle lullaby-like descent.
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
// Three notes that climb then return — C5, G5, C5 — mimic a playful splash.
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
// Four notes rising from G4 to E5 — a classic "resolution" pattern.
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
// Four notes descending from A4 to A3 — a slow, sad farewell.
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
// Four notes rising from C5 to C6 — a triumphant "new life" fanfare.
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
// Two short high beeps at A5 — an urgent, attention-grabbing warning.
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
// Two short beeps at D4 — a lower, more ominous pitch than the fullness alert
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
// Two ascending notes — C5 then E5 — give a soft, confident "data saved" chime.
// Intentionally shorter and quieter-feeling than the care action melodies so it
// does not interrupt the game feel — it is a background confirmation, not a celebration.
void SpeakerManager::playSaveSound() {
    M5.Speaker.tone(523, 80);   // C5
    delay(100);
    M5.Speaker.tone(659, 150);  // E5
    delay(180);
    M5.Speaker.stop();
}

// playThirstAlertSound()
// Two rising beeps — G5 then C6 — a bright, insistent warning that the pet needs water.
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
// A quick descending two-note "glug" — E5 then C5 — to suggest drinking.
void SpeakerManager::playDrinkSound() {
    M5.Speaker.tone(659, 100);  // E5
    delay(120);
    M5.Speaker.tone(523, 150);  // C5
    delay(180);
    M5.Speaker.stop();
}
