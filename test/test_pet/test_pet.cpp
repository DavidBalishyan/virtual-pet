#include <unity.h>
#include <Arduino.h>

#include "../../lib/Pet/pet.h"

// Helpers
static Pet pet;
static SpeakerManager speaker;

void setUp(void) {
    setMillis(0);
    pet = Pet();  // fresh pet before each test
}

void tearDown(void) {}

// Constructor & defaults

void test_default_fullness(void) {
    TEST_ASSERT_EQUAL_INT(Pet::DEFAULT_FULLNESS, pet.getFullness());
}

void test_default_happy(void) {
    TEST_ASSERT_EQUAL_INT(Pet::DEFAULT_HAPPY, pet.getHappy());
}

void test_default_energised(void) {
    TEST_ASSERT_EQUAL_INT(Pet::DEFAULT_ENERGISED, pet.getEnergised());
}

void test_default_sick(void) {
    TEST_ASSERT_EQUAL_INT(Pet::DEFAULT_SICK, pet.getSick());
}

void test_default_state_idle(void) {
    TEST_ASSERT_EQUAL(STATE_IDLE, pet.getState());
}

void test_default_not_dead(void) {
    TEST_ASSERT_FALSE(pet.isDead());
    TEST_ASSERT_FALSE(pet.isInDeadState());
}

// Setters & constraining

void test_setter_constrains_above_100(void) {
    pet.setFullness(200);
    TEST_ASSERT_EQUAL_INT(100, pet.getFullness());
}

void test_setter_constrains_below_0(void) {
    pet.setFullness(-50);
    TEST_ASSERT_EQUAL_INT(0, pet.getFullness());
}

void test_setter_allows_normal(void) {
    pet.setFullness(42);
    TEST_ASSERT_EQUAL_INT(42, pet.getFullness());
}

void test_all_setters_constrain(void) {
    pet.setHappy(150);
    pet.setTired(-10);
    pet.setSick(300);
    pet.setCleanliness(99);
    pet.setEnergised(-1);
    pet.setSad(50);
    TEST_ASSERT_EQUAL_INT(100, pet.getHappy());
    TEST_ASSERT_EQUAL_INT(0, pet.getTired());
    TEST_ASSERT_EQUAL_INT(100, pet.getSick());
    TEST_ASSERT_EQUAL_INT(99, pet.getCleanliness());
    TEST_ASSERT_EQUAL_INT(0, pet.getEnergised());
    TEST_ASSERT_EQUAL_INT(50, pet.getSad());
}

// Care actions

void test_feed_increases_fullness(void) {
    int before = pet.getFullness();
    pet.feed();
    TEST_ASSERT_TRUE(pet.getFullness() >= before);
}

void test_feed_sets_eating_state(void) {
    pet.feed();
    TEST_ASSERT_EQUAL(STATE_EATING, pet.getState());
}

void test_feed_does_not_overflow(void) {
    pet.setFullness(100);
    pet.feed();
    TEST_ASSERT_EQUAL_INT(100, pet.getFullness());
}

void test_sleep_reduces_tired(void) {
    pet.setTired(80);
    pet.sleep();
    TEST_ASSERT_TRUE(pet.getTired() < 80);
}

void test_sleep_sets_sleeping_state(void) {
    pet.sleep();
    TEST_ASSERT_EQUAL(STATE_SLEEPING, pet.getState());
}

void test_play_increases_happy(void) {
    int before = pet.getHappy();
    pet.play();
    TEST_ASSERT_TRUE(pet.getHappy() >= before);
}

void test_play_sets_playing_state(void) {
    pet.play();
    TEST_ASSERT_EQUAL(STATE_PLAYING, pet.getState());
}

void test_bathe_increases_cleanliness(void) {
    pet.setCleanliness(30);
    pet.bathe();
    TEST_ASSERT_TRUE(pet.getCleanliness() > 30);
}

void test_bathe_sets_bathing_state(void) {
    pet.bathe();
    TEST_ASSERT_EQUAL(STATE_BATHING, pet.getState());
}

void test_heal_reduces_sickness(void) {
    pet.setSick(80);
    pet.heal();
    TEST_ASSERT_TRUE(pet.getSick() < 80);
}

void test_heal_sets_healing_state(void) {
    pet.heal();
    TEST_ASSERT_EQUAL(STATE_HEALING, pet.getState());
}

// State machine

void test_action_state_returns_to_idle_next_frame(void) {
    pet.feed();
    pet.updateState(speaker);
    TEST_ASSERT_EQUAL(STATE_IDLE, pet.getState());
}

void test_sick_state_sticky_until_healed(void) {
    pet.setSick(70);
    pet.updateState(speaker);                     // should auto-transition IDLE → SICK
    TEST_ASSERT_EQUAL(STATE_SICK, pet.getState());

    pet.updateState(speaker);                     // stays SICK
    TEST_ASSERT_EQUAL(STATE_SICK, pet.getState());
}

void test_heal_exits_sick_state(void) {
    pet.setSick(70);
    pet.updateState(speaker);                     // -> SICK
    pet.heal();                            // -> HEALING, then next update → IDLE
    TEST_ASSERT_EQUAL(STATE_HEALING, pet.getState());
    pet.updateState(speaker);
    TEST_ASSERT_EQUAL(STATE_IDLE, pet.getState());
}

// Death
void test_dies_when_fullness_zero(void) {
    pet.setFullness(0);
    TEST_ASSERT_TRUE(pet.isDead());
}

void test_dies_when_energised_zero(void) {
    pet.setEnergised(0);
    TEST_ASSERT_TRUE(pet.isDead());
}

void test_dies_when_happy_zero(void) {
    pet.setHappy(0);
    TEST_ASSERT_TRUE(pet.isDead());
}

void test_death_sets_state_dead(void) {
    pet.setFullness(0);
    pet.updateState(speaker);
    TEST_ASSERT_TRUE(pet.isInDeadState());
}

void test_reset_revives_pet(void) {
    pet.setFullness(0);
    pet.updateState(speaker);
    TEST_ASSERT_TRUE(pet.isInDeadState());

    pet.reset(speaker);
    TEST_ASSERT_FALSE(pet.isDead());
    TEST_ASSERT_FALSE(pet.isInDeadState());
    TEST_ASSERT_EQUAL(STATE_IDLE, pet.getState());
    TEST_ASSERT_EQUAL_INT(Pet::DEFAULT_FULLNESS, pet.getFullness());
}

// Mood

void test_mood_neutral_by_default(void) {
    // Default stats: fullness=90, happy=70, sick=0
    // sick 0 -> not > 50
    // fullness 90 -> not < 30
    // happy 70 -> not > 70 (strictly greater)
    TEST_ASSERT_EQUAL(MOOD_NEUTRAL, pet.computeMood());
}

void test_mood_happy_when_happy_high(void) {
    pet.setHappy(100);
    pet.setSick(0);
    pet.setFullness(90);
    TEST_ASSERT_EQUAL(MOOD_HAPPY, pet.computeMood());
}

void test_mood_hungry_when_fullness_low(void) {
    pet.setFullness(20);
    pet.setSick(0);
    pet.setHappy(80);
    // Hungry (fullness < 30) beats Happy (happy > 70) in priority
    TEST_ASSERT_EQUAL(MOOD_HUNGRY, pet.computeMood());
}

void test_mood_unwell_when_sick_high(void) {
    pet.setSick(60);
    pet.setFullness(10);  // would trigger HUNGRY, but UNWELL has higher priority
    pet.setHappy(80);
    TEST_ASSERT_EQUAL(MOOD_UNWELL, pet.computeMood());
}

void test_mood_neutral_at_boundary(void) {
    pet.setHappy(70);    // not > 70
    pet.setFullness(30); // not < 30
    pet.setSick(50);     // not > 50
    TEST_ASSERT_EQUAL(MOOD_NEUTRAL, pet.computeMood());
}

// Pet name
void test_pet_name_not_empty(void) {
    const char* name = pet.getPetName();
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_TRUE(name[0] != '\0');
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_default_fullness);
    RUN_TEST(test_default_happy);
    RUN_TEST(test_default_energised);
    RUN_TEST(test_default_sick);
    RUN_TEST(test_default_state_idle);
    RUN_TEST(test_default_not_dead);
    RUN_TEST(test_setter_constrains_above_100);
    RUN_TEST(test_setter_constrains_below_0);
    RUN_TEST(test_setter_allows_normal);
    RUN_TEST(test_all_setters_constrain);
    RUN_TEST(test_feed_increases_fullness);
    RUN_TEST(test_feed_sets_eating_state);
    RUN_TEST(test_feed_does_not_overflow);
    RUN_TEST(test_sleep_reduces_tired);
    RUN_TEST(test_sleep_sets_sleeping_state);
    RUN_TEST(test_play_increases_happy);
    RUN_TEST(test_play_sets_playing_state);
    RUN_TEST(test_bathe_increases_cleanliness);
    RUN_TEST(test_bathe_sets_bathing_state);
    RUN_TEST(test_heal_reduces_sickness);
    RUN_TEST(test_heal_sets_healing_state);
    RUN_TEST(test_action_state_returns_to_idle_next_frame);
    RUN_TEST(test_sick_state_sticky_until_healed);
    RUN_TEST(test_heal_exits_sick_state);
    RUN_TEST(test_dies_when_fullness_zero);
    RUN_TEST(test_dies_when_energised_zero);
    RUN_TEST(test_dies_when_happy_zero);
    RUN_TEST(test_death_sets_state_dead);
    RUN_TEST(test_reset_revives_pet);
    RUN_TEST(test_mood_neutral_by_default);
    RUN_TEST(test_mood_happy_when_happy_high);
    RUN_TEST(test_mood_hungry_when_fullness_low);
    RUN_TEST(test_mood_unwell_when_sick_high);
    RUN_TEST(test_mood_neutral_at_boundary);
    RUN_TEST(test_pet_name_not_empty);
    return UNITY_END();
}
