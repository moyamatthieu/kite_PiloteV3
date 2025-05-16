#include "../../include/control/safety.h"
#include "../../include/core/logging.h"

void SafetyManager::init() {
    LOG_INFO("SAFETY", "Initialisation du gestionnaire de sécurité");
    // Initialisation des paramètres de sécurité
}

bool SafetyManager::checkLimits(float value, float minLimit, float maxLimit) {
    if (value < minLimit || value > maxLimit) {
        LOG_WARNING("SAFETY", "Valeur hors limites: %.2f (min: %.2f, max: %.2f)", value, minLimit, maxLimit);
        return false;
    }
    return true;
}

void SafetyManager::handleSafety(float windSpeed, float lineTension, float kiteAltitude) {
    if (!checkLimits(windSpeed, MIN_WIND_SPEED, MAX_WIND_SPEED)) {
        LOG_ERROR("SAFETY", "Vitesse du vent hors limites: %.2f m/s", windSpeed);
        // Actions correctives pour la vitesse du vent
    }
    if (!checkLimits(lineTension, MIN_LINE_TENSION, MAX_LINE_TENSION)) {
        LOG_ERROR("SAFETY", "Tension des lignes hors limites: %.2f N", lineTension);
        // Actions correctives pour la tension des lignes
    }
    if (!checkLimits(kiteAltitude, MIN_KITE_ALTITUDE, MAX_KITE_ALTITUDE)) {
        LOG_ERROR("SAFETY", "Altitude du kite hors limites: %.2f m", kiteAltitude);
        // Actions correctives pour l'altitude du kite
    }
}