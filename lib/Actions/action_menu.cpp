#include "action_menu.h"

ActionMenu::ActionMenu() : currentActionIndex(0) {
    // Each entry carries the action type, display name, description, and
    // which pet stat it affects. The relevantStat field lets the Interact
    // screen highlight the correct bar without needing a separate lookup.
    actions[0] = {ACTION_FEED,  "Feed",  "Give food to pet",    STAT_FULLNESS};
    actions[1] = {ACTION_PLAY,  "Play",  "Play with pet",        STAT_HAPPINESS};
    actions[2] = {ACTION_SLEEP, "Sleep", "Let pet rest",         STAT_ENERGY};
    actions[3] = {ACTION_BATHE, "Bathe", "Clean the pet",        STAT_CLEANLINESS};
    actions[4] = {ACTION_HEAL,  "Heal",  "Treat pet illness",    STAT_SICKNESS};
    // Save only appears when persistence is on. Without it, Back moves up to
    // index 5 so the array stays packed (no empty gap) and matches NUM_ACTIONS.
    #ifdef ENABLE_PERSISTENCE
    actions[5] = {ACTION_SAVE,  "Save",  "Save pet progress",    STAT_NONE};
    actions[6] = {ACTION_BACK,  "Back",  "Return to main screen",STAT_NONE};
    #else
    actions[5] = {ACTION_BACK,  "Back",  "Return to main screen",STAT_NONE};
    #endif
}

void ActionMenu::update(const ButtonHandler& buttons) {
    // Button C: Previous action
    if (buttons.wasButtonCPressed()) {
        currentActionIndex--;
        if (currentActionIndex < 0) {
            currentActionIndex = NUM_ACTIONS - 1;  // Wrap around to last
        }
    }

    // Button B: Next action
    if (buttons.wasButtonBPressed()) {
        currentActionIndex++;
        if (currentActionIndex >= NUM_ACTIONS) {
            currentActionIndex = 0;  // Wrap around to first
        }
    }
}

Action ActionMenu::getSelectedAction() const {
    return actions[currentActionIndex];
}

int ActionMenu::getCurrentActionIndex() const {
    return currentActionIndex;
}

bool ActionMenu::isBackSelected() const {
    // Returns true when the user has scrolled to the Back entry.
    // main.cpp checks this before calling confirmAction() so the navigation
    // manager can switch screens without executing a pet action.
    return actions[currentActionIndex].type == ACTION_BACK;
}

RelevantStat ActionMenu::getRelevantStat() const {
    // Returns which stat the currently selected action affects.
    // DisplayManager calls this to know which bar to highlight on the Interact screen.
    return actions[currentActionIndex].relevantStat;
}

void ActionMenu::confirmAction(Pet& pet, DisplayManager& display
    #ifdef ENABLE_SOUND
    , SpeakerManager& speaker
    #endif
    #ifdef ENABLE_PERSISTENCE
    , StorageManager& storage
    #endif
    ) {
    Action selectedAction = getSelectedAction();

    // Each case calls the matching Pet method directly and, when sound is on,
    // plays its sound. Save and Back are special cases: Save writes to storage
    // instead of changing a pet stat, and Back is normally handled by
    // NavigationManager before we get here.
    switch (selectedAction.type) {
        case ACTION_FEED:
            pet.feed();
            #ifdef ENABLE_SOUND
            speaker.playFeedSound();
            #endif
            break;
        case ACTION_PLAY:
            pet.play();
            #ifdef ENABLE_SOUND
            speaker.playPlaySound();
            #endif
            break;
        case ACTION_SLEEP:
            pet.sleep();
            #ifdef ENABLE_SOUND
            speaker.playSleepSound();
            #endif
            break;
        case ACTION_BATHE:
            pet.bathe();
            #ifdef ENABLE_SOUND
            speaker.playBatheSound();
            #endif
            break;
        case ACTION_HEAL:
            pet.heal();
            #ifdef ENABLE_SOUND
            speaker.playHealSound();
            #endif
            break;
        #ifdef ENABLE_PERSISTENCE
        case ACTION_SAVE:
            storage.save(pet);
            #ifdef ENABLE_SOUND
            speaker.playSaveSound();
            #endif
            break;
        #endif
        case ACTION_BACK:
            // Back is handled by the NavigationManager before confirmAction() is called.
            // If we somehow reach here, do nothing — there is no pet action to run.
            return;
    }
}
