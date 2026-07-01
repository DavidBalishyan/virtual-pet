# Virtual Pet

A Tamagotchi-style virtual pet for the **M5StickC Plus 2** (ESP32), built over a 10-session embedded C++ course.

You feed it, play with it, put it to sleep. Ignore it and it dies. Press a button and start over. That's the whole thing.

---

## What it does

The pet lives on a 135x240 colour screen. It has stats that tick up and down on their own - fullness, happiness, energy, cleanliness, sickness, hydration, tiredness, sadness. You keep it alive through a menu: Feed, Play, Sleep, Bathe, Heal, Drink. Each action boosts one stat but costs another (playing makes the pet happy but tires it out). Let a critical stat hit zero and the pet dies. Press A on the death screen to respawn.

The sprite changes with the pet's mood - neutral, happy, hungry, unwell, thirsty - computed from its stats. You can tilt the device to slide the sprite around, or shake it to trigger play time from any screen.

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

## How it ticks

Boot, init hardware, then loop forever. Each loop does three things:

1. **Read inputs** - which buttons were pressed, what the accelerometer says, was it shaken.
2. **Update the pet** - stats decay on their own timer, state machine runs (eating? sleeping? dead?), handle whatever action the player picked.
3. **Draw the screen** - sprite, mood text, stat bars, nav tabs.

Every module has one job. The menu doesn't draw. The display doesn't know what a Pet is - it just takes numbers and sprites. Navigation doesn't know what actions exist. Each piece stays readable on its own.

---

## Feature switches

The project is a cumulative curriculum. Every major feature is wrapped in an `#ifdef`:

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

With `ENABLE_WIRELESS` on, the pet starts a WiFi access point called `VirtualPet` and serves a live stats dashboard at `http://192.168.4.1/`.

Connect your phone to the `VirtualPet` network (no password), open that address, and you'll see all eight stats as coloured bars, the pet's mood, and its name. The page refreshes every three seconds. If the pet dies, a red notice shows up at the bottom.

No router or internet needed - the pet is the server. Implementation is in `lib/Wireless/wireless_manager.cpp`.

---

## Tests

```bash
pio test -e native
```

Three suites cover the hardware-independent bits:
- **test_pet** (35 tests) - stats, actions, state machine, moods, death, reset
- **test_animation_manager** (8 tests) - frame cycling, wrap-around, reset
- **test_tilt_motion** (8 tests) - smoothing, clamping, self-recentre

The tests compile the real source files against a stub `Arduino.h` so they run on a PC.

---

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
