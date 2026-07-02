#include <Arduino.h>
#include <WiFi.h>
#include "wireless_manager.h"

#include "dashboard_assets.h"

static const char* AP_SSID = "VirtualPet";

static const int HTTP_PORT = 80;
static const int WS_PORT = 81;

WirelessManager::WirelessManager()
    : petPtr(nullptr), server(HTTP_PORT), webSocket(WS_PORT), lastBroadcastMs(0) {
}

void WirelessManager::begin(Pet& pet) {
    petPtr = &pet;

    WiFi.softAP(AP_SSID);

    server.on("/", std::bind(&WirelessManager::handleRoot, this));
    server.on("/style.css", std::bind(&WirelessManager::handleCSS, this));
    server.on("/script.js", std::bind(&WirelessManager::handleJS, this));
    server.begin();

    webSocket.begin();
    webSocket.onEvent(std::bind(&WirelessManager::onWebSocketEvent, this,
                                std::placeholders::_1, std::placeholders::_2,
                                std::placeholders::_3, std::placeholders::_4));

#ifdef DEBUG
    Serial.print("WiFi AP started: ");
    Serial.print(AP_SSID);
    Serial.print(" at http://");
    Serial.print(WiFi.softAPIP());
    Serial.print(" | WebSocket on ws://");
    Serial.print(WiFi.softAPIP());
    Serial.print(":");
    Serial.println(WS_PORT);
#endif
}

void WirelessManager::handleClient() {
    server.handleClient();
    webSocket.loop();
}

void WirelessManager::broadcastStats() {
    unsigned long now = millis();
    if (now - lastBroadcastMs < BROADCAST_INTERVAL) return;
    lastBroadcastMs = now;

    String json = buildStatsJson();
    webSocket.broadcastTXT(json);
}

void WirelessManager::handleRoot() {
    String html = buildStatsPage();
    server.send(200, "text/html", html);
}

void WirelessManager::onWebSocketEvent(uint8_t num, WStype_t type,
                                        uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED: {
            String json = buildStatsJson();
            webSocket.sendTXT(num, json);
            break;
        }
        case WStype_TEXT:
            break;
        default:
            break;
    }
}

String WirelessManager::buildStatsJson() {
    String json = "{";
    json += "\"pet\":{";
    json += "\"name\":\"" + String(petPtr->getPetName()) + "\",";
    json += "\"mood\":\"" + moodToJson(petPtr->computeMood()) + "\",";
    json += "\"alive\":" + String(petPtr->isInDeadState() ? "false" : "true");
    json += "},";
    json += "\"stats\":{";
    json += "\"fullness\":"     + String(petPtr->getFullness()) + ",";
    json += "\"happy\":"        + String(petPtr->getHappy()) + ",";
    json += "\"energy\":"       + String(petPtr->getEnergised()) + ",";
    json += "\"cleanliness\":"  + String(petPtr->getCleanliness()) + ",";
    json += "\"sick\":"         + String(petPtr->getSick()) + ",";
    json += "\"hydration\":"    + String(petPtr->getHydration()) + ",";
    json += "\"tired\":"        + String(petPtr->getTired()) + ",";
    json += "\"sad\":"          + String(petPtr->getSad());
    json += "}";
    json += "}";
    return json;
}

String WirelessManager::moodToJson(MoodSprite mood) {
    switch (mood) {
        case MOOD_HAPPY:  return "happy";
        case MOOD_UNWELL: return "unwell";
        case MOOD_HUNGRY: return "hungry";
        case MOOD_THIRSTY: return "thirsty";
        case MOOD_NEUTRAL:
        default:          return "neutral";
    }
}

void WirelessManager::handleCSS() {
    server.send_P(200, "text/css", DASHBOARD_CSS);
}

void WirelessManager::handleJS() {
    server.send_P(200, "application/javascript", DASHBOARD_JS);
}

String WirelessManager::buildStatsPage() {
    String page = DASHBOARD_HTML;
    page.replace("{{PET_NAME}}", String(petPtr->getPetName()));
    return page;
}

