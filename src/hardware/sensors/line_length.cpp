/*
  -----------------------
  Kite PiloteV3 - Module Line Length Sensor (Implémentation)
  -----------------------
  
  Implémentation du module de gestion du capteur de longueur de ligne.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/line_length.h"
#include "utils/logging.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Variables statiques
static int currentLength = 0;
static bool sensorInitialized = false;
static SemaphoreHandle_t lineLengthMutex = NULL;

/**
 * Initialise le capteur de longueur de ligne
 * @return true si succès, false si échec
 */
bool lineLengthInit() {
    // Protection thread-safe
    if (lineLengthMutex == NULL) {
        lineLengthMutex = xSemaphoreCreateMutex();
        if (lineLengthMutex == NULL) {
            LOG_ERROR("LINE", "Échec de création du mutex");
            return false;
        }
    }
    
    // Simulation d'initialisation réussie
    LOG_INFO("LINE", "Initialisation du capteur de longueur de ligne réussie");
    
    sensorInitialized = true;
    return true;
}

/**
 * Lit la longueur de ligne actuelle
 * @return Longueur de ligne en centimètres (ou -1 si erreur)
 */
int lineLengthRead() {
    if (!sensorInitialized || lineLengthMutex == NULL) {
        LOG_ERROR("LINE", "Capteur non initialisé ou mutex non disponible");
        return -1;
    }
    
    int result = -1;
    
    if (xSemaphoreTake(lineLengthMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        result = currentLength;
        xSemaphoreGive(lineLengthMutex);
    } else {
        LOG_ERROR("LINE", "Impossible d'acquérir le mutex");
    }
    
    return result;
}

/**
 * Calibre le capteur de longueur de ligne
 * @param minLength Longueur minimale en cm
 * @param maxLength Longueur maximale en cm
 * @return true si succès, false si échec
 */
bool lineLengthCalibrate(int minLength, int maxLength) {
    if (!sensorInitialized || lineLengthMutex == NULL) {
        LOG_ERROR("LINE", "Capteur non initialisé ou mutex non disponible");
        return false;
    }
    
    LOG_INFO("LINE", "Calibration du capteur de longueur (min: %d, max: %d)", minLength, maxLength);
    return true;
}

/**
 * Vérifie si le capteur de longueur de ligne est fonctionnel
 * @return true si fonctionnel, false sinon
 */
bool lineLengthIsHealthy() {
    return sensorInitialized;
}

/**
 * Mise à jour périodique du capteur de longueur de ligne
 * Fonction thread-safe compatible avec FreeRTOS
 * @return true si succès, false si échec
 */
bool updateLineLengthSensor() {
    // Vérifier si le capteur est initialisé
    if (!sensorInitialized) {
        static unsigned long lastInitAttempt = 0;
        unsigned long now = millis();
        
        // Tenter une réinitialisation au plus une fois toutes les 5 secondes
        if (now - lastInitAttempt > 5000) {
            lastInitAttempt = now;
            LOG_WARNING("LINE", "Capteur non initialisé, tentative d'initialisation...");
            lineLengthInit();
        }
        
        return false;
    }
    
    // Protection thread-safe
    if (lineLengthMutex == NULL) {
        LOG_ERROR("LINE", "Mutex non disponible");
        return false;
    }
    
    bool result = false;
    
    if (xSemaphoreTake(lineLengthMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Simulation de mise à jour des données
        // Générer une longueur entre 100 et 500 cm qui varie lentement
        currentLength = 300 + 200 * sin(millis() / 10000.0);
        
        result = true;
        xSemaphoreGive(lineLengthMutex);
    } else {
        LOG_ERROR("LINE", "Impossible d'acquérir le mutex pour la mise à jour");
    }
    
    return result;
}