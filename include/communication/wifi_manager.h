#pragma once

#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager();
    void begin(const char* ssid, const char* password);
    bool isConnected();
    void reconnect();
    const char* getSSID();
    IPAddress getLocalIP();
};