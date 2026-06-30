// piskel_converter — host-side tool (runs on Mac/PC, not on the ESP32).
// Reads a Piskel .c export and writes a .h file containing RGB565 pixel data
// that the M5StickC Plus 2 LCD can display directly via M5Canvas::pushImage().
//
// Usage:
//   Compile once:  g++ -std=c++17 -o piskel_converter main.cpp
//   Run:           ./piskel_converter <input.c> <output.h>
//
// Why is this tool needed?
//   Piskel exports pixels in ABGR8888 format: 4 bytes per pixel laid out as
//   alpha, blue, green, red (highest byte to lowest). The M5StickC Plus 2 LCD
//   uses RGB565 format: 2 bytes per pixel (5 red bits, 6 green bits, 5 blue
//   bits). This tool converts between the two.
//
//   Watch out: the format is ABGR, NOT ARGB. They look similar but the red and
//   blue bytes are in opposite positions. Treating one as the other makes red
//   pixels render as blue and blue pixels render as red. Green looks the same
//   in both formats (it always sits in the middle byte), so this bug is easy
//   to miss until you draw a sprite that contains red or blue.

// Standard C++ library includes.
// Each include unlocks a specific set of tools from the standard library.
#include <iostream>   // std::cout and std::cerr — printing to the terminal
#include <fstream>    // std::ifstream and std::ofstream — reading and writing files
#include <sstream>    // std::istringstream — treating a string like a stream of tokens
#include <string>     // std::string — working with text
#include <vector>     // std::vector — a resizable list (used to collect all pixel values)
#include <iomanip>    // std::setw and std::setfill — formatting hex output with leading zeros
#include <algorithm>  // std::transform — converting strings to upper/lower case
#include <cctype>     // std::isalnum, std::isxdigit — testing individual characters
#include <cinttypes>

// The transparent colour key used by M5Canvas::pushImage().
// Pixels with this value are skipped during drawing, letting the background
// show through. The logical colour is magenta (0xF81F), but the value stored
// in the output array is 0x1FF8 — the byte-swapped form.
//
// Why byte-swap?
//   The ESP32 is little-endian. A uint16_t stored in memory has its low byte
//   at the lower address. The SPI driver sends bytes in address order, so the
//   low byte goes first. The LCD controller is big-endian — it expects the
//   high byte first. Without pre-swapping, every colour arrives at the display
//   with its bytes reversed and the colour is wrong.
//
//   By storing the byte-swapped value, the bytes land on the SPI bus in the
//   order the display needs, and colours appear correctly. The transparent
//   colour key must also be stored in swapped form so pushImage() can compare
//   it against the (swapped) pixel values in the array.
static const uint16_t TRANSPARENT_COLOR_KEY = 0x1FF8;  // 0xF81F byte-swapped

// Convert one 32-bit ABGR8888 pixel (Piskel format) into a 16-bit RGB565
// pixel (M5StickC Plus 2 LCD format).
// Fully transparent pixels (alpha == 0) become the transparent colour key
// instead of black, so the drawing code can skip them.
uint16_t convertAbgrToRgb565(uint32_t abgrPixel) {
    // Extract the alpha channel from the most-significant byte.
    // A value of 0 means fully transparent; 255 means fully opaque.
    uint8_t alpha = (abgrPixel >> 24) & 0xFF;

    if (alpha == 0) {
        // Transparent pixel — replace with the colour key so the renderer
        // knows to skip this pixel when drawing the sprite.
        return TRANSPARENT_COLOR_KEY;
    }

    // Extract the red, green, and blue channels.
    // Piskel lays the bytes out as alpha (highest), then blue, then green,
    // then red (lowest). This is ABGR — note that blue and red are reversed
    // compared to the more familiar ARGB layout.
    uint8_t blue  = (abgrPixel >> 16) & 0xFF;
    uint8_t green = (abgrPixel >>  8) & 0xFF;
    uint8_t red   = (abgrPixel >>  0) & 0xFF;

    // Pack into RGB565: red gets 5 bits, green gets 6 bits, blue gets 5 bits.
    // Shifting right discards the least-significant bits — a small loss of
    // colour precision that is not visible on the LCD at this sprite size.
    uint16_t red5   = (red   >> 3) & 0x1F;   // top 5 bits of red
    uint16_t green6 = (green >> 2) & 0x3F;   // top 6 bits of green
    uint16_t blue5  = (blue  >> 3) & 0x1F;   // top 5 bits of blue

    // Combine the three channels into one 16-bit value.
    // Red occupies bits 15–11, green bits 10–5, blue bits 4–0.
    uint16_t rgb565 = (red5 << 11) | (green6 << 5) | blue5;

    // Swap the two bytes before storing.
    // The SPI bus sends bytes in memory order (low byte first on the ESP32).
    // The LCD controller expects the high byte first. Pre-swapping here means
    // the bytes arrive at the display in the correct order.
    return (rgb565 >> 8) | (rgb565 << 8);
}

// Read the integer value from a Piskel #define line.
// Piskel defines look like: "#define MY_SPRITE_WIDTH 32"
// This function returns the third token (the integer at the end).
int readDefineValue(const std::string& line) {
    std::istringstream stream(line);
    std::string token;
    int tokenIndex = 0;

    while (stream >> token) {
        tokenIndex++;
        if (tokenIndex == 3) {
            return std::stoi(token);
        }
    }

    return 0;
}

// Build a C-safe lowercase identifier string from a filename.
// Used to produce the array name and #define prefix in the output .h file.
// Example: "assets/sprites/raw/Pet_Idle.c" becomes "pet_idle"
std::string makeIdentifier(const std::string& filePath) {
    std::string name = filePath;

    // Remove directory path — keep only the filename part.
    size_t slashPosition = name.find_last_of("/\\");
    if (slashPosition != std::string::npos) {
        name = name.substr(slashPosition + 1);
    }

    // Remove the file extension.
    size_t dotPosition = name.find_last_of('.');
    if (dotPosition != std::string::npos) {
        name = name.substr(0, dotPosition);
    }

    // Convert every character to lowercase.
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    // Replace any character that is not a letter or digit with an underscore.
    // This makes the name safe to use as a C identifier.
    for (char& character : name) {
        if (std::isalnum(character) == 0) {
            character = '_';
        }
    }

    return name;
}

// main() is the entry point — the first function the OS calls when the program runs.
// argc is the number of arguments the user typed (including the program name itself).
// argv is an array of those arguments as strings.
// Example: running "./piskel_converter input.c output.h" gives argc=3,
//          argv[0]="./piskel_converter", argv[1]="input.c", argv[2]="output.h"
int main(int argc, char* argv[]) {
    // We expect exactly 3 arguments: the program name, the input file, and the output file.
    if (argc != 3) {
        std::cerr << "Usage: piskel_converter <input.c> <output.h>" << std::endl;
        std::cerr << "Example: ./piskel_converter assets/sprites/raw/pet_idle.c lib/Display/sprites/pet_idle.h" << std::endl;
        return 1;
    }

    std::string inputPath  = argv[1];   // path to the Piskel .c export
    std::string outputPath = argv[2];   // path for the converted .h file

    // --- Step 1: Read the Piskel .c file ---

    std::ifstream inputFile(inputPath);
    if (!inputFile.is_open()) {
        std::cerr << "ERROR: Could not open input file: " << inputPath << std::endl;
        return 1;
    }

    int frameCount  = 0;
    int frameWidth  = 0;
    int frameHeight = 0;

    // std::vector is a resizable list. We do not know how many pixels the file
    // contains until we have read it, so a vector lets us add values one at a
    // time with push_back() and it grows automatically to fit them all.
    std::vector<uint32_t> rawPixels;

    std::string line;
    while (std::getline(inputFile, line)) {
        // Check for the three dimension #define lines.
        if (line.find("FRAME_COUNT") != std::string::npos) {
            frameCount = readDefineValue(line);
        } else if (line.find("FRAME_WIDTH") != std::string::npos) {
            frameWidth = readDefineValue(line);
        } else if (line.find("FRAME_HEIGHT") != std::string::npos) {
            frameHeight = readDefineValue(line);
        }

        // Scan the line for every 8-digit hex value in Piskel format: 0xAARRGGBB.
        // We search for "0x" and then verify the next 8 characters are all hex digits.
        // This avoids std::regex, which has known memory issues on macOS for this
        // use case and would crash on files of this size.
        size_t searchPosition = 0;
        while (searchPosition < line.length()) {
            // Find the next "0x" prefix.
            size_t hexStart = line.find("0x", searchPosition);
            if (hexStart == std::string::npos) {
                break;  // No more hex values on this line.
            }

            // A Piskel pixel value is always exactly 8 hex digits after "0x".
            // Check that 8 more characters exist and that all 8 are hex digits.
            size_t digitsStart = hexStart + 2;
            size_t digitsEnd   = digitsStart + 8;

            if (digitsEnd > line.length()) {
                break;  // Not enough characters left for a full 8-digit value.
            }

            bool allHexDigits = true;
            for (size_t i = digitsStart; i < digitsEnd; i++) {
                if (std::isxdigit(line[i]) == 0) {
                    allHexDigits = false;
                    break;
                }
            }

            if (allHexDigits) {
                std::string hexString = line.substr(digitsStart, 8);
                uint32_t pixelValue = std::stoul(hexString, nullptr, 16);
                rawPixels.push_back(pixelValue);
            }

            // Advance past this "0x" to continue scanning the rest of the line.
            searchPosition = hexStart + 2;
        }
    }

    inputFile.close();

    // --- Step 2: Validate that the file was read correctly ---

    if (frameWidth == 0 || frameHeight == 0 || frameCount == 0) {
        std::cerr << "ERROR: Could not read sprite dimensions. Is this a valid Piskel .c export?" << std::endl;
        return 1;
    }

    int pixelsPerFrame    = frameWidth * frameHeight;
    int expectedPixelCount = frameCount * pixelsPerFrame;

    if (static_cast<int>(rawPixels.size()) < expectedPixelCount) {
        std::cerr << "ERROR: Expected " << expectedPixelCount << " pixels but only found ";
        std::cerr << rawPixels.size() << " in the file." << std::endl;
        return 1;
    }

    // --- Step 3: Build the identifier strings for the output file ---

    std::string lowerIdentifier = makeIdentifier(inputPath);

    std::string upperIdentifier = lowerIdentifier;
    std::transform(upperIdentifier.begin(), upperIdentifier.end(),
                   upperIdentifier.begin(), ::toupper);

    // --- Step 4: Write the output .h file ---

    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "ERROR: Could not open output file: " << outputPath << std::endl;
        return 1;
    }

    // Write the file header with format notes and usage instructions.
    outputFile << "#pragma once"                                                              << std::endl;
    outputFile << "#include <stdint.h>"                                                       << std::endl;
    outputFile << "#include <pgmspace.h>"                                                     << std::endl;
    outputFile << ""                                                                          << std::endl;
    outputFile << "// Sprite: " << lowerIdentifier                                           << std::endl;
    outputFile << "// Size:   " << frameWidth << "x" << frameHeight << " pixels"             << std::endl;
    outputFile << "// Frames: " << frameCount                                                 << std::endl;
    outputFile << "//"                                                                        << std::endl;
    outputFile << "// Format: RGB565 — 16-bit colour for the M5StickC Plus 2 LCD."           << std::endl;
    outputFile << "//   Each uint16_t value encodes one pixel as: RRRRRGGGGGGBBBBB"          << std::endl;
    outputFile << "//   5 bits red | 6 bits green | 5 bits blue"                             << std::endl;
    outputFile << "//"                                                                        << std::endl;
    outputFile << "// Transparent colour key: 0x1FF8 (byte-swapped magenta 0xF81F)."         << std::endl;
    outputFile << "//   Pixels with this value are skipped by the drawing code."             << std::endl;
    outputFile << "//   They let the screen background colour show through."                 << std::endl;
    outputFile << "//   The value is pre-swapped to match the byte order of all other"       << std::endl;
    outputFile << "//   pixels in this array (see byte-swap note in the converter tool)."    << std::endl;
    outputFile << "//"                                                                        << std::endl;
    outputFile << "// PROGMEM: marks this array for storage in ESP32 flash memory."          << std::endl;
    outputFile << "//   On ESP32, PROGMEM is equivalent to const — flash storage is"         << std::endl;
    outputFile << "//   memory-mapped and arrays can be accessed with normal [] syntax."      << std::endl;
    outputFile << "//   It is included here as a clear signal that this data is read-only"   << std::endl;
    outputFile << "//   and should not be copied into RAM."                                  << std::endl;
    outputFile << "//"                                                                        << std::endl;
    outputFile << "// Generated by: tools/piskel_converter"                                  << std::endl;
    outputFile << ""                                                                          << std::endl;
    outputFile << "#define SPRITE_" << upperIdentifier << "_FRAME_COUNT  " << frameCount     << std::endl;
    outputFile << "#define SPRITE_" << upperIdentifier << "_WIDTH        " << frameWidth      << std::endl;
    outputFile << "#define SPRITE_" << upperIdentifier << "_HEIGHT       " << frameHeight     << std::endl;
    outputFile << ""                                                                          << std::endl;
    outputFile << "// Transparent colour key — pass this to pushImage() so it skips"         << std::endl;
    outputFile << "// transparent pixels and lets the background show through."              << std::endl;
    outputFile << "// This is 0xF81F (magenta) with its two bytes pre-swapped to 0x1FF8,"   << std::endl;
    outputFile << "// matching the byte order of all other pixel values in this array."      << std::endl;
    outputFile << "#ifndef SPRITE_TRANSPARENT_COLOR"                                          << std::endl;
    outputFile << "#define SPRITE_TRANSPARENT_COLOR 0x1FF8"                                   << std::endl;
    outputFile << "#endif"                                                                    << std::endl;
    outputFile << ""                                                                          << std::endl;
    outputFile << "static const uint16_t PROGMEM sprite_" << lowerIdentifier;
    outputFile << "[" << frameCount << "][" << pixelsPerFrame << "] = {"                      << std::endl;

    // Write each frame as a block of RGB565 rows.
    for (int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
        outputFile << "{" << std::endl;

        int frameStartPixel = frameIndex * pixelsPerFrame;

        for (int row = 0; row < frameHeight; row++) {
            outputFile << "    ";

            for (int col = 0; col < frameWidth; col++) {
                int pixelIndex = frameStartPixel + (row * frameWidth) + col;
                uint16_t rgb565Pixel = convertAbgrToRgb565(rawPixels[pixelIndex]);

                // Write the pixel as a 4-digit hex value with leading zeros.
                outputFile << "0x"
                           << std::hex << std::uppercase
                           << std::setw(4) << std::setfill('0')
                           << rgb565Pixel
                           << std::dec;

                // Every pixel except the very last in the frame gets a comma.
                bool isLastPixelInFrame = (row == frameHeight - 1) && (col == frameWidth - 1);
                if (!isLastPixelInFrame) {
                    outputFile << ", ";
                }
            }

            outputFile << std::endl;
        }

        // All frames except the last need a comma after the closing brace,
        // because they are elements in the outer array initialiser list.
        bool isLastFrame = (frameIndex == frameCount - 1);
        if (isLastFrame) {
            outputFile << "}" << std::endl;
        } else {
            outputFile << "}," << std::endl;
        }
    }

    outputFile << "};" << std::endl;
    outputFile.close();

    // Report what was converted so the user can confirm the numbers look right.
    std::cout << "Converted " << frameCount << " frame(s), "
              << frameWidth << "x" << frameHeight << " pixels each." << std::endl;
    std::cout << "Output: " << outputPath << std::endl;

    return 0;
}
