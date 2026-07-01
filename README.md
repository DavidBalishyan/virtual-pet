# Virtual Pet

A Tamagotchi-style virtual pet that runs on the **M5StickC Plus 2** (ESP32), built piece by piece over a 10-session embedded C++ course.

Shake it. Feed it. Watch it get hungry. If you ignore it long enough, it dies. Press a button and start again — that's the whole loop.

---

## What it does

The pet lives on the little 135x240 LCD. It has eight stats that tick up and down on their own — fullness, happiness, energy, cleanliness, sickness, hydration, tiredness, sadness. You keep it alive by picking actions from a menu: Feed, Play, Sleep, Bathe, Heal, Drink. Each action bumps one stat at the cost of another (play makes the pet happy but drains its energy). Let any critical stat hit zero and the pet dies. Press A on the death screen to start a new life.

It has moods, too. The sprite changes depending on how the pet is feeling — neutral, happy, hungry, unwell, thirsty. The mood is computed from the stats, so the face always matches.

You can also tilt the device to slide the sprite around the screen, or give it a shake to trigger play time from anywhere.

---

## Hardware

| What | Part |
|---|---|
| Board | M5StickC Plus 2 (ESP32-PICO-V3-02) |
| Screen | 135 x 240 colour LCD |
| Sensors | MPU6886 accelerometer/gyro |
| Audio | Built-in buzzer |
| Storage | NVS flash (stats survive power-off) |
| Wireless | WiFi (serves a live stats dashboard in a browser) |

---

## Building and running

This is a [PlatformIO](https://platformio.org/) project.

```bash
# Build and upload to the device
pio run -e m5stick-c -t upload

# Open the serial monitor
pio device monitor

# Run unit tests on your PC
pio test -e native
```

---

## Project structure

```
lib/
  Config/        Feature switches that turn each session's code on/off
  Pet/           The pet itself — stats, actions, mood computation, state machine
  Display/       Screen drawing, sprite rendering, animation timing, layout
  Button/        Button A/B/C edge detection
  Imu/           Accelerometer reads, shake detection, tilt motion smoothing
  Timer/         Background stat decay — the pet gets hungry on its own
  Actions/       The Feed / Play / Sleep / … menu
  Navigation/    Screen transitions (Main, Stats, Interact)
  Speaker/       Buzzer melodies for events and alerts
  Storage/       Save and load stats to NVS flash
  Wireless/      WiFi access point and web dashboard
src/main.cpp     The single entry point — setup + loop, ties everything together
test/            Unit tests (Unity framework, runnable on PC)
tools/           Sprite conversion utilities
assets/          Raw sprite artwork
```

---

## How it works

The device boots, initialises the hardware, and enters a forever-loop. Each iteration of that loop does three things:

1. **Read inputs** — which buttons were pressed, what the accelerometer says, whether the device was shaken.
2. **Update the pet** — apply background stat decay on its own timer, run the state machine (is the pet eating? sleeping? dead?), handle any action the user selected.
3. **Render the screen** — draw the sprite, the mood word, the stat bars, the navigation tabs.

Every module has a single job. The action menu doesn't know how to draw. The display manager doesn't know what a Pet object is — it just takes plain numbers and sprites. The navigation manager doesn't know what actions exist. This keeps each piece readable on its own.

---

## Feature switches

The project is designed as a cumulative curriculum. Every major feature is wrapped in an `#ifdef`:

| Flag | What it enables | Session |
|---|---|---|
| *(none)* | Boot, screen, one stat, sprite | 1 |
| `ENABLE_ACTION_MENU` | Feed / Play / Sleep / Bathe / Heal / Drink menu | 2 |
| `ENABLE_IMU_PLAY` | Shake to play, tilt to slide the sprite | 3 |
| `ENABLE_SOUND` | Buzzer melodies and alerts | 4 |
| `ENABLE_PERSISTENCE` | Save/load stats to NVS (survives power-off) | 5 |
| `ENABLE_MULTISCREEN` | Stats screen with all stat bars | 6 |
| `ENABLE_MOOD_SPRITES` | Four mood faces instead of one | 6 |
| `ENABLE_WIRELESS` | WiFi AP and web dashboard | 7 |

Flip them on and off in `lib/Config/scaffold_config.h` to see the project at each stage.

---

## Web dashboard

When `ENABLE_WIRELESS` is on, the pet broadcasts a WiFi access point called `VirtualPet` and serves a live stats dashboard at `http://192.168.4.1/`.

Connect your phone or laptop to the `VirtualPet` network — no password — then open that address in a browser. You'll see all eight stats as coloured progress bars, the pet's current mood as a badge, and its name. The page auto-refreshes every three seconds so you can watch the numbers change in real time without looking at the device screen. If the pet dies, a bold red notice appears at the bottom of the page.

The whole thing runs on the device itself — the ESP32's softAP mode means no router or internet connection is needed. The pet is the server.

Implementation lives in `lib/Wireless/wireless_manager.cpp`. It uses the Arduino `WebServer` library and builds the HTML page as plain string concatenation (no templates, no filesystem). The dashboard is deliberately kept to a single page with inline CSS so there's nothing to cache or serve beyond one HTTP response.

---

## Tests

Three test suites cover the hardware-independent modules:

```
pio test -e native
```

- **test_pet** (35 tests) — stats, actions, state machine, moods, death, reset
- **test_animation_manager** (8 tests) — frame cycling, wrap-around, reset
- **test_tilt_motion** (8 tests) — smoothing, clamping, self-recentre

The tests compile the real source files from `lib/` against a stub `Arduino.h` so they run on a PC with no hardware attached.

---

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
