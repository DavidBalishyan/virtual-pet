#ifndef SCAFFOLD_CONFIG_H
#define SCAFFOLD_CONFIG_H

// =========================================================================
// scaffold_config.h — the teaching "feature switches" for the whole project.
// =========================================================================
//
// WHY THIS FILE EXISTS
// This virtual pet is built up over a 10-session course. At the start of each
// session a student arrives with everything from the previous sessions and
// adds ONE new feature. To make that possible from a single set of source
// files, each big feature is wrapped in an "#ifdef" — the code only gets
// compiled when its flag below is switched on.
//
// HOW TO USE IT
// Each line below is a switch. A plain "#define" line is ON; adding a leading
// "// " comments it out, which turns the feature OFF (the code is compiled
// out — it is as if that code had not been written yet). This is the same
// idea as the SPRITE_TEST switch in main.cpp.
//
// On this "finished pet" reference, every switch is ON. Each session branch
// in the course turns some OFF to recreate that session's starting point.
//
// WHY A SEPARATE HEADER (and not just #define in main.cpp)?
// main.cpp and every file under lib/ are compiled separately, so a #define
// written in main.cpp is NOT visible inside, say, pet.cpp. Putting the
// switches in one header that every feature file #includes is the only way
// for all of them to agree on which features are turned on.
//
// THE SESSIONS (each one turns on the next switch — a cumulative staircase):
//   Session 1  none on            — boot + screen + one stat + sprite
//   Session 2  ENABLE_ACTION_MENU — the button menu (Feed / Play / ...)
//   Session 3  ENABLE_IMU_PLAY    — shake the device to play
//   Session 4  ENABLE_SOUND       — buzzer melodies and alerts
//   Session 5  ENABLE_PERSISTENCE — save and load the pet (the Save action)
//   Session 6  ENABLE_MULTISCREEN + ENABLE_MOOD_SPRITES — the Stats screen
//                                   and the four mood faces
// -------------------------------------------------------------------------

// #define ENABLE_ACTION_MENU
// #define ENABLE_IMU_PLAY
// #define ENABLE_SOUND
// #define ENABLE_PERSISTENCE
// #define ENABLE_MULTISCREEN
// #define ENABLE_MOOD_SPRITES

// -------------------------------------------------------------------------
// Rule between switches: the "Save" action lives inside the action menu, so
// you cannot have saving without the menu. If the switches are set to an
// impossible combination the build stops here with a clear message, rather
// than failing later with a confusing error.
// -------------------------------------------------------------------------
#if defined(ENABLE_PERSISTENCE) && !defined(ENABLE_ACTION_MENU)
#error "ENABLE_PERSISTENCE requires ENABLE_ACTION_MENU (the Save action lives in the menu)."
#endif

#endif // SCAFFOLD_CONFIG_H
