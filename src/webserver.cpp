#include "../include/kite_webserver.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include "fallback_html.h"

// Fonction pour obtenir le port du serveur
uint16_t getServerPort(AsyncWebServer* server) {
    return 80; // Port par défaut, à adapter selon votre configuration
}

// Fonction pour définir le mode de fonctionnement
// Cette fonction est simplifiée car on n'utilise plus le mode SPIFFS
void setWebServerMode(bool useFiles) {
    // Toujours mode HTML généré
    Serial.println("Mode génération de code HTML activé");
}

// Gestion des routes - simplifié sans SPIFFS
void setupServerRoutes(AsyncWebServer* server) {
    // Route principale
    server->on("/", [](AsyncWebServerRequest *request){
        // Utilisation du HTML généré directement avec gestion dynamique
        String htmlContent = String(fallbackHtml);
        htmlContent.replace("{status}", getSystemStatusString());
        htmlContent.replace("{ip}", WiFi.localIP().toString());
        request->send(200, "text/html", htmlContent);
    });
    
    // API pour obtenir les informations du système
    server->on("/api/info", [](AsyncWebServerRequest *request){
        // Créer dynamiquement la chaîne JSON pour éviter des problèmes d'allocation de mémoire
        char jsonBuffer[256]; // Buffer statique assez grand pour le JSON
        snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"ip\":\"%s\", \"status\":\"%s\"}", 
                 WiFi.localIP().toString().c_str(), 
                 getSystemStatusString().c_str());
        request->send(200, "application/json", jsonBuffer);
    });

    // API pour redémarrer le système avec gestion des erreurs
    server->on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Redémarrage en cours...");
        ESP.restart();
    });

    // Accès à la dashboard HTML
    server->on("/dashboard", [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "Page de tableau de bord disponible en mode simple.");
    });
    
    Serial.println("Routes HTTP configurées en mode génération de code HTML");
}
