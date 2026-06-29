#include <Arduino.h>   // for constrain() and round(); also required for all lib/ .cpp files
#include "tilt_motion.h"

// Constructor — start the pet centred. Both smoothed offsets begin at 0 so the
// pet sits exactly where it does today until tilt data starts easing it around.
TiltMotion::TiltMotion()
    : smoothedX(0.0f),
      smoothedY(0.0f) {
}

// update() — turn this frame's tilt into a smoothed, on-screen pixel offset.
// Called once per loop with the latest accelerometer readings. The steps are:
// scale the raw tilt to pixels, clamp it so the pet cannot leave the screen,
// then ease the stored offset toward that clamped target (the low-pass filter).
void TiltMotion::update(float accelX, float accelY) {
    // Step 1 & 2 — scale tilt (g-force) to pixels, then clamp.
    // constrain(value, low, high) is an Arduino helper that returns value if it
    // is within range, or the nearest limit if it is outside. Clamping the
    // TARGET (not the smoothed result) means even a violent tilt can only ever
    // ask the pet to slide as far as MAX_OFFSET, so it always stays visible.
    float targetX = constrain(accelX * TILT_SCALE, -MAX_OFFSET_X, MAX_OFFSET_X);
    float targetY = constrain(accelY * TILT_SCALE, -MAX_OFFSET_Y, MAX_OFFSET_Y);

    // Step 3 — the low-pass filter. Instead of snapping to the target, move a
    // fraction (SMOOTHING_FACTOR) of the remaining distance toward it each frame.
    // This is the same "ease a little closer every tick" idea the millis() timers
    // use, applied to position. It smooths out accelerometer noise so the pet
    // glides instead of jittering. At rest the tilt is ~0, so the target is ~0 and
    // the offset naturally eases back to centre on its own.
    smoothedX = smoothedX + (targetX - smoothedX) * SMOOTHING_FACTOR;
    smoothedY = smoothedY + (targetY - smoothedY) * SMOOTHING_FACTOR;
}

// getOffsetX() — the horizontal offset to draw at this frame, in whole pixels.
// We round here, at the moment we hand the value out, so the easing maths above
// keeps its full floating-point precision frame to frame.
int TiltMotion::getOffsetX() const {
    return (int) round(smoothedX);
}

// getOffsetY() — the vertical offset to draw at this frame, in whole pixels.
int TiltMotion::getOffsetY() const {
    return (int) round(smoothedY);
}
