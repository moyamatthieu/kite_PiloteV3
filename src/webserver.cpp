#include "../include/webserver.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>

// Fonction pour obtenir le port du serveur - implémentation correcte sans utiliser getPort()
uint16_t getServerPort(AsyncWebServer* server) {
    // La classe AsyncWebServer n'a pas de méthode getPort()
    // Nous utilisons une variable globale ou un paramètre constant à la place
    return 80; // Port par défaut, à adapter selon votre configuration
}

// Gestion des routes - utilisation correcte des constantes HTTP
void setupServerRoutes(AsyncWebServer* server) {
    // Utilisation de la constante HTTP_GET correcte définie dans ESPAsyncWebServer.h
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Kite PiloteV3 - Serveur Web");
    });
    
    // Ajout d'autres routes si nécessaire
    server->on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Système en ligne");
    });
}