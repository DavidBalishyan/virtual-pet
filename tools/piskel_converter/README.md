# Piskel Converter

A small C++ command-line tool that converts a Piskel sprite export (`.c` file) into a C++ header file (`.h`) ready to include in the virtual pet firmware.

This tool runs on your **Mac or PC** — it is never compiled into the ESP32 firmware. See `platformio.ini` for a note on why PlatformIO ignores the `tools/` directory.

---

## Why this tool exists

Piskel exports pixel data in **ARGB8888** format: 4 bytes per pixel, stored as `0xAARRGGBB`.

The M5StickC Plus 2 LCD uses **RGB565** format: 2 bytes per pixel, with 5 bits for red, 6 bits for green, and 5 bits for blue.

This tool converts every pixel from one format to the other, and replaces fully transparent pixels with a magenta colour key (`0xF81F`) that the drawing code uses to skip those pixels at render time.

See `SPRITE_GUIDE.md` at the project root for the full sprite creation workflow.

---

## Prerequisites

You need a C++ compiler with C++17 support. On macOS this is already installed as part of the Xcode Command Line Tools.

To check you have it, open Terminal and run:

```bash
g++ --version
```

If you see a version number, you are ready. If not, run:

```bash
xcode-select --install
```

---

## Step 1 — Compile the converter (one time only)

Open a terminal at the project root and run:

```bash
g++ -std=c++17 -o tools/piskel_converter/piskel_converter tools/piskel_converter/main.cpp
```

This produces an executable called `piskel_converter` inside `tools/piskel_converter/`. You only need to do this once. If you ever edit `main.cpp`, run the command again to recompile.

---

## Step 2 — Run the converter

```bash
./tools/piskel_converter/piskel_converter <input.c> <output.h>
```

**Arguments:**

| Argument | Description |
|---|---|
| `<input.c>` | Path to the Piskel `.c` export. These live in `assets/sprites/raw/`. |
| `<output.h>` | Path for the generated header file. These go in `lib/Display/sprites/`. |

**Example — converting a pet idle sprite:**

```bash
./tools/piskel_converter/piskel_converter assets/sprites/raw/pet_idle.c lib/Display/sprites/pet_idle.h
```

On success you will see:

```
Converted 1 frame(s), 32x32 pixels each.
Output: lib/Display/sprites/pet_idle.h
```

---

## What the output file looks like

The generated `.h` file contains a `uint16_t` array in RGB565 format, one row of pixels per line for readability:

```cpp
#pragma once
#include <stdint.h>
#include <pgmspace.h>

// Sprite: pet_idle
// Size:   32x32 pixels
// Frames: 1
// ...

#define SPRITE_PET_IDLE_FRAME_COUNT  1
#define SPRITE_PET_IDLE_WIDTH        32
#define SPRITE_PET_IDLE_HEIGHT       32

#ifndef SPRITE_TRANSPARENT_COLOR
#define SPRITE_TRANSPARENT_COLOR 0xF81F
#endif

static const uint16_t PROGMEM sprite_pet_idle[1][1024] = {
{
    0xF81F, 0xF81F, 0xF81F, ...,   // transparent pixels (background shows through)
    0x5CCD, 0x5CCD, 0xF81F, ...,   // coloured pixels (the pet body)
    ...
}
};
```

To use this sprite in the firmware, `#include` the generated header in `display_manager.cpp` (or wherever the drawing code lives).

---

## Converting all sprites at once

Once you have several sprites ready, you can convert them all with a short shell loop run from the project root:

```bash
for file in assets/sprites/raw/*.c; do
    name=$(basename "$file" .c)
    ./tools/piskel_converter/piskel_converter "$file" "lib/Display/sprites/${name}.h"
done
```

---

## Troubleshooting

**"Could not open input file"**
Check the path to the `.c` file. Make sure the file was exported from Piskel and saved to `assets/sprites/raw/`.

**"Could not read sprite dimensions"**
The input file is not a valid Piskel C export, or the `#define` lines are missing. Re-export from Piskel using Export → C file.

**"Expected X pixels but only found Y"**
The pixel data in the file is shorter than the declared dimensions suggest. Re-export from Piskel — the file may have been truncated.
