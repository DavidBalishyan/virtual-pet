#ifndef WIRELESS_MANAGER_H
#define WIRELESS_MANAGER_H

#include <WebServer.h>
#include <WebSocketsServer.h>
#include "../Config/scaffold_config.h"
#include "../Pet/pet.h"
#include "../Timer/time_manager.h"
#include "../Display/display_manager.h"
#ifdef ENABLE_PERSISTENCE
#include "../Storage/storage_manager.h"
#endif
#ifdef ENABLE_SOUND
#include "../Speaker/speaker_manager.h"
#endif

class WirelessManager {
public:
    WirelessManager();
    void begin(Pet& pet);
    void handleClient();
    void broadcastStats();

    // Dashboard command handling
    #ifdef ENABLE_PERSISTENCE
    void setStorage(StorageManager& storage);
    #endif
    void setTimers(TimerManager& timers);
    // Gives the dashboard a handle on the screen so a "setBrightness" command
    // can adjust the backlight and buildStatsJson can report the current level.
    void setDisplay(DisplayManager& display);
    #ifdef ENABLE_SOUND
    // Gives the dashboard a handle on the speaker so "playSong"/"stopSong"
    // commands can start and stop the non-blocking song player.
    void setSpeaker(SpeakerManager& speaker);
    #endif
    bool isResetRequested() const;
    void clearResetRequest();

private:
    Pet*                petPtr;
    TimerManager*       timersPtr;
    DisplayManager*     displayPtr;
    WebServer           server;
    WebSocketsServer    webSocket;
    unsigned long       lastBroadcastMs;
    bool                resetRequested;

    #ifdef ENABLE_PERSISTENCE
    StorageManager*     storagePtr;
    #endif
    #ifdef ENABLE_SOUND
    SpeakerManager*     speakerPtr;
    #endif

    static const unsigned long BROADCAST_INTERVAL = 500;

    void handleRoot();
    void handleCSS();
    void handleJS();
    void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    void processCommand(const String& json);
    String buildStatsPage();
    String buildStatsJson();
    String moodToJson(MoodSprite mood);
};

#endif // WIRELESS_MANAGER_H
