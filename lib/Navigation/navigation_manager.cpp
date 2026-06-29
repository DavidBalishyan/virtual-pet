#include "navigation_manager.h"
#include <Arduino.h>

NavigationManager::NavigationManager()
    : currentScreen(SCREEN_MAIN),
      confirmActionRequested(false) {
    // Start on the Main screen.
}

void NavigationManager::update(const ButtonHandler& buttons, bool backSelected) {
    // Reset the confirm flag at the start of every frame so that it is only
    // true for the single loop iteration in which the user pressed A.
    // If we did not reset it here, confirmAction() would fire every frame
    // after the first press — feeding the pet repeatedly without extra presses.
    confirmActionRequested = false;

    switch (currentScreen) {
        case SCREEN_MAIN:
            handleMainScreenInput(buttons);
            break;
        #ifdef ENABLE_MULTISCREEN
        case SCREEN_STATS:
            handleStatsScreenInput(buttons);
            break;
        #endif
        #ifdef ENABLE_ACTION_MENU
        case SCREEN_INTERACT:
            handleInteractScreenInput(buttons, backSelected);
            break;
        #endif
    }
}

void NavigationManager::handleMainScreenInput(const ButtonHandler& buttons) {
    // From Main, a single button press jumps straight to another screen — no
    // intermediate selection step. Each destination is gated by its feature, so
    // a button does nothing when the screen it leads to is switched off.
    #ifdef ENABLE_ACTION_MENU
    // B opens the Interact screen (the action menu).
    if (buttons.wasButtonBPressed()) {
        currentScreen = SCREEN_INTERACT;
    }
    #endif
    #ifdef ENABLE_MULTISCREEN
    // C opens the Stats screen.
    if (buttons.wasButtonCPressed()) {
        currentScreen = SCREEN_STATS;
    }
    #endif
}

#ifdef ENABLE_MULTISCREEN
void NavigationManager::handleStatsScreenInput(const ButtonHandler& buttons) {
    // Stats is a read-only screen — any button press returns to Main.
    if (buttons.wasButtonBPressed() || buttons.wasButtonCPressed() || buttons.wasButtonAPressed()) {
        currentScreen = SCREEN_MAIN;
    }
}
#endif

#ifdef ENABLE_ACTION_MENU
void NavigationManager::handleInteractScreenInput(const ButtonHandler& buttons, bool backSelected) {
    // B and C are NOT handled here — loop() calls menu.update(buttons) first
    // so they cycle through actions rather than triggering a screen switch.
    // This handler only cares about A: either go back to Main or confirm an action.
    if (buttons.wasButtonAPressed()) {
        if (backSelected) {
            currentScreen = SCREEN_MAIN;
        } else {
            confirmActionRequested = true;
        }
    }
}
#endif

ScreenState NavigationManager::getCurrentScreen() const {
    return currentScreen;
}

bool NavigationManager::shouldConfirmAction() const {
    return confirmActionRequested;
}
