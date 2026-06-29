#ifndef SCREEN_LAYOUT_H
#define SCREEN_LAYOUT_H

// A struct is a plain bundle of related values with named fields.
// Unlike a class, it has no methods and no private data — it is just data.
// Using a struct here means we can pass a whole zone around as one value,
// rather than passing four separate integers every time.

// Describes the position and size of one rectangular region of the screen.
// x and y are the top-left corner; width and height are in pixels.
struct ScreenZone {
    int x;
    int y;
    int width;
    int height;
};

// Groups the two Y positions that every stat bar needs: where the text label
// sits and where the progress bar sits directly below it.
struct StatBarZone {
    int labelY;
    int barY;
};

// -----------------------------------------------------------------------
// ScreenState — which top-level screen the user is currently viewing.
//
// An enum (enumeration) is a named list of integer constants. Using one
// instead of raw numbers (0, 1, 2) makes the code self-documenting: the
// reader can see SCREEN_STATS and immediately know what it means, rather
// than having to remember that "1 means stats".
//
// Each value is prefixed with SCREEN_ so it does not collide with other
// names elsewhere in the project. Without the prefix a plain enum value
// like STATS would sit in the global namespace and could clash with any
// other constant of the same name.
//
// This enum lives here (in screen_layout.h) because deciding *which*
// screen to show is a display concern. Any file that needs to know the
// current screen can include this header.
// -----------------------------------------------------------------------
enum ScreenState {
    SCREEN_MAIN,       // Pet face + bottom nav bar ("Stats" | "Interact")
    SCREEN_STATS,      // Stat bars + mood word, no pet sprite (read-only data view)
    SCREEN_INTERACT    // Pet face + contextual stat bar + action menu
};

// -----------------------------------------------------------------------
// RelevantStat — which pet stat is directly affected by a given action.
//
// Each menu action (Feed, Play, Sleep…) changes exactly one stat. Storing
// this information alongside the action lets DisplayManager pick the right
// stat bar to highlight on the Interact screen without needing a separate
// switch statement in every place that asks the question.
//
// STAT_NONE is used for Save and Back — actions that do not target a single
// stat, so no bar is drawn for them.
// -----------------------------------------------------------------------
enum RelevantStat {
    STAT_HAPPINESS,
    STAT_FULLNESS,
    STAT_ENERGY,
    STAT_CLEANLINESS,
    STAT_SICKNESS,
    STAT_NONE
};

// -----------------------------------------------------------------------
// MoodSprite — the four visual moods the pet's sprite can show.
//
// This is different from PetState (in pet.h), which says what the pet is
// *doing* (eating, sleeping…). MoodSprite says how the pet *looks* right
// now. Pet::computeMood() reads the stats and returns one of these, and
// DisplayManager uses it to choose BOTH the sprite picture and the mood
// word shown under it.
//
// It lives here (not in pet.h) for the same reason as the enums above:
// DisplayManager must be able to name these moods without knowing what a
// Pet is, and Pet can include this lightweight header to return one — just
// like the action menu includes this header to use RelevantStat. The rule
// is to keep the dependency pointing this way: Pet -> screen_layout.h, and
// Display -> screen_layout.h, but never Display -> Pet.
//
// Each value is prefixed with MOOD_ so it cannot clash with names
// elsewhere in the project. Adding a new mood is three small steps: add a
// value here, add a rule in Pet::computeMood(), and add a sprite case in
// DisplayManager::spriteForMood().
// -----------------------------------------------------------------------
enum MoodSprite {
    MOOD_NEUTRAL,
    MOOD_HAPPY,
    MOOD_UNWELL,
    MOOD_HUNGRY
};

#endif
