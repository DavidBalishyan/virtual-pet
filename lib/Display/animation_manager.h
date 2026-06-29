#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

// ---------------------------------------------------------------
// AnimationManager
//
// Works out WHICH frame of a multi-frame sprite should be on screen
// right now. It does not draw anything itself — DisplayManager asks it
// for the current frame number and draws that frame. Keeping the
// "which frame?" timing here, separate from the "how do I draw it?"
// code in DisplayManager, means each class still has exactly one job.
//
// It uses the same non-blocking millis() timer pattern as TimerManager:
// every loop() we check how much time has passed and only advance the
// frame when enough has. We never call delay(), so buttons, sound, and
// the rest of the loop keep running smoothly while the pet animates.
//
// How to use it:
//   1. Create one AnimationManager, telling it how many frames the
//      sprite has:  AnimationManager petAnimation(FRAME_COUNT);
//   2. Call petAnimation.update() once per loop().
//   3. Draw sprite frame petAnimation.getCurrentFrame().
//   4. Call petAnimation.reset() when the animation should restart from
//      the first frame (for example, when switching screens).
// ---------------------------------------------------------------
class AnimationManager {
public:
    // How long each frame stays on screen, in milliseconds.
    // 200 ms = 5 frames per second — slow enough to read clearly on a
    // small screen, and a gentle, kid-friendly speed to start from.
    static const unsigned long FRAME_DURATION_MS = 200;

    // Constructor.
    //   frameCount      — how many frames the sprite has (1 = a still image).
    //   frameDurationMs — how long each frame is shown; defaults to
    //                     FRAME_DURATION_MS (5 fps).
    AnimationManager(int frameCount, unsigned long frameDurationMs = FRAME_DURATION_MS);

    // update() — call once per loop(). Advances to the next frame if
    // frameDurationMs has passed since the last advance. Does nothing for a
    // single-frame sprite (there is nothing to cycle through).
    void update();

    // getCurrentFrame() — returns the frame index to draw right now,
    // always in the range 0 .. frameCount - 1.
    int getCurrentFrame() const;

    // reset() — jump back to the first frame and restart the timer, so an
    // animation begins cleanly from frame 0 instead of mid-cycle.
    void reset();

private:
    // Total number of frames in the sprite being animated.
    int frameCount;

    // How long each frame stays on screen before advancing, in milliseconds.
    unsigned long frameDurationMs;

    // The frame index currently being shown (0 .. frameCount - 1).
    int currentFrame;

    // The millis() timestamp when we last advanced the frame. Comparing
    // millis() against this is how we time the next advance without ever
    // blocking the loop.
    unsigned long lastFrameAdvanceTime;
};

#endif
