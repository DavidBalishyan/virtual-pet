#include <Arduino.h>
#include "animation_manager.h"

// Constructor — store the sprite's frame count and per-frame duration, and
// start on frame 0. lastFrameAdvanceTime starts at 0 so the first update()
// advances straight away (the same "fire immediately" trick TimerManager uses).
AnimationManager::AnimationManager(int frameCount, unsigned long frameDurationMs)
    : frameCount(frameCount),
      frameDurationMs(frameDurationMs),
      currentFrame(0),
      lastFrameAdvanceTime(0) {
}

// update() — advance to the next frame once enough time has passed.
// Called once per loop(). Uses millis() instead of delay() so the rest of the
// program keeps running while the sprite animates.
void AnimationManager::update() {
    // A still image (one frame, or a misconfigured zero) has nothing to cycle
    // through. Returning here also avoids dividing by zero in the wrap below.
    if (frameCount <= 1) {
        return;
    }

    unsigned long currentTime = millis();

    if (currentTime - lastFrameAdvanceTime >= frameDurationMs) {
        // Step to the next frame, wrapping back to 0 after the last one so the
        // animation loops forever.
        currentFrame = (currentFrame + 1) % frameCount;
        lastFrameAdvanceTime = currentTime;
    }
}

// getCurrentFrame() — the frame index DisplayManager should draw this frame.
int AnimationManager::getCurrentFrame() const {
    return currentFrame;
}

// reset() — restart the animation from the first frame.
// Called when the sprite should visibly start over, for example after
// switching screens, so the pet does not appear to resume mid-bounce.
void AnimationManager::reset() {
    currentFrame = 0;
    lastFrameAdvanceTime = millis();
}
