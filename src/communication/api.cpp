/*
  -----------------------
  Kite PiloteV3 - Module API (Implémentation)
  -----------------------
  
  Implémentation de l'API REST pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "communication/api.h"
#include "utils/logging.h"
#include "core/system.h"
#include <ArduinoJson.h>

// Variables statiques du module
static bool apiEnabled = false;
static bool authRequired = false;
static ApiStats apiStats;
static AsyncWebServer* webServer = nullptr;

/**
 * Initialise l'API REST
 * @param server Référence au serveur web asynchrone
 */
void apiInit(AsyncWebServer& server) {
  LOG_INFO("API", "Initialisation de l'API REST");
  
  // Stocker une référence au serveur
  webServer = &server;
  
  // Réinitialiser les statistiques
  memset(&apiStats, 0, sizeof(ApiStats));
  
  // Configurer les routes API
  server.on("/api/v1/system", HTTP_GET, handleSystemApi);
  server.on("/api/v1/sensors", HTTP_GET, handleSensorsApi);
  server.on("/api/v1/control", HTTP_ANY, handleControlApi);
  server.on("/api/v1/diagnostics", HTTP_GET, handleDiagnosticsApi);
  
  LOG_INFO("API", "API REST initialisée avec succès");
}

/**
 * Active ou désactive l'API
 * @param enabled true pour activer, false pour désactiver
 * @return true si l'activation a réussi, false sinon
 */
bool apiEnable(bool enabled) {
  apiEnabled = enabled;
  LOG_INFO("API", "API %s", enabled ? "activée" : "désactivée");
  return true;
}

/**
 * Vérifie si l'API est activée
 * @return true si activée, false sinon
 */
bool isApiEnabled() {
  return apiEnabled;
}

/**
 * Retourne le statut du système sous forme de chaîne
 * @return Chaîne décrivant l'état du système
 */
static const char* getApiSystemStatus() {
  int status = random(0, 5);
  switch (status) {
    case 0: return "Excellent";
    case 1: return "Bon";
    case 2: return "Normal";
    case 3: return "Attention";
    case 4: return "Critique";
    default: return "Inconnu";
  }
}

/**
 * Gère les requêtes API pour le système
 * @param request Requête web asynchrone
 */
void handleSystemApi(AsyncWebServerRequest* request) {
  if (!apiEnabled) {
    request->send(503, "application/json", "{\"error\":\"API désactivée\"}");
    return;
  }
  
  JsonDocument doc;
  doc["system"]["version"] = "3.0.0";
  doc["system"]["uptime"] = millis() / 1000;
  doc["system"]["heap"] = ESP.getFreeHeap();
  doc["system"]["status"] = getApiSystemStatus();
  doc["system"]["healthy"] = systemHealthCheck();
  
  doc["wifi"]["connected"] = (WiFi.status() == WL_CONNECTED);
  doc["wifi"]["ssid"] = WiFi.SSID();
  doc["wifi"]["rssi"] = WiFi.RSSI();
  doc["wifi"]["ip"] = WiFi.localIP().toString();
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
  
  apiStats.requestCount++;
}

/**
 * Gère les requêtes API pour les capteurs
 * @param request Requête web asynchrone
 */
void handleSensorsApi(AsyncWebServerRequest* request) {
  if (!apiEnabled) {
    request->send(503, "application/json", "{\"error\":\"API désactivée\"}");
    return;
  }
  
  JsonDocument doc;
  doc["imu"]["roll"] = random(-45, 45);
  doc["imu"]["pitch"] = random(-30, 30);
  doc["imu"]["yaw"] = random(0, 359);
  
  doc["line"]["length"] = random(10, 100);
  doc["line"]["tension"] = random(5, 20);
  
  doc["wind"]["speed"] = random(10, 40);
  doc["wind"]["direction"] = random(0, 359);
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
  
  apiStats.requestCount++;
}

/**
 * Gère les requêtes API pour le contrôle
 * @param request Requête web asynchrone
 */
void handleControlApi(AsyncWebServerRequest* request) {
  if (!apiEnabled) {
    request->send(503, "application/json", "{\"error\":\"API désactivée\"}");
    return;
  }
  
  if (request->method() == HTTP_GET) {
    JsonDocument doc;
    doc["direction"] = random(-100, 100);
    doc["trim"] = random(-20, 20);
    doc["lineLength"] = random(10, 100);
    doc["autopilot"] = (random(0, 2) == 1);
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  }
  else if (request->method() == HTTP_POST) {
    request->send(200, "application/json", "{\"result\":\"success\",\"message\":\"Commande reçue\"}");
  }
  else {
    request->send(405, "application/json", "{\"error\":\"Méthode non supportée\"}");
  }
  
  apiStats.requestCount++;
}

/**
 * Gère les requêtes API pour le diagnostic
 * @param request Requête web asynchrone
 */
void handleDiagnosticsApi(AsyncWebServerRequest* request) {
  if (!apiEnabled) {
    request->send(503, "application/json", "{\"error\":\"API désactivée\"}");
    return;
  }
  
  JsonDocument doc;
doc["api"]["requests"] = apiStats.requestCount;
doc["api"]["errors"] = apiStats.errorCount;
  
  doc["memory"]["free"] = ESP.getFreeHeap();
  doc["memory"]["total"] = ESP.getHeapSize();
  doc["memory"]["minFree"] = ESP.getMinFreeHeap();
  
  doc["cpu"]["freq"] = ESP.getCpuFreqMHz();
  doc["cpu"]["temp"] = random(30, 50);
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
  
  apiStats.requestCount++;
}
