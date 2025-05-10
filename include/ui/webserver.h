#ifndef UI_WEBSERVER_H
#define UI_WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class Webserver {
public:
    Webserver(int port);
    void begin();
    void handleClient();
    void stop();

private:
    AsyncWebServer server;
    int _port;
};

#endif // UI_WEBSERVER_H