#ifndef WIRELESS_MANAGER_H
#define WIRELESS_MANAGER_H

#include <WebServer.h>
#include <WebSocketsServer.h>
#include "../Config/scaffold_config.h"
#include "../Pet/pet.h"

class WirelessManager {
public:
    WirelessManager();
    void begin(Pet& pet);
    void handleClient();
    void broadcastStats();

private:
    Pet*                petPtr;
    WebServer           server;
    WebSocketsServer    webSocket;
    unsigned long       lastBroadcastMs;

    static const unsigned long BROADCAST_INTERVAL = 500;

    void handleRoot();
    void handleCSS();
    void handleJS();
    void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    String buildStatsPage();
    String buildStatsJson();
    String moodToJson(MoodSprite mood);
};

#endif // WIRELESS_MANAGER_H
