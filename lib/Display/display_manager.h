#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <M5StickCPlus2.h>
#include "../Config/scaffold_config.h"  // ENABLE_* feature switches — read this first.
#include "screen_layout.h"
// One 80x80 sprite per mood. spriteForMood() maps a MoodSprite to the matching
// array, so the pet's face changes with how it feels. All four are the same
// size and (for now) a single still frame.
#include "sprites/neutral_placeholder.h"
#ifdef ENABLE_MOOD_SPRITES
#include "sprites/happy_placeholder.h"
#include "sprites/unwell_placeholder.h"
#include "sprites/hungry_placeholder.h"
#endif
#include "animation_manager.h"   // decides which sprite frame to draw each loop

// DisplayManager only deals in primitive values (ints, strings, enums).
// It never receives whole manager objects, so it does not need to know
// what an ActionMenu or a Pet is. Whoever calls renderDisplay is
// responsible for pulling the values out and passing them in.

class DisplayManager {
private:
    // Display dimensions for M5Stick C Plus 2
    static const int SCREEN_WIDTH  = 135;
    static const int SCREEN_HEIGHT = 240;

    // -----------------------------------------------------------------------
    // Stats screen zone constants. The Stats screen is a pure data view: five
    // stat bars at the top, then the mood word, then a Back hint at the bottom.
    // No pet sprite.
    // -----------------------------------------------------------------------
    static constexpr ScreenZone TITLE_ZONE    = {  0,   5, 135, 19 };
    static constexpr ScreenZone STATS_ZONE    = {  5,  26, 125, 130 };
    static constexpr ScreenZone MOOD_ZONE     = {  0, 180, 135, 18 };
    static constexpr ScreenZone MENU_ZONE     = {  5, 220, 125, 20 };

    // Per-stat bar positions within STATS_ZONE
    static constexpr StatBarZone HAPPY_BAR_ZONE  = {  26,  36 };
    static constexpr StatBarZone FULLNESS_BAR_ZONE = {  48,  58 };
    static constexpr StatBarZone ENERGY_BAR_ZONE = {  70,  80 };
    static constexpr StatBarZone CLEAN_BAR_ZONE    = {  92, 102 };
    static constexpr StatBarZone SICK_BAR_ZONE     = { 114, 124 };
    static constexpr StatBarZone HYDRATION_BAR_ZONE = { 136, 146 };

    // -----------------------------------------------------------------------
    // Main screen zone constants — no stat bars, so the face gets more room.
    // The bottom nav bar lets the user pick Stats or Interact.
    // -----------------------------------------------------------------------
    static constexpr int         MAIN_FACE_CENTER_Y = 110;
    static constexpr int         MAIN_MOOD_Y        = 155;
    // A single fullness bar on the Main screen, drawn in the band between the mood
    // word and the nav bar. This gives the pet one visible, at-a-glance stat on
    // the home screen — the bar the Session 1 dials (starting fullness and decay
    // speed) visibly change. Positions mirror the Interact contextual bar so the
    // bar sits in the same place on both screens.
    static constexpr int         MAIN_FULLNESS_LABEL_Y = 177;
    static constexpr int         MAIN_FULLNESS_BAR_Y   = 190;
    static constexpr ScreenZone  MAIN_STAT_ZONE       = {  5, 174, 125, 30 };
    // Nav bar shares the same Y/height as MENU_ZONE so the bottom bar does not
    // appear to move when switching between the Main, Stats, and Interact screens.
    static constexpr ScreenZone  MAIN_NAV_ZONE      = {  5, 220, 125, 20 };

    // -----------------------------------------------------------------------
    // Interact screen zone constants. The face and mood text are positioned to
    // match the Main screen exactly (MAIN_FACE_CENTER_Y / MAIN_MOOD_Y) so the
    // pet does not appear to jump when switching screens. The contextual stat
    // bar and the action menu indicator then fill the region below the mood.
    // -----------------------------------------------------------------------
    static constexpr int         INTERACT_FACE_CENTER_Y = 110;  // = MAIN_FACE_CENTER_Y
    static constexpr int         INTERACT_MOOD_Y        = 155;  // = MAIN_MOOD_Y
    static constexpr int         INTERACT_STAT_LABEL_Y  = 177;
    static constexpr int         INTERACT_STAT_BAR_Y    = 190;
    static constexpr ScreenZone  INTERACT_STAT_ZONE     = {  5, 174, 125, 30 };

    // Geometry constants shared across all screens
    static const int STAT_BAR_HEIGHT = 10;

    // State tracked between frames.
    // We now redraw the whole screen to the off-screen canvas every loop and
    // push it in one shot, so there is no longer any redraw throttle to manage.
    // lastRenderedScreen is still useful: it lets us spot when the user has
    // switched screens so the sprite animation can restart cleanly from frame 0.
    ScreenState  lastRenderedScreen;   // The screen drawn on the previous frame
    bool petWasDeadLastFrame;          // Draws the death screen once, then holds it

    // Off-screen double-buffer. Every drawing call below targets this canvas
    // instead of the LCD directly; the finished frame is then copied to the
    // screen in one shot by pushCanvas(). Drawing off-screen and pushing once
    // is what lets us redraw every loop — including the animated sprite —
    // without the screen ever flickering. The buffer is 135x240 16-bit pixels
    // (~63 KB), allocated once in init(). See Hardware Gotcha 3 in DEV_ROADMAP.md.
    M5Canvas canvas = M5Canvas(&M5.Lcd);

    // Times the pet sprite's frame cycling. Created with the sprite's frame
    // count so it knows how many frames to loop through; update() is called
    // each loop and getCurrentFrame() tells drawPetSprite() which frame to draw.
    // The mood placeholders are single-frame, so this is 1 for now: the animator
    // simply holds frame 0 (it does nothing for a 1-frame sprite). When the moods
    // gain a second frame, bump this to that frame count and the loop resumes.
    AnimationManager petAnimation = AnimationManager(SPRITE_NEUTRAL_PLACEHOLDER_FRAME_COUNT);

    // pushCanvas() — copies the finished off-screen frame to the LCD in one
    // operation. Called at the end of every render path so the screen only
    // ever updates as a complete frame.
    void pushCanvas();

    // -----------------------------------------------------------------------
    // Private render methods — one per screen.
    // Each owns the layout and redraw logic for its screen only.
    // -----------------------------------------------------------------------
    // spriteOffsetX/Y slide the pet sprite away from centre to follow the device
    // tilt. The Stats screen has no pet sprite, so renderStatsScreen() does not
    // take an offset.
    void renderMainScreen(int fullness, MoodSprite mood, const char* petName, int spriteOffsetX, int spriteOffsetY);
    #ifdef ENABLE_MULTISCREEN
    void renderStatsScreen(int happiness, int fullness, int energy, int cleanliness, int sick, int hydration, MoodSprite mood, const char* petName);
    #endif
    #ifdef ENABLE_ACTION_MENU
    // The Interact screen needs two pieces of information about the action menu:
    // the action name to display, and which stat bar to highlight. We pass these
    // as primitives so DisplayManager does not need to know what an ActionMenu is.
    void renderInteractScreen(int happiness, int fullness, int energy, int cleanliness, int sick, int hydration,
                              MoodSprite mood, const char* selectedActionName,
                              RelevantStat relevantStat, const char* petName,
                              int spriteOffsetX, int spriteOffsetY);
    #endif

    // Draws the two-tab nav bar at the bottom of the Main screen.
    // The highlighted tab (mainNavIndex) gets a filled background.
    void drawMainNavBar();

    // Draws the single fullness bar on the Main screen (label + red progress bar).
    // The Main screen otherwise shows only the pet's face, so this is the one
    // stat the user can watch at a glance from Session 1 onward.
    void drawMainFullnessBar(int fullness);

    #ifdef ENABLE_ACTION_MENU
    // Draws a single stat bar for the action currently selected on the Interact screen.
    // The bar label, value, and colour are determined by relevantStat.
    // Draws nothing if relevantStat is STAT_NONE (Save / Back actions).
    void drawContextualStatBar(int happiness, int fullness, int energy, int cleanliness, int sick, int hydration, RelevantStat relevantStat);
    #endif

    // Draws just the mood label text at the given Y position.
    // Separated from drawPetSprite() so each screen can position the text independently.
    void showPetMoodText(MoodSprite mood, int textY);

    // spriteForMood() — maps a mood to the pixel data for its current frame.
    // The one place that decides which artwork each mood uses, so adding a new
    // mood means adding a single case here.
    const uint16_t* spriteForMood(MoodSprite mood, int frame);

public:
    DisplayManager();

    void init();

    // Screen management
    // Default colour is TFT_BLACK — callers can override to clear with any colour.
    // Using a default parameter (familiar from JavaScript) keeps both call styles
    // working without needing two separate methods.
    void clearScreen(uint32_t color = TFT_BLACK);
    void fillRect(int x, int y, int width, int height, uint32_t color);

    // Text display
    void printText(const char* text, int x, int y, uint32_t color = TFT_WHITE, uint8_t size = 2);
    void printCenteredText(const char* text, int y, uint32_t color = TFT_WHITE, uint8_t size = 2);

    // renderDisplay() — the single call that loop() makes every frame.
    // screenState comes from NavigationManager.
    // petIsDead bypasses the normal screen routing and shows the death screen.
    // selectedActionName and relevantStat are extracted from ActionMenu by the
    // caller — keeping DisplayManager unaware of it.
    // spriteOffsetX/Y nudge the pet sprite away from its normal centre so it can
    // follow the device tilt. They are plain pixel counts; pass 0, 0
    // to draw the pet dead-centre exactly as before.
    void renderDisplay(int happiness, int fullness, int energy, int cleanliness, int sick, int hydration,
                       MoodSprite mood, const char* selectedActionName,
                       RelevantStat relevantStat,
                       bool petIsDead, const char* petName, ScreenState screenState,
                       int spriteOffsetX, int spriteOffsetY);

    // Pet display helpers — used internally and by the three private render methods
    #ifdef ENABLE_MULTISCREEN
    // Both are used only by the Stats screen: the five-bar readout and its mood label.
    void showPetStatus(int happiness, int fullness, int energy, int cleanliness, int sick, int hydration, const char* petName);
    void showPetMood(MoodSprite mood);
    #endif
    void showMessage(const char* message);
    void showDeathScreen();

    #ifdef ENABLE_ACTION_MENU
    // drawMenuIndicator() — draws the compact action name overlay at the bottom.
    // Used on the Interact screen. Takes the action name as a plain string so
    // this helper has no knowledge of ActionMenu's internals.
    void drawMenuIndicator(const char* selectedActionName, int x, int y);
    #endif

    // drawStatusBar() — draws a single labelled progress bar.
    void drawStatusBar(int value, int maxValue, int x, int y, int width, uint32_t color);

    // drawPetSprite() — draws a bitmap sprite as the pet's face. The sprite is
    // centred horizontally on the screen, with its vertical centre at faceCenterY.
    // mood selects which artwork to draw: drawPetSprite() asks spriteForMood()
    // for the right pixels, so the pet's face matches how it is feeling.
    // Every screen uses the same 80x80 size; width/height are still passed in so
    // a different size could be supplied later without changing this function.
    // spriteOffsetX/Y are added to the centred position so the pet can slide to
    // follow the device tilt; pass 0, 0 to draw it dead-centre.
    void drawPetSprite(MoodSprite mood, int faceCenterY, int spriteWidth, int spriteHeight,
                       int spriteOffsetX, int spriteOffsetY);
};

#endif
