/*
  -----------------------
  Kite PiloteV3 - Module Tension Sensor (Implémentation)
  -----------------------
  
  Implémentation du module de gestion du capteur de tension.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/tension.h"
#include "utils/logging.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Variables statiques
static float currentTension = 0.0f;
static bool sensorInitialized = false;
static SemaphoreHandle_t tensionMutex = NULL;

/**
 * Initialise le capteur de tension
 * @return true si succès, false si échec
 */
bool tensionInit() {
    // Protection thread-safe
    if (tensionMutex == NULL) {
        tensionMutex = xSemaphoreCreateMutex();
        if (tensionMutex == NULL) {
            LOG_ERROR("TENSION", "Échec de création du mutex");
            return false;
        }
    }
    
    // Simulation d'initialisation réussie
    LOG_INFO("TENSION", "Initialisation du capteur de tension réussie");
    
    sensorInitialized = true;
    return true;
}

/**
 * Lit la tension actuelle
 * @return Tension en Newtons (ou -1 si erreur)
 */
float tensionRead() {
    if (!sensorInitialized || tensionMutex == NULL) {
        LOG_ERROR("TENSION", "Capteur non initialisé ou mutex non disponible");
        return -1.0f;
    }
    
    float result = -1.0f;
    
    if (xSemaphoreTake(tensionMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        result = currentTension;
        xSemaphoreGive(tensionMutex);
    } else {
        LOG_ERROR("TENSION", "Impossible d'acquérir le mutex");
    }
    
    return result;
}

/**
 * Calibre le capteur de tension
 * @param referenceWeight Poids de référence en Newtons
 * @return true si succès, false si échec
 */
bool tensionCalibrate(float referenceWeight) {
    if (!sensorInitialized || tensionMutex == NULL) {
        LOG_ERROR("TENSION", "Capteur non initialisé ou mutex non disponible");
        return false;
    }
    
    LOG_INFO("TENSION", "Calibration du capteur de tension avec référence: %.2f N", referenceWeight);
    return true;
}

/**
 * Vérifie si le capteur de tension est fonctionnel
 * @return true si fonctionnel, false sinon
 */
bool tensionIsHealthy() {
    return sensorInitialized;
}

/**
 * Mise à jour périodique du capteur de tension
 * Fonction thread-safe compatible avec FreeRTOS
 * @return true si succès, false si échec
 */
bool updateTensionSensor() {
    // Vérifier si le capteur est initialisé
    if (!sensorInitialized) {
        static unsigned long lastInitAttempt = 0;
        unsigned long now = millis();
        
        // Tenter une réinitialisation au plus une fois toutes les 5 secondes
        if (now - lastInitAttempt > 5000) {
            lastInitAttempt = now;
            LOG_WARNING("TENSION", "Capteur non initialisé, tentative d'initialisation...");
            tensionInit();
        }
        
        return false;
    }
    
    // Protection thread-safe
    if (tensionMutex == NULL) {
        LOG_ERROR("TENSION", "Mutex non disponible");
        return false;
    }
    
    bool result = false;
    
    if (xSemaphoreTake(tensionMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Simulation de mise à jour des données
        // Générer une tension entre 10 et 50 N qui varie selon les conditions
        currentTension = 30.0f + 20.0f * sin(millis() / 5000.0);
        
        result = true;
        xSemaphoreGive(tensionMutex);
    } else {
        LOG_ERROR("TENSION", "Impossible d'acquérir le mutex pour la mise à jour");
    }
    
    return result;
}