/*
-----------------------
  Kite PiloteV3 - Module serveur web
  -----------------------

  Module de gestion du serveur web pour le système Kite PiloteV3.

  Version: 1.0.0
  Date: 30 avril 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef KITE_WEBSERVER_H
#define KITE_WEBSERVER_H

#include <ESPAsyncWebServer.h>

// Fonction pour obtenir le port du serveur
uint16_t getServerPort(AsyncWebServer* server);

// Fonction pour initialiser SPIFFS (système de fichiers)
bool initSPIFFS();

// Fonction pour définir le mode de fonctionnement du serveur web
// - true: utilise les fichiers du dossier data (SPIFFS)
// - false: génère le contenu HTML directement depuis le code
void setWebServerMode(bool useFiles);

// Configuration des routes du serveur web
void setupServerRoutes(AsyncWebServer* server);

#endif // KITE_WEBSERVER_H
