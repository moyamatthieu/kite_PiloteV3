/*
  -----------------------
  Kite PiloteV3 - Module système (Implémentation)
  -----------------------
  
  Implémentation des fonctions système de base.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

/* === MODULE SYSTÈME ===
   Implémentation des fonctions système de base, y compris l'initialisation,
   la gestion des erreurs, et les états globaux du système.
*/

#include "../../include/core/system.h"
#include "../../include/core/logging.h"
#include <esp_system.h>
#include <Arduino.h>
#include <esp_task_wdt.h>   // Ajout: inclusion pour les fonctions watchdog

// Forward declaration pour la tâche de redémarrage
static void restartTask(void* parameters);

// Variables système globales
static SystemInfo systemInfo;          // Structure contenant les informations système
static unsigned long lastUpdateTime = 0; // Dernière mise à jour des informations système
static bool systemInitialized = false;  // Indique si le système est initialisé
static uint8_t systemStatus = 0;        // État global du système
static TaskHandle_t restartTaskHandle = NULL; // Tâche pour redémarrer le système

// === IMPLÉMENTATION DES FONCTIONS ===

/**
 * Initialise le système et configure le watchdog.
 * @return Code d'erreur système (SYS_OK si succès).
 */
SystemErrorCode systemInit() {
  if (systemInitialized) {
    LOG_WARNING("SYS", "Système déjà initialisé");
    return SYS_ALREADY_INITIALIZED;
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
  
  // Initialiser le statut système
  systemStatus = 100; // 100%
  
  // Configurer le watchdog
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);
  
  // Démarrer le watchdog pour la tâche principale
  esp_task_wdt_add(NULL);
  
  // Marquer le système comme initialisé et passer à l'état READY
  systemInitialized = true;
  systemInfo.systemState = SYS_STATE_READY;
  
  LOG_INFO("SYS", "Système initialisé avec succès");
  return SYS_OK;
}

/**
 * Retourne les informations système actuelles.
 * @return Structure SystemInfo contenant les données système.
 */
SystemInfo getSystemInfo() {
  // Mettre à jour les informations si nécessaire
  if (millis() - lastUpdateTime > 1000) {
    updateSystemInfo();
  }
  
  return systemInfo;
}

/**
 * Met à jour les informations système (mémoire, CPU, etc.).
 */
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

/**
 * Redémarre le système après un délai facultatif.
 * @param delayMs Délai avant redémarrage (en millisecondes).
 */
void systemRestart(unsigned long delayMs) {
  if (delayMs == 0) {
    LOG_INFO("SYS", "Redémarrage immédiat du système");
    delay(100);  // Petit délai pour permettre au log de s'afficher
    ESP.restart();
    return;
  }
  
  // Supprimer l'ancienne tâche de redémarrage si elle existe
  if (restartTaskHandle != NULL) {
    vTaskDelete(restartTaskHandle);
    restartTaskHandle = NULL;
  }
  
  // Créer une nouvelle tâche de redémarrage
  BaseType_t result = xTaskCreate(
    restartTask,            // Fonction de tâche
    "RestartTask",          // Nom de la tâche
    2048,                   // Taille de pile
    (void*)delayMs,         // Paramètre
    1,                      // Priorité
    &restartTaskHandle      // Handle
  );
  
  if (result != pdPASS) {
    LOG_ERROR("SYS", "Échec de création de la tâche de redémarrage");
    // Redémarrer immédiatement en cas d'échec
    delay(100);
    ESP.restart();
  }
}

/**
 * Vérifie si le système est en bonne santé.
 * @return true si le système est sain, false sinon.
 */
bool isSystemHealthy() {
  return (systemStatus >= 80); // Considéré sain si >= 80%
}

/**
 * Retourne une chaîne décrivant l'état global du système.
 * @return Chaîne de caractères représentant l'état.
 */
const char* getSystemStatusString() {
  if (systemStatus >= 90) return "Excellent";
  if (systemStatus >= 80) return "Bon";
  if (systemStatus >= 60) return "Correct";
  if (systemStatus >= 40) return "Attention";
  if (systemStatus >= 20) return "Critique";
  return "Défaillant";
}

/**
 * Alimente les watchdogs pour éviter un redémarrage intempestif.
 */
void feedWatchdogs() {
  // Nourrir le watchdog matériel
  esp_task_wdt_reset();
}

/**
 * Tâche de redémarrage du système.
 * @param parameters Paramètres passés à la tâche (délai en ms).
 */
static void restartTask(void* parameters) {
  unsigned long delayMs = (unsigned long)parameters;
  
  LOG_INFO("SYS", "Redémarrage prévu dans %lu ms", delayMs);
  
  // Attendre le délai spécifié
  vTaskDelay(pdMS_TO_TICKS(delayMs));
  
  // Effectuer le redémarrage
  LOG_INFO("SYS", "Redémarrage du système maintenant...");
  delay(100);  // Petit délai pour permettre au log de s'afficher
  
  ESP.restart();
  
  // Ne devrait jamais arriver ici
  vTaskDelete(NULL);
}

/**
 * Convertit un code d'erreur système en chaîne de caractères.
 * @param errorCode Code d'erreur système.
 * @return Chaîne de caractères décrivant l'erreur.
 */
const char* systemErrorToString(SystemErrorCode code) {
  switch (code) {
    case SYS_OK:
      return "Pas d'erreur";
    case SYS_ALREADY_INITIALIZED:
      return "Système déjà initialisé";
    case SYS_INIT_FAILED:
      return "Échec d'initialisation du système";
    case SYS_OUT_OF_MEMORY:
      return "Mémoire insuffisante";
    case SYS_TIMEOUT:
      return "Délai d'attente dépassé";
    case SYS_NOT_INITIALIZED:
      return "Système non initialisé";
    case SYS_HARDWARE_ERROR:
      return "Erreur matérielle";
    default:
      return "Erreur inconnue";
  }
}

/**
 * Vérifie la santé globale du système.
 * @return true si tout est correct, false sinon.
 */
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

/**
 * Vérifie l'état du matériel et tente une récupération si nécessaire.
 * @return true si tout est correct ou a été récupéré, false sinon.
 */
bool systemCheckHardware() {
  bool allOk = true;
  static unsigned long lastServoCheckTime = 0;
  const unsigned long servoCheckInterval = 10000; // 10 secondes entre les vérifications
  unsigned long currentTime = millis();
  
  // Vérifier l'état des servomoteurs périodiquement
  if (currentTime - lastServoCheckTime > servoCheckInterval) {
    lastServoCheckTime = currentTime;
    
    // Vérifier si les servomoteurs sont correctement initialisés
    if (!servoIsAttached(0) || !servoIsAttached(1) || !servoIsAttached(2)) {
      // Les servomoteurs ne sont pas correctement initialisés, tenter une réinitialisation
      LOG_WARNING("SYS", "Servomoteurs non initialisés, tentative de récupération...");
      
      bool servoRecovered = servoReinitialize();
      if (servoRecovered) {
        LOG_INFO("SYS", "Servomoteurs récupérés avec succès");
      } else {
        LOG_ERROR("SYS", "Échec de récupération des servomoteurs");
        allOk = false;
      }
    }
  }
  
  // Ajouter ici d'autres vérifications matérielles si nécessaire
  
  return allOk;
}

/**
 * Active le mode économie d'énergie.
 */
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

/**
 * Désactive le mode économie d'énergie.
 */
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

/**
 * Retourne la version actuelle du système.
 * @return Chaîne de caractères contenant la version.
 */
const char* getSystemVersion() {
  return SYSTEM_VERSION;
}

/**
 * Vérifie si le système est en état d'erreur.
 * @return true si une erreur est présente, false sinon.
 */
bool hasSystemError() {
  return systemInfo.hasError;
}

/**
 * Efface toutes les erreurs système.
 */
void clearSystemErrors() {
  systemInfo.hasError = false;
  memset(systemInfo.lastErrorMessage, 0, sizeof(systemInfo.lastErrorMessage));
  LOG_INFO("SYS", "Erreurs système effacées");
}

/**
 * Gère une erreur système en enregistrant un message et en activant l'indicateur d'erreur.
 * @param errorSource Source de l'erreur.
 * @param errorMessage Message décrivant l'erreur.
 */
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
