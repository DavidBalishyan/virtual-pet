#ifndef WIRELESS_MANAGER_H
#define WIRELESS_MANAGER_H

#include <WebServer.h>
#include "../Config/scaffold_config.h"
#include "../Pet/pet.h"

class WirelessManager {
public:
    WirelessManager();
    void begin(Pet& pet);
    void handleClient();

private:
    Pet*       petPtr;   // Points to the pet whose stats we serve
    WebServer  server;   // HTTP server on port 80

    // Handles GET / - builds and returns the stats dashboard page.
    // Registered with std::bind in begin() so it stays a named member function.
    void handleRoot();

    // Builds the full HTML page string from the pet's current stats.
    String buildStatsPage();
};

#endif
