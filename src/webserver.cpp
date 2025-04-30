#include "../include/kite_webserver.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <SPIFFS.h>

// Variable globale pour déterminer le mode de fonctionnement
bool useSpiffsFiles = false;

// Fonction pour obtenir le port du serveur
uint16_t getServerPort(AsyncWebServer* server) {
    return 80; // Port par défaut, à adapter selon votre configuration
}

// Fonction pour initialiser SPIFFS
bool initSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Erreur lors de l'initialisation de SPIFFS");
        return false;
    }
    Serial.println("SPIFFS initialisé avec succès");
    return true;
}

// Fonction pour définir le mode de fonctionnement
void setWebServerMode(bool useFiles) {
    useSpiffsFiles = useFiles;
    Serial.println(useSpiffsFiles ? "Mode fichiers SPIFFS activé" : "Mode génération de code HTML activé");
}

// Gestion des routes - avec support pour les deux modes
void setupServerRoutes(AsyncWebServer* server) {
    // Initialiser SPIFFS si nécessaire
    if (useSpiffsFiles) {
        if (!initSPIFFS()) {
            Serial.println("Passage au mode génération de code HTML par défaut");
            useSpiffsFiles = false;
        }
    }

    // Route principale - utiliser WebRequestMethod sans les constantes
    server->on("/", [](AsyncWebServerRequest *request){
        if (useSpiffsFiles && SPIFFS.exists("/index.html")) {
            request->send(SPIFFS, "/index.html", "text/html");
        } else {
            request->send(200, "text/html", "Kite PiloteV3 - Serveur Web");
        }
    });
    
    // API pour obtenir les informations du système
    server->on("/api/info", [](AsyncWebServerRequest *request){
        String json = "{\"ip\":\"" + WiFi.localIP().toString() + "\"}";
        request->send(200, "application/json", json);
    });

    // API pour redémarrer le système avec vérification manuelle de la méthode
    server->on("/api/restart", [](AsyncWebServerRequest *request){
        // Vérifier manuellement si la méthode est POST
        if (request->methodToString() == "POST") {
            request->send(200, "text/plain", "Redémarrage en cours...");
            delay(500);
            ESP.restart();
        } else {
            request->send(405, "text/plain", "Méthode non autorisée");
        }
    });

    // Accès à la dashboard HTML
    server->on("/dashboard", [](AsyncWebServerRequest *request){
        if (useSpiffsFiles && SPIFFS.exists("/dashboard.html")) {
            request->send(SPIFFS, "/dashboard.html", "text/html");
        } else {
            request->send(200, "text/html", "Page de tableau de bord non disponible en mode génération de code.");
        }
    });
    
    // Si mode fichiers SPIFFS, servir les fichiers statiques
    if (useSpiffsFiles) {
        // Gestionnaire pour servir les fichiers statiques du dossier data
        server->serveStatic("/", SPIFFS, "/");
        
        Serial.println("Routes pour fichiers statiques configurées");
    }
}
