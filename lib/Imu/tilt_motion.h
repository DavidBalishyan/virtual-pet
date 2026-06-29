#ifndef TILT_MOTION_H
#define TILT_MOTION_H

// ---------------------------------------------------------------
// TiltMotion
//
// Turns the device's physical tilt into a smoothed, steady (x, y) value.
// It is pure float maths with NO display or M5 dependencies — which is why it
// lives in lib/Imu/ next to the raw accelerometer reader, not in Display: the
// smoothed motion it produces can drive ANY interaction, not just the screen.
// Its first use is the pet-sprite offset (DisplayManager adds the (x, y) to the
// sprite's normal centre), but you could just as well use the same value to,
// say, trigger an action on a strong tilt.
//
// This is the position counterpart to AnimationManager: AnimationManager
// decides WHICH frame to show over time, TiltMotion decides WHERE to show
// it based on tilt. They compose — the pet bounces (animation) AND glides
// (tilt) at the same time.
//
// Two ideas live in here:
//
//   1. A low-pass filter (smoothing). Raw accelerometer data is noisy, so
//      drawing the pet at the raw value would make it jitter. Instead of
//      snapping straight to each new reading, we ease toward it a little
//      every frame:  smoothed += (target - smoothed) * SMOOTHING_FACTOR.
//      That is the same "move a little closer each tick" idea as the
//      millis() timers, applied to position instead of time.
//
//   2. A clamp. A hard tilt could push the pet right off the screen, so we
//      cap the offset to a maximum number of pixels in each direction. The
//      pet always stays visible.
//
// There are no M5 or display dependencies here — it is pure float maths, so
// it is easy to read, easy to test in your head, and easy to reuse.
//
// How to use it:
//   1. Create one TiltMotion:  TiltMotion spriteMotion;
//   2. Each loop, after reading the IMU, feed it the live tilt:
//        spriteMotion.update(imu.getAccelX(), imu.getAccelY());
//   3. Draw the sprite at its normal centre PLUS the offset:
//        spriteMotion.getOffsetX(), spriteMotion.getOffsetY().
// ---------------------------------------------------------------
class TiltMotion {
public:
    // How strongly we ease toward each new tilt reading, between 0 and 1.
    // This is the low-pass filter strength: 0 would freeze the pet (it never
    // moves), 1 would snap instantly to the raw reading (no smoothing, lots of
    // jitter). 0.2 means "close 20% of the remaining gap each frame" — smooth
    // and calm, with just a little lag. Turn it up for a snappier feel.
    static constexpr float SMOOTHING_FACTOR = 0.2f;

    // Converts tilt (in g-force) into screen pixels. When the device is tilted
    // fully on its side, that axis reads about 1.0 G, so a scale of 60 would ask
    // for a 60-pixel slide at full tilt (before clamping). This is the main
    // sensitivity knob — bigger means the pet moves further for the same tilt.
    static constexpr float TILT_SCALE = 60.0f;

    // The furthest the pet may slide from centre, in pixels, on each axis.
    // The sprite is 80 px wide on a 135 px screen, leaving (135 - 80) / 2 = 27 px
    // of slack on each side, so 25 keeps it just inside the edge. Vertically the
    // title and mood text crowd the sprite more than the screen edge does, so we
    // keep the vertical travel a little smaller.
    static constexpr int MAX_OFFSET_X = 25;
    static constexpr int MAX_OFFSET_Y = 20;

    // Constructor — start the pet centred (zero offset) so it does not jump on
    // the first frame before any smoothing has happened.
    TiltMotion();

    // update() — call once per loop with the latest accelerometer X and Y.
    // Scales the tilt to pixels, clamps it so the pet stays on screen, then eases
    // the stored offset toward that clamped target. Never blocks (no delay()).
    void update(float accelX, float accelY);

    // getOffsetX() — the horizontal pixel offset to add to the sprite's centre
    // right now, rounded to a whole pixel.
    int getOffsetX() const;

    // getOffsetY() — the vertical pixel offset to add to the sprite's centre
    // right now, rounded to a whole pixel.
    int getOffsetY() const;

private:
    // The current smoothed offset, kept as floats so the easing maths does not
    // lose precision to rounding every frame. getOffsetX/Y() round these to
    // whole pixels only when handing them out.
    float smoothedX;
    float smoothedY;
};

#endif
