#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <M5StickCPlus2.h>

/**
 * ButtonHandler
 * 
 * Abstracts button input handling for the M5StickCPlus2 device.
 * Provides a simple polling-based interface for checking button states.
 * Currently supports three buttons: A (side), B (top), and C (power/middle)
 * 
 * Usage:
 *   ButtonHandler buttons;
 *   
 *   // In your loop:
 *   if (buttons.wasButtonAPressed()) {
 *       // Handle button A press
 *   }
 */
class ButtonHandler {
private:
    // GPIO pin for Button C — not exposed by the M5StickCPlus2 library so read directly
    static const int BUTTON_C_PIN = 35;

    // Button states from the previous frame — used to detect the moment a button first goes down
    bool prevButtonA;
    bool prevButtonB;
    bool prevButtonC;

    // Button states from the current frame — updated each call to update()
    bool currentButtonA;
    bool currentButtonB;
    bool currentButtonC;

public:
    // Constructor
    ButtonHandler();

    /**
     * update()
     * Must be called once per loop iteration.
     * Polls the current button states from the M5Stick hardware.
     */
    void update();

    /**
     * Query methods - detect if button was pressed in current cycle
     */
    bool wasButtonAPressed() const;
    bool wasButtonBPressed() const;
    bool wasButtonCPressed() const;

    /**
     * Current state methods - detect if button is currently held
     */
    bool isButtonAHeld() const;
    bool isButtonBHeld() const;
    bool isButtonCHeld() const;
};

#endif
