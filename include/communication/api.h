/*
  -----------------------
  API de Communication du Kite PiloteV3
  -----------------------
  
  Ce module gère l'API REST pour le contrôle et la surveillance du kite.
*/

#ifndef API_H
#define API_H

#include <ESPAsyncWebServer.h>
#include "../../core/config.h"

// === CONSTANTES DE L'API ===
#define API_VERSION "v1"           // Version actuelle de l'API
#define MAX_PAYLOAD_SIZE 1024      // Taille maximale des données JSON (octets)
#define API_RATE_LIMIT 100         // Limite de requêtes par minute

// === CODES D'ÉTAT DE L'API ===
typedef enum {
    API_SUCCESS = 0,               // Opération réussie
    API_ERROR_INVALID_PARAM,       // Paramètres invalides
    API_ERROR_NOT_FOUND,          // Ressource non trouvée
    API_ERROR_UNAUTHORIZED,        // Non autorisé
    API_ERROR_INTERNAL,           // Erreur interne du serveur
    API_ERROR_TIMEOUT             // Délai d'attente dépassé
} ApiStatus;

// === STRUCTURES DE DONNÉES ===

// Structure pour les données de télémétrie
typedef struct {
    float altitude;               // Altitude actuelle (m)
    float speed;                 // Vitesse du kite (m/s)
    float windSpeed;            // Vitesse du vent (m/s)
    float position[3];          // Position [x, y, z] (m)
    float tension;              // Tension des lignes (N)
    uint32_t timestamp;         // Horodatage Unix
} TelemetryData;

// Structure pour la configuration de l'API
typedef struct {
    bool authEnabled;            // Authentification activée
    uint16_t port;              // Port du serveur
    char endpoint[32];          // Point d'entrée de base
    bool corsEnabled;           // CORS activé
    uint16_t timeout;           // Délai d'attente (ms)
} ApiConfig;

// Structure pour les statistiques de l'API
typedef struct {
    uint32_t requestCount;        // Nombre total de requêtes reçues
    uint32_t errorCount;          // Nombre total d'erreurs
} ApiStats;

// === PROTOTYPES DES FONCTIONS ===

// Initialisation de l'API
void apiInit(AsyncWebServer& server);

// Activation/désactivation de l'API
bool apiEnable(bool enabled);

// Configuration de l'API
void apiConfigure(const ApiConfig& config);

// Gestion des requêtes
void handleSystemApi(AsyncWebServerRequest* request);
void handleSensorsApi(AsyncWebServerRequest* request);
void handleControlApi(AsyncWebServerRequest* request);
void handleDiagnosticsApi(AsyncWebServerRequest* request);
void handleModulesApi(AsyncWebServerRequest* request);

// Envoi des données de télémétrie
void sendTelemetryData(const TelemetryData& data);

// Validation des données
bool validateRequest(AsyncWebServerRequest* request);

// Gestion des erreurs
void handleApiError(AsyncWebServerRequest* request, ApiStatus status);

#endif // API_H
