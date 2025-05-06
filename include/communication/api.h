/*
  -----------------------
  Kite PiloteV3 - Module API (Interface)
  -----------------------
  
  Interface REST API pour la communication avec le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "../../core/config.h"

// === DÉFINITION DES TYPES ===

// États de l'API
typedef enum {
  API_STATE_DISABLED = 0,       // API désactivée
  API_STATE_ENABLED = 1,        // API activée
  API_STATE_AUTH_REQUIRED = 2,  // API activée avec authentification
  API_STATE_RESTRICTED = 3      // API en mode restreint (lecture seule)
} ApiState;

// Structure pour les statistiques de l'API
typedef struct {
  uint32_t requestCount;        // Nombre total de requêtes
  uint32_t errorCount;          // Nombre d'erreurs
  uint32_t lastRequestTime;     // Horodatage de la dernière requête
  uint32_t maxResponseTime;     // Temps de réponse maximal (ms)
  uint32_t avgResponseTime;     // Temps de réponse moyen (ms)
  uint32_t bytesReceived;       // Octets reçus
  uint32_t bytesSent;           // Octets envoyés
} ApiStats;

// === DÉCLARATION DES FONCTIONS ===

/**
 * Initialise l'API REST
 * @param server Serveur web asynchrone
 * @return true si l'initialisation réussit, false sinon
 */
bool apiInit(AsyncWebServer& server);

/**
 * Active ou désactive l'API
 * @param enabled True pour activer, false pour désactiver
 * @return true si l'opération réussit, false sinon
 */
bool apiEnable(bool enabled);

/**
 * Obtient l'état d'activation de l'API
 * @return True si l'API est activée, false sinon
 */
bool isApiEnabled();

/**
 * Configure l'authentification pour l'API
 * @param username Nom d'utilisateur
 * @param password Mot de passe
 * @return true si succès, false si échec
 */
bool apiSetAuth(const char* username, const char* password);

/**
 * Définit les permissions pour un endpoint spécifique
 * @param endpoint Chemin de l'endpoint
 * @param method Méthode HTTP (GET, POST, etc.)
 * @param requireAuth Requiert une authentification si true
 * @return true si succès, false si échec
 */
bool apiSetPermission(const char* endpoint, const char* method, bool requireAuth);

/**
 * Obtient les statistiques de l'API
 * @return Structure contenant les statistiques
 */
ApiStats apiGetStats();

/**
 * Gère une requête API
 * @param request Objet de requête
 * @param callback Fonction de callback à appeler avec la réponse
 * @return true si la requête a été traitée, false sinon
 */
bool apiHandleRequest(AsyncWebServerRequest* request, void (*callback)(AsyncWebServerResponse*));

/**
 * Réinitialise les statistiques de l'API
 */
void apiResetStats();

/**
 * Génère une réponse JSON à partir d'une structure
 * @param data Pointeur vers la structure de données
 * @param type Type de la structure
 * @return Chaîne JSON
 */
String apiGenerateJsonResponse(void* data, const char* type);

/**
 * Enregistre un endpoint API personnalisé
 * @param endpoint Chemin de l'endpoint
 * @param method Méthode HTTP
 * @param handler Fonction de gestionnaire
 * @return true si succès, false si échec
 */
bool apiRegisterEndpoint(const char* endpoint, const char* method, ArRequestHandlerFunction handler);

/**
 * Gère les requêtes API pour le système
 * @param request Requête web asynchrone
 */
void handleSystemApi(AsyncWebServerRequest* request);

/**
 * Gère les requêtes API pour les capteurs
 * @param request Requête web asynchrone
 */
void handleSensorsApi(AsyncWebServerRequest* request);

/**
 * Gère les requêtes API pour le contrôle
 * @param request Requête web asynchrone
 */
void handleControlApi(AsyncWebServerRequest* request);

/**
 * Gère les requêtes API pour le diagnostic
 * @param request Requête web asynchrone
 */
void handleDiagnosticsApi(AsyncWebServerRequest* request);