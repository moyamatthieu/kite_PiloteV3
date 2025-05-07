/*
  -----------------------
  Kite PiloteV3 - Module serveur web
  -----------------------

  Module de gestion du serveur web pour le système Kite PiloteV3.
  Version optimisée avec mise en cache et contrôle des performances.

  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef KITE_WEBSERVER_H
#define KITE_WEBSERVER_H

#include <ESPAsyncWebServer.h>

// Fonction pour obtenir le port du serveur - version optimisée
uint16_t getServerPort(AsyncWebServer* server);

// Fonction pour définir le mode de fonctionnement du serveur web
// En mode optimisé, le mode est toujours HTML généré
void setWebServerMode(bool useFiles);

// Configuration des routes du serveur web avec optimisations
void setupServerRoutes(AsyncWebServer* server);

// Fonction optimisée pour obtenir l'état du système sous forme de chaîne
// Utilise une mise en cache interne pour éviter les régénérations fréquentes
String getSystemStatusString();

#endif // KITE_WEBSERVER_H
