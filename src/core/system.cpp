/*
  -----------------------
  Kite PiloteV3 - Module système (Implémentation)
  -----------------------
  
  Implémentation des fonctions système de base pour Kite PiloteV3.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../../include/core/system.h"
#include "../../include/utils/logging.h"
#include <esp_system.h>

// Variables statiques
static SystemInfo systemInfo;
static unsigned long lastUpdateTime = 0;
static bool systemInitialized = false;

// === IMPLÉMENTATION DES FONCTIONS ===

bool systemInit() {
  if (systemInitialized) {
    return true;
  }
  
  // Initialiser la structure des informations système
  memset(&systemInfo, 0, sizeof(SystemInfo));
  systemInfo.systemState = SYS_STATE_INIT;
  systemInfo.uptimeSeconds = 0;
  systemInfo.freeHeapBytes = ESP.getFreeHeap();
  systemInfo.cpuUsagePercent = 0;
  systemInfo.cpuTemperature = 0.0f;
  systemInfo.batteryVoltage = 0;
  systemInfo.batteryPercent = 0;
  systemInfo.hasError = false;
  systemInfo.hasWarning = false;
  memset(systemInfo.lastErrorMessage, 0, sizeof(systemInfo.lastErrorMessage));
  
  // Marquer le système comme initialisé et passer à l'état READY
  systemInitialized = true;
  systemInfo.systemState = SYS_STATE_READY;
  
  LOG_INFO("SYS", "Système initialisé avec succès");
  return true;
}

SystemInfo getSystemInfo() {
  // Mettre à jour les informations si nécessaire
  if (millis() - lastUpdateTime > 1000) {
    updateSystemInfo();
  }
  
  return systemInfo;
}

void updateSystemInfo() {
  // Mettre à jour le temps d'activité (en secondes)
  systemInfo.uptimeSeconds = millis() / 1000;
  
  // Mettre à jour la mémoire disponible
  systemInfo.freeHeapBytes = ESP.getFreeHeap();
  
  // Simuler des valeurs pour l'utilisation CPU et la température
  // Dans une implémentation réelle, ces valeurs seraient obtenues des capteurs
  systemInfo.cpuUsagePercent = random(5, 30); // Simulation de 5-30% d'utilisation CPU
  systemInfo.cpuTemperature = 30.0f + (random(0, 100) / 10.0f); // Simulation de température 30-40°C
  
  // Mettre à jour le timestamp de dernière mise à jour
  lastUpdateTime = millis();
}

void systemRestart() {
  LOG_WARNING("SYS", "Redémarrage du système demandé");
  
  // Attendre un peu pour que le message puisse être transmis
  delay(100);
  
  // Redémarrer l'ESP32
  ESP.restart();
}

bool systemHealthCheck() {
  // Vérifier la mémoire disponible
  if (systemInfo.freeHeapBytes < 10000) {
    handleSystemError("SYS", "Mémoire critique");
    return false;
  }
  
  // Vérifier la température
  if (systemInfo.cpuTemperature > 70.0f) {
    handleSystemError("SYS", "Température CPU critique");
    return false;
  }
  
  // Vérifier le niveau de batterie
  if (systemInfo.batteryPercent < 10 && systemInfo.batteryVoltage > 0) {
    systemInfo.hasWarning = true;
    LOG_WARNING("SYS", "Niveau de batterie faible: %d%%", systemInfo.batteryPercent);
  } else {
    // Tout va bien, effacer l'avertissement si présent
    if (systemInfo.hasWarning) {
      systemInfo.hasWarning = false;
    }
  }
  
  return true;
}

void enterPowerSaveMode() {
  if (systemInfo.systemState == SYS_STATE_POWER_SAVE) {
    return; // Déjà en mode économie d'énergie
  }
  
  LOG_INFO("SYS", "Activation du mode économie d'énergie");
  
  // Code pour réduire la consommation d'énergie
  // - Réduire la fréquence CPU
  // - Désactiver les périphériques non essentiels
  
  systemInfo.systemState = SYS_STATE_POWER_SAVE;
}

void exitPowerSaveMode() {
  if (systemInfo.systemState != SYS_STATE_POWER_SAVE) {
    return; // Pas en mode économie d'énergie
  }
  
  LOG_INFO("SYS", "Désactivation du mode économie d'énergie");
  
  // Code pour revenir au mode normal
  // - Restaurer la fréquence CPU
  // - Réactiver les périphériques
  
  systemInfo.systemState = SYS_STATE_RUNNING;
}

const char* getSystemVersion() {
  return SYSTEM_VERSION;
}

bool hasSystemError() {
  return systemInfo.hasError;
}

void clearSystemErrors() {
  systemInfo.hasError = false;
  memset(systemInfo.lastErrorMessage, 0, sizeof(systemInfo.lastErrorMessage));
  LOG_INFO("SYS", "Erreurs système effacées");
}

void handleSystemError(const char* errorSource, const char* errorMessage) {
  systemInfo.hasError = true;
  
  // Copier le message d'erreur
  snprintf(systemInfo.lastErrorMessage, sizeof(systemInfo.lastErrorMessage),
           "[%s] %s", errorSource, errorMessage);
  
  // Passer à l'état d'erreur
  systemInfo.systemState = SYS_STATE_ERROR;
  
  // Logger l'erreur
  LOG_ERROR("SYS", "Erreur système: %s", systemInfo.lastErrorMessage);
}