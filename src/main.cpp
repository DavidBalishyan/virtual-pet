#include <Arduino.h>
#include "M5StickCPlus2.h"
#include "../lib/Config/scaffold_config.h"  // The ENABLE_* feature switches — read this first.
#include "../lib/Pet/pet.h"
#include "../lib/Display/display_manager.h"
#include "../lib/Imu/tilt_motion.h"
#include "../lib/Button/button_handler.h"
#ifdef ENABLE_ACTION_MENU
#include "../lib/Actions/action_menu.h"
#endif
#include "../lib/Navigation/navigation_manager.h"
#include "../lib/Timer/time_manager.h"
#include "../lib/Imu/imu_manager.h"
#ifdef ENABLE_SOUND
#include "../lib/Speaker/speaker_manager.h"
#endif
#ifdef ENABLE_PERSISTENCE
#include "../lib/Storage/storage_manager.h"
#endif

// -------------------------------------------------------------------------
// SPRITE_TEST — quick render test for Task 12.
// When this is defined, setup() draws one sprite to the screen and stops.
// The entire game loop is bypassed — nothing else runs.
// To return to normal operation, comment out or delete this #define.
// -------------------------------------------------------------------------
// #define SPRITE_TEST
#include "../lib/Display/sprites/80x80_test.h"

// -------------------------------------------------------------------------
// TILT_MOVEMENT_ENABLED — on/off switch for the optional tilt-movement demo.
// Tilt is part of the motion feature, so it only exists when ENABLE_IMU_PLAY
// is on. When true, the pet sprite slides around the screen to follow how the
// device is tilted (driven by the accelerometer). When false, the pet is
// drawn dead-centre exactly as it was before this feature existed.
// Flip it to false and rebuild to revert to the plain (centred) pet.
// -------------------------------------------------------------------------
#ifdef ENABLE_IMU_PLAY
static const bool TILT_MOVEMENT_ENABLED = true;
#endif

// Global instances — one object per system area.
// Each manager is responsible for exactly one job.
Pet             myPet;    // Holds all of the pet's stats and care actions.
DisplayManager  display;  // Draws everything to the screen.
ButtonHandler   buttons;  // Reads and tracks button presses.
#ifdef ENABLE_ACTION_MENU
ActionMenu      menu;     // Manages the list of actions the user can choose.
#endif
NavigationManager navManager; // Tracks which screen is active and routes button input.
TimerManager    timers;   // Handles all automatic stat changes over time.
#ifdef ENABLE_IMU_PLAY
ImuManager      imu;      // Reads accelerometer data and detects shake gestures.
TiltMotion      spriteMotion; // Turns live tilt into a smoothed pet-sprite screen offset.
#endif
#ifdef ENABLE_SOUND
SpeakerManager  speaker;  // Plays buzzer melodies for pet events and alerts.
#endif
#ifdef ENABLE_PERSISTENCE
StorageManager  storage;  // Saves and loads pet stats to NVS flash storage.
#endif

// handleDeathScreen() — the only interaction once the pet has died: pressing
// Button A starts a new game by resetting the pet's stats.
void handleDeathScreen() {
    if (buttons.wasButtonAPressed()) {
        myPet.reset(
            #ifdef ENABLE_SOUND
            speaker
            #endif
        );
        #ifdef ENABLE_PERSISTENCE
        // Clear the saved data too, so the dead pet does not reload on next boot.
        storage.clear();
        #endif
    }
}

#ifdef DEBUG
// printPetStateToSerial() — every few seconds, print the pet's stats to the
// Serial Monitor so you can watch them change live (and confirm a saved stat
// reloaded after a power-cycle). Throttled with the same millis() pattern as the
// stat timers so it does not flood the monitor. Only compiled when DEBUG is on
// (uncomment the build flag in platformio.ini), then open it with `pio device monitor`.
void printPetStateToSerial() {
    static unsigned long lastPrintTime = 0;
    const unsigned long PRINT_INTERVAL = 3000;  // print once every 3 seconds

    if (millis() - lastPrintTime < PRINT_INTERVAL) {
        return;  // not time yet — leave without printing
    }
    lastPrintTime = millis();

    Serial.printf("Fullness:%d  Happy:%d  Energy:%d  Clean:%d  Sick:%d  Hydration:%d\n",
                  myPet.getFullness(), myPet.getHappy(), myPet.getEnergised(),
                  myPet.getCleanliness(), myPet.getSick(), myPet.getHydration());
}
#endif

// updateLivePet() — one frame of normal gameplay while the pet is alive: run the
// automatic stat timers, cycle the menu when the Interact screen is showing, let
// navigation switch screens, confirm a chosen action, and play on a shake gesture.
void updateLivePet() {
    // Run all automatic stat changes (fullness decay, happiness decay, energy drain).
    // The rules for what changes and how fast live in TimerManager, not here.
    timers.update(myPet);

    #ifdef ENABLE_ACTION_MENU
    // Only cycle through menu actions when the Interact screen is visible.
    // Calling menu.update() on other screens would silently change the selected
    // action while the user cannot even see the menu.
    if (navManager.getCurrentScreen() == SCREEN_INTERACT) {
        menu.update(buttons);
    }
    #endif

    // Update navigation — reads button input and switches screens if needed.
    // This must run AFTER menu.update() so the latest "is Back highlighted?"
    // value is what NavigationManager sees. We pass that single fact as a
    // bool rather than handing over the whole menu object — NavigationManager
    // does not need to know what an ActionMenu is. Reading it into a named local
    // gives us one place to supply a fallback when the menu is switched off.
    #ifdef ENABLE_ACTION_MENU
    bool backSelected = menu.isBackSelected();
    #else
    bool backSelected = false;  // No menu, so "Back" can never be highlighted.
    #endif
    navManager.update(buttons, backSelected);

    #ifdef ENABLE_ACTION_MENU
    // shouldConfirmAction() is true for exactly one frame when the user pressed A
    // on a non-Back action. Calling confirmAction() here keeps the pet/speaker/storage
    // logic out of NavigationManager, which only handles screen transitions.
    if (navManager.shouldConfirmAction()) {
        menu.confirmAction(myPet, display
            #ifdef ENABLE_SOUND
            , speaker
            #endif
            #ifdef ENABLE_PERSISTENCE
            , storage
            #endif
            );
    }
    #endif

    #ifdef ENABLE_IMU_PLAY
    // A shake gesture triggers play from any screen.
    // wasShaken() fires only on the first frame of a gesture so play() is called once.
    if (imu.wasShaken()) {
        myPet.play();
    }

    // CHALLENGE (Session 3): when the tilt demo is on, make a big tilt trigger an
    // interaction of YOUR choice. The live tilt is imu.getAccelX() / imu.getAccelY()
    // (or spriteMotion's offset). Copy the shape of the shake check above — e.g.
    // "if the device is tilted far enough, call myPet.sleep();" — and decide which
    // action it fires (feed / play / sleep / bathe / heal).
    #endif
}

// renderCurrentScreen() — draws the whole screen for this frame. DisplayManager
// takes plain values (not manager objects), so we pull the handful of stats and
// menu fields it needs and hand them over. NavigationManager decides which screen
// to draw, and isInDeadState() tells it whether to show the death screen.
void renderCurrentScreen() {
    // Pull the two menu-derived values the display needs into named locals.
    // With the action menu switched off there is no menu to ask, so we supply
    // harmless fallbacks: no action name and no highlighted stat.
    #ifdef ENABLE_ACTION_MENU
    Action selectedAction = menu.getSelectedAction();
    const char*  selectedActionName = selectedAction.name;
    RelevantStat selectedActionStat = selectedAction.relevantStat;
    #else
    const char*  selectedActionName = "";
    RelevantStat selectedActionStat = STAT_NONE;
    #endif

    // Work out the sprite offset to draw with. When the tilt demo is on we use
    // the helper's smoothed values; when it is off we pass 0, 0 so the pet draws
    // dead-centre exactly as it did before this feature existed. Reading these
    // into named variables keeps the renderDisplay() call below easy to follow.
    int spriteOffsetX = 0;
    int spriteOffsetY = 0;
    #ifdef ENABLE_IMU_PLAY
    if (TILT_MOVEMENT_ENABLED) {
        spriteOffsetX = spriteMotion.getOffsetX();
        spriteOffsetY = spriteMotion.getOffsetY();
    }
    #endif

    display.renderDisplay(
        myPet.getHappy(), myPet.getFullness(), myPet.getEnergised(),
        myPet.getCleanliness(), myPet.getSick(), myPet.getHydration(),
        myPet.computeMood(),
        selectedActionName,
        selectedActionStat,
        myPet.isInDeadState(), myPet.getPetName(),
        navManager.getCurrentScreen(),
        spriteOffsetX, spriteOffsetY
    );
}

// -------------------------------------------------------------------------
// Where the program starts.
// This is an embedded device, so there is no main() that we write ourselves.
// When the M5StickC powers on, the ESP32's Arduino core runs its own start-up
// code and then calls the two functions below for us:
//   • setup() runs ONCE — use it to initialise the hardware and load saved data.
//   • loop()  runs FOREVER — the core calls it over and over, as fast as it can.
// Together, setup() + loop() are this program's main(): the one-time set-up,
// then an endless cycle of "read inputs, update the pet, draw the screen".
// -------------------------------------------------------------------------
void setup() {
    M5.begin();

    #ifdef SPRITE_TEST
    // Clear the screen to black so transparent pixels show the background colour.
    M5.Lcd.fillScreen(TFT_BLACK);

    // Calculate the top-left corner that centres the sprite on the screen.
    int spriteX = (135 - SPRITE_80X80_TEST_WIDTH)  / 2;
    int spriteY = (240 - SPRITE_80X80_TEST_HEIGHT) / 2;

    // Draw the sprite. The last argument is the transparent colour key —
    // any pixel matching 0xF81F (magenta) is skipped, letting black show through.
    M5.Lcd.pushImage(spriteX, spriteY,
                     SPRITE_80X80_TEST_WIDTH,
                     SPRITE_80X80_TEST_HEIGHT,
                     sprite_80x80_test[0],
                     SPRITE_TRANSPARENT_COLOR);
    return; // Skip all normal initialisation.
    #endif

    #ifdef DEBUG
    Serial.begin(115200);
    #endif

    display.init();
    #ifdef ENABLE_SOUND
    speaker.init();
    #endif

    #ifdef ENABLE_PERSISTENCE
    // Restore the pet's stats from NVS flash storage.
    // If no save data exists yet (first boot), load() falls back to healthy defaults.
    // With persistence off, the pet just starts from its constructor defaults.
    storage.load(myPet);
    #endif

    display.showMessage("Virtual Pet initialized!");
    delay(2000);
}

void loop() {
    #ifdef SPRITE_TEST
    return; // Nothing to do — sprite is already drawn in setup().
    #endif

    M5.update();      // Read the latest hardware state (buttons, IMU, etc.)
    buttons.update(); // Detect which buttons were pressed this frame
    #ifdef ENABLE_IMU_PLAY
    imu.update();     // Read fresh accelerometer data and update shake detection

    // Feed the latest tilt into the sprite-motion helper so it can ease the pet
    // toward where the device is leaning. Part of the motion feature, so it only
    // runs under ENABLE_IMU_PLAY; the inner toggle then turns the demo on/off.
    if (TILT_MOVEMENT_ENABLED) {
        spriteMotion.update(imu.getAccelX(), imu.getAccelY());
    }
    #endif

    // Run the state machine — sets STATE_DEAD when a stat hits a fatal level, and
    // plays the pet's own fullness/sickness/death sounds when a stat crosses its
    // warning threshold. Runs first so the dead check below is always up to date.
    myPet.updateState(
        #ifdef ENABLE_SOUND
        speaker
        #endif
    );

    // Once dead, the only interaction is "press A to restart"; while alive, run
    // the full game tick. Each branch lives in its own helper so this loop reads
    // as a short outline of the frame.
    if (myPet.isInDeadState()) {
        handleDeathScreen();
    } else {
        updateLivePet();
    }

    // Draw the fully updated state at the end of every frame.
    renderCurrentScreen();

    #ifdef DEBUG
    // Print the pet's stats to the Serial Monitor (throttled) so you can watch
    // them change live — open it with `pio device monitor`.
    printPetStateToSerial();
    #endif
}
