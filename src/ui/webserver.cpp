#include "../../include/ui/webserver.h"
#include "../../include/core/logging.h"

Webserver::Webserver(int port) : server(port), _port(port) {}

void Webserver::begin() {
    server.begin();
    LOG_INFO("WEBSERVER", "Serveur web démarré sur le port %d", _port);
}

void Webserver::handleClient() {
    // La gestion des clients est gérée automatiquement par AsyncWebServer
}

void Webserver::stop() {
    server.end();
    LOG_INFO("WEBSERVER", "Serveur web arrêté");
}
