#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <cstdint>
#include <cstring>

class Preferences {
public:
    void begin(const char* name, bool readOnly = false) {}
    void putInt(const char* key, int32_t value) {}
    int32_t getInt(const char* key, int32_t defaultValue = 0) { return defaultValue; }
    void clear() {}
    void end() {}
};

#endif // PREFERENCES_H