/*
  -----------------------
  Kite PiloteV3 - Module Wind Sensor (Implémentation)
  -----------------------
  
  Implémentation du module de gestion du capteur de vent.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/wind.h"
#include "utils/logging.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Variables statiques
static WindData currentWindData = {0};
static bool sensorInitialized = false;
static SemaphoreHandle_t windMutex = NULL;

/**
 * Initialise le capteur de vent
 * @return true si succès, false si échec
 */
bool windInit() {
    // Protection thread-safe
    if (windMutex == NULL) {
        windMutex = xSemaphoreCreateMutex();
        if (windMutex == NULL) {
            LOG_ERROR("WIND", "Échec de création du mutex");
            return false;
        }
    }
    
    // Simulation d'initialisation réussie
    LOG_INFO("WIND", "Initialisation du capteur de vent réussie");
    
    // Initialisation des données de vent avec des valeurs par défaut
    currentWindData.speed = 5.0f;       // 5 m/s
    currentWindData.direction = 180.0f;  // Direction sud
    currentWindData.gust = 7.0f;        // 7 m/s
    currentWindData.timestamp = millis();
    currentWindData.isValid = true;
    
    sensorInitialized = true;
    return true;
}

/**
 * Lit les données actuelles du vent
 * @return Structure WindData avec les mesures actuelles
 */
WindData windRead() {
    WindData result = {0};
    result.isValid = false;
    
    if (!sensorInitialized || windMutex == NULL) {
        LOG_ERROR("WIND", "Capteur non initialisé ou mutex non disponible");
        return result;
    }
    
    if (xSemaphoreTake(windMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        result = currentWindData;
        xSemaphoreGive(windMutex);
    } else {
        LOG_ERROR("WIND", "Impossible d'acquérir le mutex");
    }
    
    return result;
}

/**
 * Calibre le capteur de vent
 * @return true si succès, false si échec
 */
bool windCalibrate() {
    if (!sensorInitialized || windMutex == NULL) {
        LOG_ERROR("WIND", "Capteur non initialisé ou mutex non disponible");
        return false;
    }
    
    LOG_INFO("WIND", "Calibration du capteur de vent");
    return true;
}

/**
 * Vérifie si le capteur de vent est fonctionnel
 * @return true si fonctionnel, false sinon
 */
bool windIsHealthy() {
    return sensorInitialized;
}

/**
 * Mise à jour périodique du capteur de vent
 * Fonction thread-safe compatible avec FreeRTOS
 * @return true si succès, false si échec
 */
bool updateWindSensor() {
    // Vérifier si le capteur est initialisé
    if (!sensorInitialized) {
        static unsigned long lastInitAttempt = 0;
        unsigned long now = millis();
        
        // Tenter une réinitialisation au plus une fois toutes les 5 secondes
        if (now - lastInitAttempt > 5000) {
            lastInitAttempt = now;
            LOG_WARNING("WIND", "Capteur non initialisé, tentative d'initialisation...");
            windInit();
        }
        
        return false;
    }
    
    // Protection thread-safe
    if (windMutex == NULL) {
        LOG_ERROR("WIND", "Mutex non disponible");
        return false;
    }
    
    bool result = false;
    
    if (xSemaphoreTake(windMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Simulation de mise à jour des données
        // Vitesse du vent qui varie entre 3 et 12 m/s
        currentWindData.speed = 7.5f + 4.5f * sin(millis() / 15000.0);
        
        // Direction qui tourne lentement (0-359 degrés)
        currentWindData.direction = fmod((millis() / 30000.0) * 360.0, 360.0);
        
        // Rafale (un peu plus que la vitesse moyenne)
        currentWindData.gust = currentWindData.speed * (1.0f + 0.3f * sin(millis() / 3000.0));
        
        // Horodatage
        currentWindData.timestamp = millis();
        currentWindData.isValid = true;
        
        result = true;
        xSemaphoreGive(windMutex);
    } else {
        LOG_ERROR("WIND", "Impossible d'acquérir le mutex pour la mise à jour");
    }
    
    return result;
}