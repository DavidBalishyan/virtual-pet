#include <Arduino.h>
#include <WiFi.h>
#include "wireless_manager.h"

// The SSID the pet broadcasts. Students sharing a classroom can add their own
// identifier by appending something unique here so their phone connects to the
// right device - e.g. "VirtualPet-Bob".
static const char* AP_SSID = "VirtualPet";

// Forward declarations of static helpers used by buildStatsPage().
// Declared here (not in the header) because they are internal to this file.
static String statRow(const char* name, int value, const char* color);
static String moodBadgeHtml(MoodSprite mood);

// The ESP32 softAP always uses 192.168.4.x for the connected device and
// 192.168.4.1 for itself, so the dashboard is always at http://192.168.4.1.
// The port is the standard HTTP port 80.
static const int HTTP_PORT = 80;

// Constructor
WirelessManager::WirelessManager()
    : petPtr(nullptr), server(HTTP_PORT) {
}

// Starts the WiFi access point, stores a pointer to the pet so the
// dashboard handler can read live stats, and registers the root route.
//
// Call this once from setup, after the pet has been constructed and
// any saved stats have been loaded.
void WirelessManager::begin(Pet& pet) {
    petPtr = &pet;

    // Start the software access point. No password - this is a local,
    // close-range dashboard that lives entirely on the device, so there
    // is no security risk worth the complexity of a password setup UI.
    WiFi.softAP(AP_SSID);

    // Register the root route using a named member function wrapped
    // with std::bind. This avoids the prohibited lambda syntax while
    // still letting us keep handleRoot() as a regular member function
    // that can access the pet through petPtr.
    server.on("/", std::bind(&WirelessManager::handleRoot, this));

    server.begin();

#ifdef DEBUG
    Serial.print("WiFi AP started: ");
    Serial.print(AP_SSID);
    Serial.print(" at http://");
    Serial.println(WiFi.softAPIP());
#endif
}

// Processes one incoming HTTP request, if any. Must be called once per
// loop() so the web server does not stall or drop connections.
void WirelessManager::handleClient() {
    server.handleClient();
}

// Called by the web server when a browser requests GET /.
// Builds the full HTML dashboard page and sends it back as a complete HTTP response.
void WirelessManager::handleRoot() {
    String html = buildStatsPage();
    server.send(200, "text/html", html);
}

String WirelessManager::buildStatsPage() {
    String page = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">";
    // Auto refresh the page every 3 seconds so the user sees live stats
    page += "<meta http-equiv=\"refresh\" content=\"3\">";

    page += "<title>Virtual Pet Dashboard</title>";
    page += "<style>";
    page += "body{background:#1a1a2e;color:#eee;font-family:sans-serif;"
            "padding:16px;max-width:400px;margin:0 auto;}";
    page += "h1{color:#f1c40f;text-align:center;margin-bottom:4px;}";
    page += ".pet-name{text-align:center;color:#aaa;margin-top:0;margin-bottom:20px;}";
    // Mood badge
    page += ".mood{text-align:center;margin-bottom:20px;}";
    page += ".mood-badge{display:inline-block;padding:6px 18px;border-radius:12px;"
            "font-weight:bold;font-size:18px;}";
    // Stat row layout
    page += ".stat{margin-bottom:14px;}";
    page += ".stat-label{display:flex;justify-content:space-between;margin-bottom:2px;}";
    page += ".stat-name{font-size:14px;}";
    page += ".stat-value{font-size:14px;font-weight:bold;}";
    // Progress bar
    page += ".bar-bg{background:#333;border-radius:6px;overflow:hidden;height:16px;}";
    page += ".bar-fill{height:100%;border-radius:6px;transition:width 0.3s;}";
    // Footer with connection info
    page += ".footer{margin-top:24px;font-size:11px;color:#666;text-align:center;}";
    page += "</style></head><body>";

    // Title
    page += "<h1>Virtual Pet</h1>";
    page += "<p class=\"pet-name\">" + String(petPtr->getPetName()) + "</p>";

    // Mood
    page += "<div class=\"mood\">";
    page += moodBadgeHtml(petPtr->computeMood());
    page += "</div>";

    // Stats
    page += statRow("Fullness",     petPtr->getFullness(),      "#e74c3c");
    page += statRow("Happy",        petPtr->getHappy(),         "#2ecc71");
    page += statRow("Energy",       petPtr->getEnergised(),     "#3498db");
    page += statRow("Cleanliness",  petPtr->getCleanliness(),   "#1abc9c");
    page += statRow("Sick",         petPtr->getSick(),          "#9b59b6");
    page += statRow("Hydration",    petPtr->getHydration(),     "#e67e22");
    page += statRow("Tired",        petPtr->getTired(),         "#95a5a6");
    page += statRow("Sad",          petPtr->getSad(),           "#7f8c8d");

    // Dead state notice
    if (petPtr->isInDeadState()) {
        page += "<p style=\"color:#e74c3c;text-align:center;font-weight:bold;"
                "margin-top:16px;\">Your pet has died!</p>";
    }

    // Footer
    page += "<div class=\"footer\">";
    page += "Connected to <strong>";
    page += AP_SSID;
    page += "</strong> -";
    page += "<a href=\"/\" style=\"color:#666;\">Refresh</a>";
    page += "</div>";

    page += "</body></html>";
    return page;
}

// Builds the HTML for one labelled progress bar, matching the visual
static String statRow(const char* name, int value, const char* color) {
    // Clamp the bar width to the 0–100 range
    int clamped = value;
    if (clamped < 0)  { clamped = 0; }
    if (clamped > 100) { clamped = 100; }

    String row = "<div class=\"stat\">";
    row += "<div class=\"stat-label\">";
    row += "<span class=\"stat-name\">" + String(name) + "</span>";
    row += "<span class=\"stat-value\">" + String(value) + "</span>";
    row += "</div>";
    row += "<div class=\"bar-bg\">";
    row += "<div class=\"bar-fill\" style=\"width:" + String(clamped) + "%;background:" + String(color) + ";\"></div>";
    row += "</div></div>";
    return row;
}

// Returns a coloured pill HTML snippet for the given mood, using the
// same label and colour associations as showPetMoodText() on the LCD.
static String moodBadgeHtml(MoodSprite mood) {
    const char* label;
    const char* bgColor;

    switch (mood) {
        case MOOD_HAPPY:  label = "Happy";   bgColor = "#2ecc71"; break;
        case MOOD_UNWELL: label = "Unwell";  bgColor = "#9b59b6"; break;
        case MOOD_HUNGRY: label = "Hungry";  bgColor = "#e74c3c"; break;
        case MOOD_THIRSTY: label = "Thirsty"; bgColor = "#e67e22"; break;
        case MOOD_NEUTRAL:
        default:          label = "Neutral"; bgColor = "#95a5a6"; break;
    }

    String html = "<span class=\"mood-badge\" style=\"background:";
    html += bgColor;
    html += ";\">";
    html += label;
    html += "</span>";
    return html;
}
