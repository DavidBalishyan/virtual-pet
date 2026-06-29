#include "display_manager.h"

DisplayManager::DisplayManager()
    : lastRenderedScreen(SCREEN_MAIN),
      petWasDeadLastFrame(false) {
}

void DisplayManager::init() {
    // Match the canvas pixel format to the LCD: 16-bit RGB565, two bytes per
    // pixel. This makes the off-screen buffer the same format as the screen,
    // so pushing it across is a straight copy, and fixes the buffer size at
    // 135 x 240 x 2 = ~63 KB.
    canvas.setColorDepth(16);

    // Allocate the off-screen canvas. This must happen here, after M5.begin()
    // has initialised the LCD, rather than in the constructor. createSprite()
    // returns the buffer pointer, or nullptr if there was not enough heap for
    // the ~63 KB buffer — we log that case so a failure is visible on serial
    // rather than showing up as a mysteriously blank screen.
    if (canvas.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT) == nullptr) {
        Serial.println("ERROR: could not allocate display canvas");
    }

    // Byte-swap note: the sprite pixels in lib/Display/sprites/ are pre-swapped
    // to match the order the LCD expects from a direct pushImage(). Drawing them
    // into this canvas and then pushing the canvas may apply the swap a second
    // time. If the pet's colours look wrong on the device (but the text/bars are
    // fine, or vice versa), uncomment the line below so the canvas swaps the
    // sprite bytes back as they are drawn in:
    //     canvas.setSwapBytes(true);

    clearScreen(TFT_BLACK);
    pushCanvas();
}

// pushCanvas() — copies the finished off-screen frame to the LCD in one shot.
// The canvas was constructed with &M5.Lcd as its parent, so pushSprite() knows
// where to send the pixels.
void DisplayManager::pushCanvas() {
    canvas.pushSprite(0, 0);
}

void DisplayManager::clearScreen(uint32_t color) {
    canvas.fillScreen(color);
}

void DisplayManager::fillRect(int x, int y, int width, int height, uint32_t color) {
    canvas.fillRect(x, y, width, height, color);
}

// renderDisplay() — the one call loop() makes every frame.
// It decides whether to show the death screen or one of the three live screens,
// then delegates to the appropriate private render method.
//
// Action-menu information arrives as three primitives (name, relevant stat,
// current index) rather than as an ActionMenu reference. This keeps
// DisplayManager unaware of ActionMenu — the caller does the extraction.
void DisplayManager::renderDisplay(int happiness, int fullness, int energy, int cleanliness,
                                   int sick, MoodSprite mood, const char* selectedActionName,
                                   RelevantStat relevantStat,
                                   bool petIsDead, const char* petName,
                                   ScreenState screenState,
                                   int spriteOffsetX, int spriteOffsetY) {
    // --- Death state ---
    // Draw the death screen once when the pet first dies, then hold it.
    // The petWasDeadLastFrame flag prevents redrawing the death screen every frame.
    if (petIsDead) {
        if (!petWasDeadLastFrame) {
            petWasDeadLastFrame = true;
            showDeathScreen();
        }
        return;
    }

    // --- Revival: coming back from dead ---
    // The pet was dead last frame but is alive now, so it has just been reset.
    // Clear the flag; the screen redraws this frame like any other — there is no
    // throttle to nudge anymore now that every frame redraws.
    if (petWasDeadLastFrame) {
        petWasDeadLastFrame = false;
    }

    // Restart the sprite animation from frame 0 whenever the screen changes, so
    // the pet does not appear to resume mid-bounce after switching screens.
    if (screenState != lastRenderedScreen) {
        petAnimation.reset();
        lastRenderedScreen = screenState;
    }

    // Advance the animation if enough time has passed. This is non-blocking:
    // on most loops it does nothing and returns immediately.
    petAnimation.update();

    // --- Normal alive render ---
    // Draw the whole screen fresh to the off-screen canvas, then push it to the
    // LCD in one shot. Redrawing every loop is what makes the sprite animate and
    // stat changes appear instantly; the single push keeps it flicker-free.
    switch (screenState) {
        case SCREEN_MAIN:
            renderMainScreen(fullness, mood, petName, spriteOffsetX, spriteOffsetY);
            break;
        #ifdef ENABLE_MULTISCREEN
        case SCREEN_STATS:
            renderStatsScreen(happiness, fullness, energy, cleanliness, sick, mood, petName);
            break;
        #endif
        #ifdef ENABLE_ACTION_MENU
        case SCREEN_INTERACT:
            renderInteractScreen(happiness, fullness, energy, cleanliness, sick, mood,
                                 selectedActionName, relevantStat, petName,
                                 spriteOffsetX, spriteOffsetY);
            break;
        #endif
    }

    pushCanvas();
}

// -----------------------------------------------------------------------
// Main screen render
// Shows the pet face filling the centre of the screen, with a two-tab nav
// bar at the bottom so the user can enter Stats or Interact.
// -----------------------------------------------------------------------
void DisplayManager::renderMainScreen(int fullness, MoodSprite mood, const char* petName,
                                      int spriteOffsetX, int spriteOffsetY) {
    clearScreen();
    printCenteredText(petName, TITLE_ZONE.y, TFT_YELLOW, 2);
    drawPetSprite(mood, MAIN_FACE_CENTER_Y, SPRITE_NEUTRAL_PLACEHOLDER_WIDTH, SPRITE_NEUTRAL_PLACEHOLDER_HEIGHT,
                  spriteOffsetX, spriteOffsetY);
    showPetMoodText(mood, MAIN_MOOD_Y);
    // Only show the fullness bar on the Main screen when there is no action menu.
    // Once ENABLE_ACTION_MENU is on, the Interact screen shows the stat bars, so a
    // bar here would be redundant — the Main screen is just the pet's face again.
    #ifndef ENABLE_ACTION_MENU
    drawMainFullnessBar(fullness);
    #endif
    drawMainNavBar();
}

// drawMainFullnessBar() — draws a single labelled fullness bar on the Main screen.
// The Main screen otherwise shows only the pet's face, so this is the one stat
// the user can watch at a glance: it drops as the pet gets hungrier and is the
// bar the Session 1 dials (starting fullness value and decay speed) visibly
// change. It reuses drawStatusBar(), the same primitive the Stats and Interact
// screens use, so every bar in the project is drawn the same way.
void DisplayManager::drawMainFullnessBar(int fullness) {
    // Clear the band first so the previous frame's bar does not bleed through.
    fillRect(MAIN_STAT_ZONE.x, MAIN_STAT_ZONE.y,
             MAIN_STAT_ZONE.width, MAIN_STAT_ZONE.height, TFT_BLACK);

    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE);
    canvas.setCursor(MAIN_STAT_ZONE.x, MAIN_FULLNESS_LABEL_Y);
    canvas.printf("Fullness: %d", fullness);
    drawStatusBar(fullness, 100, MAIN_STAT_ZONE.x, MAIN_FULLNESS_BAR_Y, MAIN_STAT_ZONE.width, TFT_RED);
}

// drawMainNavBar()
// Draws the two-tab bar at the bottom of the Main screen. The nav bar is the
// user's entry point into the rest of the app — without it the user would
// have no visible cue that B and C take them to the Interact and Stats
// screens. Both tabs are drawn the same way (no highlight state) because
// the Main screen has no concept of "selected tab"; a single press jumps
// straight to the chosen screen.
void DisplayManager::drawMainNavBar() {
    // Each tab is gated by the screen it leads to, so the bar is empty when
    // neither destination exists (Session 1). The outer guard keeps tabWidth
    // from being an unused variable in that case.
    #if defined(ENABLE_MULTISCREEN) || defined(ENABLE_ACTION_MENU)
    int tabWidth = MAIN_NAV_ZONE.width / 2;

    #ifdef ENABLE_MULTISCREEN
    // Left tab — Stats
    canvas.drawRect(MAIN_NAV_ZONE.x, MAIN_NAV_ZONE.y, tabWidth, MAIN_NAV_ZONE.height, TFT_CYAN);
    printText("Stats", MAIN_NAV_ZONE.x + 6, MAIN_NAV_ZONE.y + 5, TFT_CYAN, 1);
    #endif

    #ifdef ENABLE_ACTION_MENU
    // Right tab — Interact
    int rightX = MAIN_NAV_ZONE.x + tabWidth;
    canvas.drawRect(rightX, MAIN_NAV_ZONE.y, tabWidth, MAIN_NAV_ZONE.height, TFT_CYAN);
    printText("Interact", rightX + 3, MAIN_NAV_ZONE.y + 5, TFT_CYAN, 1);
    #endif
    #endif
}

// -----------------------------------------------------------------------
// Stats screen render
// Preserves the original layout exactly: stat bars at the top, face below.
// A "B/C: Back" hint replaces the old action menu indicator at the bottom.
// -----------------------------------------------------------------------
#ifdef ENABLE_MULTISCREEN
void DisplayManager::renderStatsScreen(int happiness, int fullness, int energy, int cleanliness,
                                       int sick, MoodSprite mood, const char* petName) {
    clearScreen();
    showPetStatus(happiness, fullness, energy, cleanliness, sick, petName);
    showPetMood(mood);

    // Back hint at the bottom instead of the action menu indicator
    canvas.drawRect(MENU_ZONE.x, MENU_ZONE.y, MENU_ZONE.width, MENU_ZONE.height, TFT_CYAN);
    printText("B/C: Back", MENU_ZONE.x + 10, MENU_ZONE.y + 4, TFT_CYAN, 1);
}
#endif

// -----------------------------------------------------------------------
// Interact screen render
// Shows the pet face in the top half, a single contextual stat bar in the
// middle (the stat the selected action affects), and the action menu
// indicator at the very bottom — the same indicator style as before.
// -----------------------------------------------------------------------
#ifdef ENABLE_ACTION_MENU
void DisplayManager::renderInteractScreen(int happiness, int fullness, int energy, int cleanliness,
                                          int sick, MoodSprite mood, const char* selectedActionName,
                                          RelevantStat relevantStat, const char* petName,
                                          int spriteOffsetX, int spriteOffsetY) {
    clearScreen();
    printCenteredText(petName, TITLE_ZONE.y, TFT_YELLOW, 2);
    drawPetSprite(mood, INTERACT_FACE_CENTER_Y, SPRITE_NEUTRAL_PLACEHOLDER_WIDTH, SPRITE_NEUTRAL_PLACEHOLDER_HEIGHT,
                  spriteOffsetX, spriteOffsetY);
    showPetMoodText(mood, INTERACT_MOOD_Y);
    drawContextualStatBar(happiness, fullness, energy, cleanliness, sick, relevantStat);
    drawMenuIndicator(selectedActionName, MENU_ZONE.x, MENU_ZONE.y);
}
#endif

#ifdef ENABLE_ACTION_MENU
// drawContextualStatBar()
// Draws a single stat bar in the INTERACT_STAT_ZONE area — only the one stat
// that the currently selected action affects. The label, current value, and
// colour all come from the relevantStat parameter, so the bar changes as the
// user scrolls through actions.
//
// Why a single stat instead of all five? The Interact screen exists for the
// user to choose an action and see its consequence. Showing only the stat
// that will change keeps the focus on the cause-and-effect link between the
// selected action and the pet's response.
void DisplayManager::drawContextualStatBar(int happiness, int fullness, int energy,
                                           int cleanliness, int sick, RelevantStat relevantStat) {
    // Clear the area first so the previous bar does not bleed through
    fillRect(INTERACT_STAT_ZONE.x, INTERACT_STAT_ZONE.y,
             INTERACT_STAT_ZONE.width, INTERACT_STAT_ZONE.height, TFT_BLACK);

    // Save and Back have no specific stat — leave the area blank
    if (relevantStat == STAT_NONE) {
        return;
    }

    const char* label;
    int         value;
    uint32_t    color;

    switch (relevantStat) {
        case STAT_HAPPINESS:  label = "Happy";  value = happiness;  color = TFT_GREEN;  break;
        case STAT_FULLNESS:     label = "Fullness"; value = fullness;     color = TFT_RED;    break;
        case STAT_ENERGY:     label = "Energy"; value = energy;     color = TFT_BLUE;   break;
        case STAT_CLEANLINESS:label = "Clean";  value = cleanliness;color = TFT_CYAN;   break;
        case STAT_SICKNESS:   label = "Sick";   value = sick;       color = TFT_PURPLE; break;
        default:              return;
    }

    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE);
    canvas.setCursor(INTERACT_STAT_ZONE.x, INTERACT_STAT_LABEL_Y);
    canvas.printf("%s: %d", label, value);
    drawStatusBar(value, 100, INTERACT_STAT_ZONE.x, INTERACT_STAT_BAR_Y, INTERACT_STAT_ZONE.width, color);
}
#endif

// -----------------------------------------------------------------------
// Shared helpers used by more than one render method
// -----------------------------------------------------------------------

#ifdef ENABLE_ACTION_MENU
// drawMenuIndicator() — the compact action name box at the very bottom.
// Used on the Interact screen. Clears its area before drawing.
// Takes the action name as a plain string — no ActionMenu reference needed —
// so this helper has no knowledge of the menu's internals.
void DisplayManager::drawMenuIndicator(const char* selectedActionName, int x, int y) {
    fillRect(x, y, MENU_ZONE.width, MENU_ZONE.height, TFT_BLACK);
    canvas.drawRect(x, y, MENU_ZONE.width, MENU_ZONE.height, TFT_CYAN);
    printText("Action: ", x + 2, y + 4, TFT_CYAN, 1);
    printText(selectedActionName, x + 50, y + 4, TFT_YELLOW, 1);
}
#endif

// showPetMoodText() — draws just the mood label at the given Y position.
// Separated from drawPetSprite() so the Main and Interact screens can place
// the text at different heights without calling the full showPetMood() path.
void DisplayManager::showPetMoodText(MoodSprite mood, int textY) {
    const char* moodText;
    uint32_t    moodColor;

    // One label and colour per mood. The cases use the named MoodSprite values
    // (not raw numbers), so this switch reads as the same four moods that
    // Pet::computeMood() returns — the word under the pet always matches its face.
    switch (mood) {
        #ifdef ENABLE_MOOD_SPRITES
        case MOOD_HAPPY:  moodText = "Happy";   moodColor = TFT_GREEN;  break;
        case MOOD_UNWELL: moodText = "Unwell";  moodColor = TFT_PURPLE; break;
        case MOOD_HUNGRY: moodText = "Hungry";  moodColor = TFT_RED;    break;
        #endif
        case MOOD_NEUTRAL:
        default:          moodText = "Neutral"; moodColor = TFT_WHITE;  break;
    }

    printCenteredText(moodText, textY, moodColor, 2);
}

#ifdef ENABLE_MULTISCREEN
// showPetMood() — draws the mood label at the Stats screen position.
// The Stats screen is a pure data view: five stat bars and this mood word,
// with no pet sprite. Used by renderStatsScreen().
void DisplayManager::showPetMood(MoodSprite mood) {
    showPetMoodText(mood, MOOD_ZONE.y);
}

// showPetStatus() — draws all five labelled stat bars using the Stats screen zones.
// Does not clear the screen — the caller must do that first.
void DisplayManager::showPetStatus(int happiness, int fullness, int energy, int cleanliness,
                                   int sick, const char* petName) {
    printCenteredText(petName, TITLE_ZONE.y, TFT_YELLOW, 2);

    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE);

    canvas.setCursor(STATS_ZONE.x, HAPPY_BAR_ZONE.labelY);
    canvas.printf("Happy: %d", happiness);
    drawStatusBar(happiness, 100, STATS_ZONE.x, HAPPY_BAR_ZONE.barY, STATS_ZONE.width, TFT_GREEN);

    canvas.setCursor(STATS_ZONE.x, FULLNESS_BAR_ZONE.labelY);
    canvas.printf("Fullness: %d", fullness);
    drawStatusBar(fullness, 100, STATS_ZONE.x, FULLNESS_BAR_ZONE.barY, STATS_ZONE.width, TFT_RED);

    canvas.setCursor(STATS_ZONE.x, ENERGY_BAR_ZONE.labelY);
    canvas.printf("Energy: %d", energy);
    drawStatusBar(energy, 100, STATS_ZONE.x, ENERGY_BAR_ZONE.barY, STATS_ZONE.width, TFT_BLUE);

    canvas.setCursor(STATS_ZONE.x, CLEAN_BAR_ZONE.labelY);
    canvas.printf("Clean: %d", cleanliness);
    drawStatusBar(cleanliness, 100, STATS_ZONE.x, CLEAN_BAR_ZONE.barY, STATS_ZONE.width, TFT_CYAN);

    canvas.setCursor(STATS_ZONE.x, SICK_BAR_ZONE.labelY);
    canvas.printf("Sick: %d", sick);
    drawStatusBar(sick, 100, STATS_ZONE.x, SICK_BAR_ZONE.barY, STATS_ZONE.width, TFT_PURPLE);
}
#endif

// spriteForMood() — returns the pixel data to draw for a given mood and frame.
// This is the single place that maps a mood to its artwork. Each mood will have
// its own picture, so this switch is where a new mood gets connected to its
// sprite (one extra case).
//
// Each mood maps to its own picture. The placeholders are single-frame, so
// frame is always 0 for now, but it is threaded through so a future animated
// sprite (more than one frame) needs no change here.
const uint16_t* DisplayManager::spriteForMood(MoodSprite mood, int frame) {
    switch (mood) {
        #ifdef ENABLE_MOOD_SPRITES
        case MOOD_HAPPY:   return sprite_happy_placeholder[frame];
        case MOOD_UNWELL:  return sprite_unwell_placeholder[frame];
        case MOOD_HUNGRY:  return sprite_hungry_placeholder[frame];
        #endif
        case MOOD_NEUTRAL:
        default:           return sprite_neutral_placeholder[frame];
    }
}

// drawPetSprite() — draws the pet's face as a bitmap sprite.
// The sprite is centred horizontally on the screen, and its vertical centre
// is positioned at faceCenterY so each screen can choose where the face sits.
//
// How does it pick the picture?
// It asks spriteForMood() for the right artwork for the current mood, using the
// animation frame chosen by petAnimation. So the same call draws a different
// face as the pet's mood changes, and animates within that face.
//
// Why pass the dimensions in as parameters?
// Every screen draws the same 80x80 size, so width and height are the same at
// each call site. They are passed in rather than hard-coded so a future screen
// could supply a different size without changing this function.
//
// Why SPRITE_TRANSPARENT_COLOR?
// The sprite is a square block of pixels, but the pet's outline is not square.
// Pixels matching SPRITE_TRANSPARENT_COLOR (0x1FF8) are skipped by pushImage(),
// so the screen background shows through and the pet appears to "float"
// instead of sitting inside a coloured rectangle.
void DisplayManager::drawPetSprite(MoodSprite mood, int faceCenterY, int spriteWidth,
                                   int spriteHeight, int spriteOffsetX, int spriteOffsetY) {
    // Start from the normal centred position, then add the tilt offset. When the
    // offset is 0 (tilt disabled, or device held flat) this is exactly the old
    // centred position, so every screen looks unchanged. The offset is already
    // clamped by TiltMotion, so adding it here cannot push the pet off screen.
    int spriteX = (SCREEN_WIDTH - spriteWidth) / 2 + spriteOffsetX;
    int spriteY = faceCenterY - (spriteHeight / 2) + spriteOffsetY;

    // Pick the picture for this mood and the current animation frame, then draw it.
    const uint16_t* spriteData = spriteForMood(mood, petAnimation.getCurrentFrame());
    canvas.pushImage(spriteX, spriteY, spriteWidth, spriteHeight, spriteData, SPRITE_TRANSPARENT_COLOR);
}

// -----------------------------------------------------------------------
// Primitive drawing and text helpers
// -----------------------------------------------------------------------

void DisplayManager::printText(const char* text, int x, int y, uint32_t color, uint8_t size) {
    canvas.setCursor(x, y);
    canvas.setTextColor(color);
    canvas.setTextSize(size);
    canvas.print(text);
}

void DisplayManager::printCenteredText(const char* text, int y, uint32_t color, uint8_t size) {
    canvas.setTextColor(color);
    canvas.setTextSize(size);

    int textWidth = strlen(text) * 6 * size;
    int x = (SCREEN_WIDTH - textWidth) / 2;
    if (x < 0) { x = 0; }

    canvas.setCursor(x, y);
    canvas.print(text);
}

void DisplayManager::drawStatusBar(int value, int maxValue, int x, int y, int width, uint32_t color) {
    canvas.drawRect(x, y, width, STAT_BAR_HEIGHT, TFT_WHITE);

    int fillWidth = (value * width) / maxValue;
    if (fillWidth > 0) {
        canvas.fillRect(x, y, fillWidth, STAT_BAR_HEIGHT, color);
    }
}

void DisplayManager::showMessage(const char* message) {
    printCenteredText(message, 160, TFT_WHITE, 1);
    pushCanvas();
}

// showDeathScreen() — clears the screen and shows game-over text.
// Drawn once when the pet dies (see renderDisplay) and then held until the
// user restarts, at which point the normal screens redraw on the next frame.
void DisplayManager::showDeathScreen() {
    clearScreen(TFT_BLACK);

    printCenteredText("Your pet", 60, TFT_RED, 2);
    printCenteredText("has died!", 85, TFT_RED, 2);
    printCenteredText("Press A to", 130, TFT_WHITE, 2);
    printCenteredText("restart", 155, TFT_WHITE, 2);
    pushCanvas();
}
