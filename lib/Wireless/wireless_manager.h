#ifndef WIRELESS_MANAGER_H
#define WIRELESS_MANAGER_H

#include <WebServer.h>
#include <WebSocketsServer.h>
#include "../Config/scaffold_config.h"
#include "../Pet/pet.h"
#ifdef ENABLE_PERSISTENCE
#include "../Storage/storage_manager.h"
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
    bool isResetRequested() const;
    void clearResetRequest();

private:
    Pet*                petPtr;
    WebServer           server;
    WebSocketsServer    webSocket;
    unsigned long       lastBroadcastMs;
    bool                resetRequested;

    #ifdef ENABLE_PERSISTENCE
    StorageManager*     storagePtr;
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
