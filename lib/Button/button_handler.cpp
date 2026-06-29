#include "button_handler.h"

ButtonHandler::ButtonHandler()
    : prevButtonA(false),    prevButtonB(false),    prevButtonC(false),
      currentButtonA(false), currentButtonB(false), currentButtonC(false) {
    // GPIO 35 is input-only on the ESP32 so INPUT is the only valid mode.
    // BtnA and BtnB are handled by the M5 library; BtnC must be set up manually.
    pinMode(BUTTON_C_PIN, INPUT);
}

// update()
// Shifts this frame's button states into the previous-frame slots, then reads
// fresh hardware state. Must be called exactly once at the top of loop() so
// that every wasButtonXPressed() query in the same frame sees the same snapshot.
void ButtonHandler::update() {
    prevButtonA = currentButtonA;
    prevButtonB = currentButtonB;
    prevButtonC = currentButtonC;

    currentButtonA = M5.BtnA.isPressed();
    currentButtonB = M5.BtnB.isPressed();
    // GPIO 35 is active-low: the pin reads LOW when the button is physically pressed
    currentButtonC = (digitalRead(BUTTON_C_PIN) == LOW);
}

// wasButtonAPressed()
// Returns true only on the single frame when the button first goes down —
// current frame is pressed AND the previous frame was not.
bool ButtonHandler::wasButtonAPressed() const {
    return currentButtonA && !prevButtonA;
}

// wasButtonBPressed()
// Returns true only on the single frame when Button B first goes down.
bool ButtonHandler::wasButtonBPressed() const {
    return currentButtonB && !prevButtonB;
}

// wasButtonCPressed()
// Returns true only on the single frame when Button C first goes down.
bool ButtonHandler::wasButtonCPressed() const {
    return currentButtonC && !prevButtonC;
}

// isButtonAHeld()
// Returns true every frame the button is held down, not just the first frame.
bool ButtonHandler::isButtonAHeld() const {
    return currentButtonA;
}

// isButtonBHeld()
// Returns true every frame Button B is held down.
bool ButtonHandler::isButtonBHeld() const {
    return currentButtonB;
}

// isButtonCHeld()
// Returns true every frame Button C is held down.
bool ButtonHandler::isButtonCHeld() const {
    return currentButtonC;
}
