#ifndef ACTION_MENU_H
#define ACTION_MENU_H

#include "../Config/scaffold_config.h"  // ENABLE_* feature switches — read this first.
#include "../Pet/pet.h"
#include "../Display/display_manager.h"
#include "../Display/screen_layout.h"
#include "../Button/button_handler.h"
#ifdef ENABLE_SOUND
#include "../Speaker/speaker_manager.h"
#endif
#ifdef ENABLE_PERSISTENCE
#include "../Storage/storage_manager.h"
#endif

// ActionMenu manages the list of things the user can do with their pet.
// It tracks which action is highlighted and executes the action when confirmed.
//
// Button Mapping (when the Interact screen is active):
//   Button B: Next action
//   Button C: Previous action
//   Button A: Confirm selected action (handled in main.cpp)
//
// Available Actions: Feed, Play, Sleep, Bathe, Heal, Save, Back

// ---------------------------------------------------------------------------
// TO ADD A NEW ACTION — the extension seam (used by the Session 2 challenge).
// Five small steps:
//   1. Add an `ACTION_*` value to the enum below.
//   2. Add a row in the `actions[]` setup in action_menu.cpp — name, description,
//      and the `RelevantStat` it changes.
//   3. Add a `case` in `confirmAction()` (action_menu.cpp) that calls the Pet method.
//   4. Add that method to `Pet` (pet.h/.cpp) — copy `feed()` as the template.
//   5. Bump `NUM_ACTIONS` so the menu's B/C wrap-around counts the new entry.
// ---------------------------------------------------------------------------

// ActionType names every possible action the user can select.
// Each value is prefixed with ACTION_ so it does not collide with other
// names elsewhere in the project. Without the prefix a plain enum value
// like SAVE or BACK could clash with a constant of the same name in
// another library or header.
enum ActionType {
    ACTION_FEED,
    ACTION_PLAY,
    ACTION_SLEEP,
    ACTION_BATHE,
    ACTION_HEAL,
    ACTION_DRINK,
    #ifdef ENABLE_PERSISTENCE
    ACTION_SAVE,   // Only exists when persistence is on — it writes the pet to NVS
    #endif
    ACTION_BACK    // Returns the user to the Main screen without doing anything
};

// Action bundles everything DisplayManager needs to know about one menu entry:
// what to call it, what it does, and which stat it affects.
// Adding relevantStat here means the display layer never needs a separate
// switch statement to figure out which bar to highlight — it just reads the field.
struct Action {
    ActionType   type;
    const char*  name;
    const char*  description;
    RelevantStat relevantStat;  // Which pet stat this action changes (STAT_NONE if none)
};

class ActionMenu {
private:
    // The Save action only exists when persistence is on, so the list is one
    // shorter without it. Keeping this count exact matters because update()
    // wraps the selection around using NUM_ACTIONS.
    static const int NUM_ACTIONS =
        #ifdef ENABLE_PERSISTENCE
        8;  // Feed, Play, Sleep, Bathe, Heal, Drink, Save, Back
        #else
        7;  // Feed, Play, Sleep, Bathe, Heal, Drink, Back
        #endif
    Action actions[NUM_ACTIONS];
    int currentActionIndex;

public:
    ActionMenu();

    // update() — call once per loop when the Interact screen is active.
    // Reads Button B (next) and Button C (previous) to cycle through actions.
    // Only call this when the Interact screen is visible — otherwise the
    // selection would change even while the user is on a different screen.
    void update(const ButtonHandler& buttons);

    // getSelectedAction() — returns the full Action struct for the highlighted entry.
    Action getSelectedAction() const;

    // getCurrentActionIndex() — returns the numeric position of the selected action (0–6).
    int getCurrentActionIndex() const;

    // isBackSelected() — returns true when the user has scrolled to "Back".
    // main.cpp uses this to switch screens instead of calling confirmAction().
    bool isBackSelected() const;

    // getRelevantStat() — returns which stat is affected by the selected action.
    // DisplayManager uses this to draw the correct contextual bar on the Interact screen.
    RelevantStat getRelevantStat() const;

    // confirmAction() — executes the selected action and plays the matching sound.
    // Only call this when isBackSelected() is false — Back is handled separately.
    // The two trailing parameters only exist when their feature is switched on:
    // the speaker when sound is on, and the storage when persistence is on (the
    // Save action writes the pet directly to NVS).
    void confirmAction(Pet& pet, DisplayManager& display
        #ifdef ENABLE_SOUND
        , SpeakerManager& speaker
        #endif
        #ifdef ENABLE_PERSISTENCE
        , StorageManager& storage
        #endif
        );
};

#endif
