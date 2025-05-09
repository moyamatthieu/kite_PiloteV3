#if MODULE_WEBSERVER_ENABLED

#include "communication/kite_webserver.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include "fallback_html.h"
#include "../include/config.h"
#include "core/config.h"
// Logging
#include "core/logging.h"

// Forward declarations des handlers en IRAM
static void IRAM_ATTR handleRoot(AsyncWebServerRequest *request);
static void IRAM_ATTR handleApiInfo(AsyncWebServerRequest *request);
static void IRAM_ATTR handleApiRestart(AsyncWebServerRequest *request);
static void IRAM_ATTR handleDashboard(AsyncWebServerRequest *request);
static void IRAM_ATTR handleFavicon(AsyncWebServerRequest *request);
static void IRAM_ATTR handleNotFound(AsyncWebServerRequest *request);

// Buffers statiques et flags pour mise en cache
static char jsonBuffer[256]; // Buffer statique pour JSON
static char htmlBuffer[1024]; // Buffer statique pour HTML
static bool htmlCacheValid = false;
static unsigned long lastHtmlGenTime = 0;
static IPAddress lastCachedIP;
static char lastStatus[64] = "";

// Fonction pour obtenir le port du serveur - optimisée
uint16_t getServerPort(AsyncWebServer* server) {
    return SERVER_PORT; // Utiliser la constante définie dans config.h
}

// Fonction pour définir le mode de fonctionnement
void setWebServerMode(bool useFiles) {
    // Toujours mode HTML généré
    LOG_INFO("WEBS", "Mode HTML optimisé activé");
}

// Fonction interne pour générer ou retourner le HTML en cache
static IRAM_ATTR const char* getHtmlContent() {
    unsigned long now = millis();
    bool ipChanged = WiFi.localIP() != lastCachedIP;
    const char* status = getSystemStatusString().c_str();
    bool statusChanged = strcmp(status, lastStatus) != 0;
    if (!htmlCacheValid || ipChanged || statusChanged || now - lastHtmlGenTime > 5000) {
        // Régénération du HTML dans htmlBuffer
        snprintf(htmlBuffer, sizeof(htmlBuffer), fallbackHtml,
                 status, WiFi.localIP().toString().c_str());
        // Mettre à jour le cache
        htmlCacheValid = true;
        lastHtmlGenTime = now;
        lastCachedIP = WiFi.localIP();
        strncpy(lastStatus, status, sizeof(lastStatus)-1);
        lastStatus[sizeof(lastStatus)-1] = '\0';
    }
    return htmlBuffer;
}

// Handlers externes
static void IRAM_ATTR handleRoot(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", getHtmlContent());
    response->addHeader("Cache-Control", "max-age=30");
    request->send(response);
}

static void IRAM_ATTR handleApiInfo(AsyncWebServerRequest *request) {
    IPAddress currentIP = WiFi.localIP();
    const char* status = getSystemStatusString().c_str();
    snprintf(jsonBuffer, sizeof(jsonBuffer),
             "{\"ip\":\"%s\",\"status\":\"%s\",\"uptime\":%lu}",
             currentIP.toString().c_str(), status, millis() / 1000);
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonBuffer);
    response->addHeader("Cache-Control", "max-age=5");
    request->send(response);
}

static void IRAM_ATTR handleApiRestart(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Redémarrage en cours...");
    delay(500);
    ESP.restart();
}

static void IRAM_ATTR handleDashboard(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
        "<html><head><title>Dashboard</title><meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>body{font-family:Arial;margin:0;padding:20px;}</style></head>"
        "<body><h1>Tableau de bord</h1><p>Version optimisée</p></body></html>");
    response->addHeader("Cache-Control", "max-age=3600");
    request->send(response);
}

static void IRAM_ATTR handleFavicon(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(204);
    response->addHeader("Cache-Control", "max-age=86400");
    request->send(response);
}

static void IRAM_ATTR handleNotFound(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(404, "text/html",
        "<html><body><h1>Page non trouvée</h1><p>Ressource non disponible</p></body></html>");
    request->send(response);
}

// Gestion des routes - version optimisée
void setupServerRoutes(AsyncWebServer* server) {
    server->on("/", HTTP_GET, handleRoot);
    server->on("/api/info", HTTP_GET, handleApiInfo);
    server->on("/api/restart", HTTP_POST, handleApiRestart);
    server->on("/dashboard", HTTP_GET, handleDashboard);
    server->on("/favicon.ico", HTTP_GET, handleFavicon);
    server->onNotFound(handleNotFound);
    LOG_INFO("WEBS", "Routes HTTP configurées (mode optimisé)");
}

#else
// Serveur web désactivé à la compilation (MODULE_WEBSERVER_ENABLED=0)
#endif
