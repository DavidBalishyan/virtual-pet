#ifndef M5STICKCPLUS2_H
#define M5STICKCPLUS2_H

#include <Arduino.h>
#include <cstdint>
#include <cstddef>
#include <cstdio>

// Color constants (matches TFT_eSPI / M5GFX values)
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0xFD20
#define TFT_GREENYELLOW 0xAFE5
#define TFT_PINK        0xF81F

class M5Unified {
public:
    void begin() {}
    void update() {}
    class Display {
    public:
        void fillScreen(uint32_t) {}
        int16_t width() { return 135; }
        int16_t height() { return 240; }
        void setRotation(int) {}
        uint16_t color565(uint8_t, uint8_t, uint8_t) { return 0; }
    } Lcd;

    class Button {
    public:
        bool isPressed() { return false; }
    } BtnA, BtnB;

    class IMU {
    public:
        void getAccel(float* x, float* y, float* z) { *x = 0; *y = 0; *z = 1.0f; }
    } Imu;

    class Speaker {
    public:
        void setVolume(int) {}
        void tone(int, int) {}
        void stop() {}
    } Speaker;
};

class M5Canvas {
public:
    M5Canvas() {}
    M5Canvas(M5Unified::Display* parent) {}
    void setColorDepth(int8_t) {}
    void* createSprite(int16_t w, int16_t h) { return reinterpret_cast<void*>(1); }
    void pushSprite(int16_t x, int16_t y) {}
    void fillScreen(uint32_t color) {}
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) {}
    void setTextSize(int8_t size) {}
    void setTextColor(uint32_t color) {}
    void setCursor(int16_t x, int16_t y) {}
    int printf(const char* format, ...) { return 0; }
    size_t print(const char* s) { return 0; }
    size_t print(int n) { return 0; }
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) {}
    void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data, uint16_t transparent) {}
    int16_t width() const { return 135; }
    int16_t height() const { return 240; }
};

inline M5Unified M5;

#endif // M5STICKCPLUS2_H
