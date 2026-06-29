#ifndef NAVIGATION_MANAGER_H
#define NAVIGATION_MANAGER_H

#include "../Config/scaffold_config.h"  // ENABLE_* feature switches — read this first.
#include "../Display/screen_layout.h"
#include "../Button/button_handler.h"

// NavigationManager owns which screen the user is currently viewing and
// handles all screen-switching input. By moving this logic into its own
// module, loop() stays short — it just delegates here instead of containing
// a growing switch statement that would need to change for every new screen.
//
// NavigationManager has no knowledge of ActionMenu. The one fact it needs
// from the menu — whether "Back" is currently highlighted — is passed in
// as a plain bool by the caller. This keeps the two modules independent.
class NavigationManager {
private:
    ScreenState currentScreen;
    bool confirmActionRequested;  // True for one loop iteration when A is pressed on Interact

    // One input handler per screen.
    // Each method only reads the buttons that are meaningful on that screen,
    // which makes it easy to reason about: open the handler, see exactly what
    // the buttons do on that screen, nothing more.
    void handleMainScreenInput(const ButtonHandler& buttons);
    #ifdef ENABLE_MULTISCREEN
    void handleStatsScreenInput(const ButtonHandler& buttons);
    #endif
    #ifdef ENABLE_ACTION_MENU
    void handleInteractScreenInput(const ButtonHandler& buttons, bool backSelected);
    #endif

public:
    NavigationManager();

    // update() — call once per loop. Resets the confirm flag, then routes
    // button input to the handler for whichever screen is currently active.
    // backSelected tells us whether the Interact-screen menu is currently
    // highlighting the "Back" entry, so handleInteractScreenInput knows
    // whether A means "go back to Main" or "confirm this action."
    void update(const ButtonHandler& buttons, bool backSelected);

    // getCurrentScreen() — returns the screen the user is currently viewing.
    ScreenState getCurrentScreen() const;

    // shouldConfirmAction() — returns true if the user pressed A on the Interact
    // screen and the selected action is NOT Back. loop() reads this once and calls
    // menu.confirmAction() when it is true.
    bool shouldConfirmAction() const;
};

#endif
