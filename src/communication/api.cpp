/*
  -----------------------
  Kite PiloteV3 - Module API (Implémentation)
  -----------------------
  
  Implémentation de l'API REST pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 9 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "communication/api_manager.h"
#include "utils/logging.h"
#include "core/module.h"
#include <ArduinoJson.h>

// Implémentation des fonctions de l'API

void apiInit(AsyncWebServer& server) {
    LOG_INFO("API", "Initialisation de l'API REST");
    
    // Enregistrement des points d'entrée de l'API
    server.on("/api/v1/system", HTTP_GET, handleSystemApi);
    server.on("/api/v1/sensors", HTTP_GET, handleSensorsApi);
    server.on("/api/v1/control", HTTP_GET, handleControlApi);
    server.on("/api/v1/diagnostics", HTTP_GET, handleDiagnosticsApi);
    server.on("/api/v1/modules", HTTP_GET, handleModulesApi);
    
    LOG_INFO("API", "Points d'entrée de l'API enregistrés");
}

bool apiEnable(bool enabled) {
    LOG_INFO("API", "API %s", enabled ? "activée" : "désactivée");
    return true;
}

void apiConfigure(const ApiConfig& config) {
    LOG_INFO("API", "Configuration de l'API appliquée (port: %d)", config.port);
}

void handleSystemApi(AsyncWebServerRequest* request) {
    if (!validateRequest(request)) {
        handleApiError(request, API_ERROR_UNAUTHORIZED);
        return;
    }
    
    // Création de la réponse JSON
    DynamicJsonDocument doc(512);
    doc["status"] = "ok";
    doc["version"] = "1.0.0";
    doc["uptime"] = millis() / 1000;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void handleSensorsApi(AsyncWebServerRequest* request) {
    if (!validateRequest(request)) {
        handleApiError(request, API_ERROR_UNAUTHORIZED);
        return;
    }
    
    // Création de la réponse JSON
    DynamicJsonDocument doc(1024);
    doc["status"] = "ok";
    
    // Ajout des données des capteurs (à implémenter avec les données réelles)
    JsonObject sensors = doc.createNestedObject("sensors");
    sensors["imu"]["pitch"] = 0.0;
    sensors["imu"]["roll"] = 0.0;
    sensors["imu"]["yaw"] = 0.0;
    sensors["wind"]["speed"] = 0.0;
    sensors["wind"]["direction"] = 0.0;
    sensors["tension"] = 0.0;
    sensors["lineLength"] = 0.0;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void handleControlApi(AsyncWebServerRequest* request) {
    if (!validateRequest(request)) {
        handleApiError(request, API_ERROR_UNAUTHORIZED);
        return;
    }
    
    // Création de la réponse JSON
    DynamicJsonDocument doc(512);
    doc["status"] = "ok";
    
    // Ajout des données de contrôle (à implémenter avec les données réelles)
    doc["autopilot"] = false;
    doc["mode"] = "manual";
    doc["servoPosition"] = 0.0;
    doc["winchPosition"] = 0.0;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void handleDiagnosticsApi(AsyncWebServerRequest* request) {
    if (!validateRequest(request)) {
        handleApiError(request, API_ERROR_UNAUTHORIZED);
        return;
    }
    
    // Création de la réponse JSON
    DynamicJsonDocument doc(1024);
    doc["status"] = "ok";
    
    // Ajout des données de diagnostic (à implémenter avec les données réelles)
    doc["memory"]["free"] = ESP.getFreeHeap();
    doc["memory"]["total"] = ESP.getHeapSize();
    doc["cpu"]["temperature"] = 0.0;
    doc["wifi"]["rssi"] = WiFi.RSSI();
    doc["wifi"]["ip"] = WiFi.localIP().toString();
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void handleModulesApi(AsyncWebServerRequest* request) {
    if (!validateRequest(request)) {
        handleApiError(request, API_ERROR_UNAUTHORIZED);
        return;
    }
    
    // Création de la réponse JSON
    DynamicJsonDocument doc(2048);
    doc["status"] = "ok";
    
    // Ajout des données des modules (à implémenter avec les données réelles)
    JsonArray modules = doc.createNestedArray("modules");
    
    // Parcourir tous les modules enregistrés
    String response;
    
    // Vérifier si le registre de modules est disponible
    if (ModuleRegistry::instance().modules().size() > 0) {
        for (Module* m : ModuleRegistry::instance().modules()) {
            JsonObject module = modules.createNestedObject();
            module["name"] = m->name();
            module["state"] = m->stateString();
            module["description"] = m->description();
        }
    } else {
        // Ajouter un message si aucun module n'est enregistré
        doc["message"] = "Aucun module enregistré";
    }
    
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

bool validateRequest(AsyncWebServerRequest* request) {
    // Vérification de l'authentification (à implémenter)
    return true;
}

void handleApiError(AsyncWebServerRequest* request, ApiStatus status) {
    // Création de la réponse d'erreur JSON
    DynamicJsonDocument doc(256);
    doc["status"] = "error";
    
    switch (status) {
        case API_ERROR_INVALID_PARAM:
            doc["code"] = "invalid_param";
            doc["message"] = "Paramètres invalides";
            request->send(400, "application/json", doc.as<String>());
            break;
        case API_ERROR_NOT_FOUND:
            doc["code"] = "not_found";
            doc["message"] = "Ressource non trouvée";
            request->send(404, "application/json", doc.as<String>());
            break;
        case API_ERROR_UNAUTHORIZED:
            doc["code"] = "unauthorized";
            doc["message"] = "Non autorisé";
            request->send(401, "application/json", doc.as<String>());
            break;
        case API_ERROR_INTERNAL:
            doc["code"] = "internal_error";
            doc["message"] = "Erreur interne du serveur";
            request->send(500, "application/json", doc.as<String>());
            break;
        case API_ERROR_TIMEOUT:
            doc["code"] = "timeout";
            doc["message"] = "Délai d'attente dépassé";
            request->send(504, "application/json", doc.as<String>());
            break;
        default:
            doc["code"] = "unknown_error";
            doc["message"] = "Erreur inconnue";
            request->send(500, "application/json", doc.as<String>());
            break;
    }
}

void sendTelemetryData(const TelemetryData& data) {
    // Implémentation à compléter
    LOG_INFO("API", "Envoi des données de télémétrie");
}