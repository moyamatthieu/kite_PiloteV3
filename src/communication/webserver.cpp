#include "communication/kite_webserver.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include "fallback_html.h"
#include "../include/config.h"
#include "core/config.h"

// Mise en cache des valeurs pour éviter les régénérations fréquentes
static char jsonBuffer[256]; // Buffer statique pour JSON
static String lastHtmlContent; // Mise en cache du contenu HTML
static unsigned long lastHtmlGenTime = 0;
static IPAddress lastCachedIP;
static String lastStatus;

// Fonction pour obtenir le port du serveur - optimisée
uint16_t getServerPort(AsyncWebServer* server) {
    return SERVER_PORT; // Utiliser la constante définie dans config.h
}

// Fonction pour définir le mode de fonctionnement
void setWebServerMode(bool useFiles) {
    // Toujours mode HTML généré
    Serial.println("Mode génération de code HTML optimisé activé");
}

// Fonction interne pour générer le HTML ou utiliser la version mise en cache
String getHtmlContent() {
    unsigned long now = millis();
    bool ipChanged = WiFi.localIP() != lastCachedIP;
    bool statusMayHaveChanged = (now - lastHtmlGenTime) > 5000; // Vérifier toutes les 5 secondes
    
    // Ne régénérer que si nécessaire (IP changée ou contenu initial vide)
    if (lastHtmlContent.length() == 0 || ipChanged || statusMayHaveChanged) {
        // Générer le contenu HTML avec substitutions
        String htmlContent = String(fallbackHtml);
        String status = getSystemStatusString();
        String ip = WiFi.localIP().toString();
        
        htmlContent.replace("{status}", status);
        htmlContent.replace("{ip}", ip);
        
        // Mettre à jour les variables de cache
        lastHtmlContent = htmlContent;
        lastHtmlGenTime = now;
        lastCachedIP = WiFi.localIP();
        lastStatus = status;
        
        return htmlContent;
    }
    
    // Retourner la version mise en cache
    return lastHtmlContent;
}

// Gestion des routes - version optimisée
void setupServerRoutes(AsyncWebServer* server) {
    // Route principale avec mise en cache
    server->on("/", [](AsyncWebServerRequest *request){
        // Utiliser la fonction optimisée pour obtenir le HTML
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", getHtmlContent());
        
        // Ajouter des en-têtes de cache pour réduire les requêtes
        response->addHeader("Cache-Control", "max-age=30");  // Cache côté client pendant 30 secondes
        request->send(response);
    });
    
    // API pour obtenir les informations du système - optimisée avec mise en cache
    server->on("/api/info", [](AsyncWebServerRequest *request){
        // Vérifier si l'IP ou le statut ont changé avant de régénérer
        IPAddress currentIP = WiFi.localIP();
        String currentStatus = getSystemStatusString();
        
        if (currentIP != lastCachedIP || currentStatus != lastStatus) {
            // Mettre à jour le cache seulement si nécessaire
            snprintf(jsonBuffer, sizeof(jsonBuffer), 
                     "{\"ip\":\"%s\", \"status\":\"%s\", \"uptime\":\"%lu\"}", 
                     currentIP.toString().c_str(), 
                     currentStatus.c_str(),
                     millis() / 1000);
            
            lastCachedIP = currentIP;
            lastStatus = currentStatus;
        }
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonBuffer);
        response->addHeader("Cache-Control", "max-age=5");  // Cache court pour les données dynamiques
        request->send(response);
    });

    // API pour redémarrer le système avec gestion des erreurs
    server->on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Redémarrage en cours...");
        // Utiliser un délai avant le redémarrage pour permettre l'envoi de la réponse
        delay(500);
        ESP.restart();
    });

    // Accès à la dashboard HTML avec mise en cache statique
    server->on("/dashboard", [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", 
            "<html><head><title>Dashboard</title><meta name='viewport' content='width=device-width, initial-scale=1'>"
            "<style>body{font-family:Arial;margin:0;padding:20px;}</style></head>"
            "<body><h1>Tableau de bord</h1><p>Version optimisée</p></body></html>");
        
        // Mise en cache longue durée pour le contenu statique
        response->addHeader("Cache-Control", "max-age=3600");  // Cache pendant 1 heure
        request->send(response);
    });
    
    // Gérer les requêtes de favicon pour éviter les requêtes inutiles
    server->on("/favicon.ico", [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(204); // No Content
        response->addHeader("Cache-Control", "max-age=86400"); // Cache pendant 24 heures
        request->send(response);
    });
    
    // Intercepter les requêtes 404 pour économiser des ressources
    server->onNotFound([](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(404, "text/html", 
            "<html><body><h1>Page non trouvée</h1><p>Ressource non disponible</p></body></html>");
        request->send(response);
    });
    
    Serial.println("Routes HTTP configurées en mode optimisé");
}
