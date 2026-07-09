#include <unity.h>
#include <Arduino.h>
#include "../../lib/Speaker/speaker_manager.h"

// These tests exercise the NON-BLOCKING song player on the host. The stub
// Arduino.h gives us a controllable clock via setMillis(); the stub M5.Speaker
// swallows tone()/stop() calls, so we assert on the player's own state
// (isSongPlaying) rather than on any real sound.
//
// The player advances at most one note per updateSong() call, and only once the
// current note's slot has elapsed. Every test starts the clock from a known
// value so the elapsed-time comparisons are deterministic.

void setUp(void) { setMillis(0); }
void tearDown(void) {}

// startSong() on a valid id begins playback immediately.
void test_start_marks_playing(void) {
    SpeakerManager speaker;
    speaker.startSong(0);
    TEST_ASSERT_TRUE(speaker.isSongPlaying());
}

// Out-of-range ids are ignored and never start playback.
void test_out_of_range_id_ignored(void) {
    SpeakerManager speaker;
    speaker.startSong(-1);
    TEST_ASSERT_FALSE(speaker.isSongPlaying());
    speaker.startSong(SpeakerManager::songCount());  // one past the last valid id
    TEST_ASSERT_FALSE(speaker.isSongPlaying());
}

// The song keeps playing while the first note's time slot has not yet elapsed.
void test_stays_on_note_until_duration_elapses(void) {
    SpeakerManager speaker;
    speaker.startSong(0);
    setMillis(10);            // well within the first note
    speaker.updateSong();
    TEST_ASSERT_TRUE(speaker.isSongPlaying());
}

// Driving the clock forward past every note eventually ends the song.
void test_song_completes_and_stops(void) {
    SpeakerManager speaker;
    speaker.startSong(0);
    unsigned long t = 0;
    // Each tick jumps 1s (longer than any single note) so exactly one note is
    // consumed per updateSong(). The song is finite, so this terminates.
    for (int i = 0; i < 200 && speaker.isSongPlaying(); i++) {
        t += 1000;
        setMillis(t);
        speaker.updateSong();
    }
    TEST_ASSERT_FALSE(speaker.isSongPlaying());
}

// A song containing rests (id 2 = Mario) also drains cleanly without hanging.
void test_song_with_rests_completes(void) {
    SpeakerManager speaker;
    speaker.startSong(2);
    unsigned long t = 0;
    for (int i = 0; i < 200 && speaker.isSongPlaying(); i++) {
        t += 1000;
        setMillis(t);
        speaker.updateSong();
    }
    TEST_ASSERT_FALSE(speaker.isSongPlaying());
}

// The longer song (id 3, a chromatic tempo-based melody) also drains cleanly.
void test_long_song_completes(void) {
    SpeakerManager speaker;
    speaker.startSong(3);
    unsigned long t = 0;
    for (int i = 0; i < 400 && speaker.isSongPlaying(); i++) {
        t += 1000;
        setMillis(t);
        speaker.updateSong();
    }
    TEST_ASSERT_FALSE(speaker.isSongPlaying());
}

// stopSong() halts playback immediately.
void test_stop_song_halts_playback(void) {
    SpeakerManager speaker;
    speaker.startSong(0);
    speaker.stopSong();
    TEST_ASSERT_FALSE(speaker.isSongPlaying());
}

// updateSong() on an idle player is a harmless no-op.
void test_update_when_idle_is_noop(void) {
    SpeakerManager speaker;
    speaker.updateSong();
    TEST_ASSERT_FALSE(speaker.isSongPlaying());
}

// The song registry exposes at least one song, and songName() bounds-checks.
void test_song_metadata(void) {
    TEST_ASSERT_TRUE(SpeakerManager::songCount() >= 1);
    TEST_ASSERT_TRUE(SpeakerManager::songName(0)[0] != '\0');
    TEST_ASSERT_EQUAL_STRING("", SpeakerManager::songName(-1));
    TEST_ASSERT_EQUAL_STRING("", SpeakerManager::songName(SpeakerManager::songCount()));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_start_marks_playing);
    RUN_TEST(test_out_of_range_id_ignored);
    RUN_TEST(test_stays_on_note_until_duration_elapses);
    RUN_TEST(test_song_completes_and_stops);
    RUN_TEST(test_song_with_rests_completes);
    RUN_TEST(test_long_song_completes);
    RUN_TEST(test_stop_song_halts_playback);
    RUN_TEST(test_update_when_idle_is_noop);
    RUN_TEST(test_song_metadata);
    return UNITY_END();
}
