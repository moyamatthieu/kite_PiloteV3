/*
  -----------------------
  Kite PiloteV3 - Module Dashboard (Implémentation)
  -----------------------
  
  Implémentation du tableau de bord du système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../../include/ui/dashboard.h"
#include "../../include/utils/logging.h"
#include "../../include/core/config.h"
#include <Arduino.h>

// Variables statiques du module
static DashboardData dashboardData;
static unsigned long lastUpdateTime = 0;
static bool isInitialized = false;

// === IMPLÉMENTATION DES FONCTIONS ===

bool dashboardInit() {
  if (isInitialized) {
    return true;
  }
  
  // Initialisation des valeurs par défaut
  memset(&dashboardData, 0, sizeof(DashboardData));
  strncpy(dashboardData.statusMessage, "Système initialisé", sizeof(dashboardData.statusMessage) - 1);
  dashboardData.systemStatus = 100;
  
  isInitialized = true;
  lastUpdateTime = millis();
  
  LOG_INFO("DASH", "Module initialisé avec succès");
  return true;
}

bool dashboardUpdate(DashboardUpdateType updateType) {
  if (!isInitialized) {
    LOG_ERROR("DASH", "Tentative de mise à jour sans initialisation");
    return false;
  }
  
  lastUpdateTime = millis();
  
  // Mettre à jour les données selon le type demandé
  switch (updateType) {
    case DASH_UPDATE_FULL:
      // Une mise à jour complète nécessite des données de plusieurs sources
      // Cette fonction appellerait d'autres fonctions pour collecter toutes les données
      break;
      
    case DASH_UPDATE_SYSTEM:
      // Mise à jour des informations système uniquement
      dashboardData.uptime = millis() / 1000;
      dashboardData.freeHeap = ESP.getFreeHeap();
      // Les autres données système seraient collectées ici
      break;
      
    // Autres cas selon les besoins...
      
    default:
      break;
  }
  
  return true;
}

DashboardData dashboardGetData() {
  return dashboardData;
}

bool dashboardUpdateSystem(uint32_t uptime, uint32_t freeHeap, uint8_t cpuUsage, float cpuTemp) {
  if (!isInitialized) {
    return false;
  }
  
  dashboardData.uptime = uptime;
  dashboardData.freeHeap = freeHeap;
  dashboardData.cpuUsage = cpuUsage;
  dashboardData.cpuTemperature = cpuTemp;
  
  return true;
}

bool dashboardUpdateKite(const IMUData& imuData) {
  if (!isInitialized) {
    return false;
  }
  
  // Calculer les données du kite à partir des données IMU
  // Ceci est une implémentation simplifiée
  
  // Exemple: Estimer l'altitude basée sur l'angle et des modèles simples
  // orientation[0] = pitch, orientation[1] = roll, orientation[2] = yaw
  float angleRad = imuData.orientation[0] * (PI / 180.0f);
  float lineLength = dashboardData.lineLength; // Longueur actuelle des lignes
  
  dashboardData.kiteAltitude = sin(angleRad) * lineLength / 100.0f; // cm → m
  
  // Calculer la vitesse du kite en fonction des données gyroscopiques
  // Conversion des taux de rotation en une estimation de vitesse
  // gyro[0] = vitesse angulaire x, gyro[1] = vitesse angulaire y, gyro[2] = vitesse angulaire z
  dashboardData.kiteSpeed = sqrt(sq(imuData.gyro[0]) + sq(imuData.gyro[1]) + sq(imuData.gyro[2])) * 0.01f;
  
  // Puissance estimée basée sur la vitesse et la tension (formule simplifiée)
  dashboardData.kitePower = dashboardData.kiteSpeed * dashboardData.lineTension * 10.0f;
  
  return true;
}

bool dashboardUpdateControl(int16_t directionAngle, int16_t trimAngle, uint16_t lineLength, float lineTension) {
  if (!isInitialized) {
    return false;
  }
  
  dashboardData.directionAngle = directionAngle;
  dashboardData.trimAngle = trimAngle;
  dashboardData.lineLength = lineLength;
  dashboardData.lineTension = lineTension;
  
  return true;
}

bool dashboardUpdatePerformance(float currentPower, uint32_t totalEnergy, float efficiency) {
  if (!isInitialized) {
    return false;
  }
  
  dashboardData.currentPower = currentPower;
  dashboardData.totalEnergy = totalEnergy;
  dashboardData.efficiency = efficiency;
  
  // Mettre à jour le pic de puissance si nécessaire
  if (currentPower > dashboardData.peakPower) {
    dashboardData.peakPower = currentPower;
  }
  
  return true;
}

bool dashboardUpdateAutopilot(AutopilotMode mode, uint8_t confidence) {
  if (!isInitialized) {
    return false;
  }
  
  dashboardData.autopilotMode = mode;
  dashboardData.autopilotConfidence = confidence;
  
  return true;
}

bool dashboardUpdateStatus(uint8_t status, const char* message, bool hasWarning, bool hasError) {
  if (!isInitialized) {
    return false;
  }
  
  dashboardData.systemStatus = status;
  strncpy(dashboardData.statusMessage, message, sizeof(dashboardData.statusMessage) - 1);
  dashboardData.hasWarning = hasWarning;
  dashboardData.hasError = hasError;
  
  return true;
}

String dashboardToJson(DashboardUpdateType updateType) {
  String json = "{";
  
  // Ajouter données système
  if (updateType == DASH_UPDATE_FULL || updateType == DASH_UPDATE_SYSTEM) {
    json += "\"system\":{";
    json += "\"uptime\":" + String(dashboardData.uptime) + ",";
    json += "\"freeHeap\":" + String(dashboardData.freeHeap) + ",";
    json += "\"cpuUsage\":" + String(dashboardData.cpuUsage) + ",";
    json += "\"cpuTemp\":" + String(dashboardData.cpuTemperature);
    json += "},";
  }
  
  // Ajouter données kite
  if (updateType == DASH_UPDATE_FULL || updateType == DASH_UPDATE_KITE) {
    json += "\"kite\":{";
    json += "\"altitude\":" + String(dashboardData.kiteAltitude) + ",";
    json += "\"speed\":" + String(dashboardData.kiteSpeed) + ",";
    json += "\"power\":" + String(dashboardData.kitePower);
    json += "},";
  }
  
  // Ajouter données contrôle
  if (updateType == DASH_UPDATE_FULL || updateType == DASH_UPDATE_CONTROL) {
    json += "\"control\":{";
    json += "\"direction\":" + String(dashboardData.directionAngle) + ",";
    json += "\"trim\":" + String(dashboardData.trimAngle) + ",";
    json += "\"lineLength\":" + String(dashboardData.lineLength) + ",";
    json += "\"tension\":" + String(dashboardData.lineTension);
    json += "},";
  }
  
  // Ajouter données performance
  if (updateType == DASH_UPDATE_FULL || updateType == DASH_UPDATE_PERFORMANCE) {
    json += "\"performance\":{";
    json += "\"currentPower\":" + String(dashboardData.currentPower) + ",";
    json += "\"totalEnergy\":" + String(dashboardData.totalEnergy) + ",";
    json += "\"peakPower\":" + String(dashboardData.peakPower) + ",";
    json += "\"efficiency\":" + String(dashboardData.efficiency);
    json += "},";
  }
  
  // Ajouter données état
  if (updateType == DASH_UPDATE_FULL || updateType == DASH_UPDATE_STATUS) {
    json += "\"status\":{";
    json += "\"systemStatus\":" + String(dashboardData.systemStatus) + ",";
    json += "\"message\":\"" + String(dashboardData.statusMessage) + "\",";
    json += "\"autopilotMode\":" + String(dashboardData.autopilotMode) + ",";
    json += "\"autopilotConfidence\":" + String(dashboardData.autopilotConfidence) + ",";
    json += "\"hasWarning\":" + String(dashboardData.hasWarning ? "true" : "false") + ",";
    json += "\"hasError\":" + String(dashboardData.hasError ? "true" : "false");
    json += "}";
  }
  
  // Supprimer la virgule finale si nécessaire
  if (json.endsWith(",")) {
    json.remove(json.length() - 1);
  }
  
  json += "}";
  return json;
}
