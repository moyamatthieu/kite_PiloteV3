#include "hal/drivers/wind_driver.h"
#include "core/logging.h" // Assumer que le nouveau système de logging est accessible via ce chemin
#include <Arduino.h>     // Pour millis(), analogRead() si applicable
#include <cmath>         // Pour les calculs mathématiques

WindDriver::WindDriver(const char* name)
    : SensorComponent(name, true), // Activé par défaut
      _maxGustSinceLastRead(0.0f),
      _lastGustResetTime(0) {
    memset(&_processedData, 0, sizeof(ProcessedWindData));
    _processedData.isValid = false;
    // La configuration par défaut est gérée par le constructeur de WindDriverConfig
}

ErrorCode WindDriver::initialize() {
    setState(ComponentState::INITIALIZING);
    LOG_INFO("WindDriver", "Initializing %s...", getName());

    // Vérifier si les broches sont configurées (si applicable)
    // Exemple: if (_config.anemometerPin == 0xFF || _config.windVanePin == 0xFF) {
    //     LOG_WARNING("WindDriver", "Anemometer or Wind Vane pin not configured.");
    //     // Peut-être pas une erreur fatale si la configuration est attendue plus tard
    // }

    // Initialisation spécifique au matériel ici
    // Exemple: pinMode(_config.anemometerPin, INPUT_PULLUP);
    // Exemple: pinMode(_config.windVanePin, INPUT);

    // Pour la simulation, nous n'avons pas de matériel à initialiser
    LOG_INFO("WindDriver", "Wind sensor %s initialized (simulated).", getName());
    _lastGustResetTime = millis();
    setState(ComponentState::IDLE);
    return ErrorCode::OK;
}

ErrorCode WindDriver::applyConfig() {
    LOG_INFO("WindDriver", "Applying configuration for %s...", getName());
    // Appliquer les paramètres de _config au matériel réel
    // Exemple: configurer des registres I2C, des seuils, etc.

    // Pour la simulation, il n'y a rien à appliquer matériellement
    LOG_INFO("WindDriver", "Configuration applied for %s (simulated).", getName());
    return ErrorCode::OK;
}

ErrorCode WindDriver::configure(const WindDriverConfig& config) {
    LOG_INFO("WindDriver", "New configuration received for %s.", getName());
    _config = config;
    if (getState() == ComponentState::ACTIVE || getState() == ComponentState::IDLE || getState() == ComponentState::INITIALIZING) {
        ErrorCode status = applyConfig();
        if (status != ErrorCode::OK) {
            setState(ComponentState::ERROR);
            return status;
        }
        // Si l'initialisation dépend de la config, et que nous sommes déjà initialisés, 
        // il pourrait être nécessaire de réinitialiser certaines parties.
        if (getState() == ComponentState::IDLE || getState() == ComponentState::ACTIVE) {
             LOG_INFO("WindDriver", "Re-applying config to already initialized driver.");
        }
    } else {
        LOG_WARNING("WindDriver", "Component %s not in a state to apply new config immediately (current state: %s). Config stored.", getName(), stateString());
    }
    return ErrorCode::OK;
}

void WindDriver::readSensor() {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        LOG_WARNING("WindDriver", "%s not active or idle, cannot read sensor.", getName());
        _processedData.isValid = false;
        return;
    }
    // Mettre à jour l'état à ACTIVE si ce n'est pas déjà le cas et que nous lisons
    if(getState() == ComponentState::IDLE) {
        setState(ComponentState::ACTIVE);
    }

    // Simulation de lecture de données
    _processedData.speed = 5.0f + (rand() % 20) / 10.0f; // Vitesse entre 5.0 et 6.9 m/s
    _processedData.direction = fmod((180.0f + (rand() % 40) - 20.0f), 360.0f); // Direction autour de 180 deg
    if (_processedData.direction < 0) _processedData.direction += 360.0f;

    // Simulation de rafale simple
    if (_processedData.speed > _maxGustSinceLastRead) {
        _maxGustSinceLastRead = _processedData.speed;
    }

    // Rafraîchir la valeur de rafale toutes les N secondes (par exemple, 10s)
    if (millis() - _lastGustResetTime > 10000) {
        _processedData.gust = _maxGustSinceLastRead;
        _maxGustSinceLastRead = _processedData.speed; // Réinitialiser pour la prochaine période
        _lastGustResetTime = millis();
    } else {
        // Conserver la valeur de rafale précédente jusqu'au prochain reset
        // ou la mettre à jour si la rafale actuelle est plus élevée que la rafale enregistrée
        if (_maxGustSinceLastRead > _processedData.gust) {
             _processedData.gust = _maxGustSinceLastRead;
        }
    }
    
    _processedData.timestamp = millis();
    _processedData.isValid = true;

    // LOG_DEBUG("WindDriver", "Simulated Wind Data: Speed=%.2fm/s, Dir=%.1fdeg, Gust=%.2fm/s", 
    //           _processedData.speed, _processedData.direction, _processedData.gust);
}

void WindDriver::readSensor();

bool WindDriver::read() {
    readSensor();
    return _processedData.isValid;
}

// Méthodes spécifiques (exemples, à implémenter si nécessaire)
ErrorCode WindDriver::calibrateSpeed(float knownWindSpeed) {
    LOG_INFO("WindDriver", "Calibrating speed for %s with known speed %.2f m/s...", getName(), knownWindSpeed);
    setState(ComponentState::INITIALIZING);
    // ... simulation ...
    LOG_INFO("WindDriver", "Speed calibration finished for %s.", getName());
    setState(ComponentState::IDLE);
    return ErrorCode::INVALID_PARAMETER;
}

ErrorCode WindDriver::calibrateDirection(float knownWindDirection) {
    LOG_INFO("WindDriver", "Calibrating direction for %s with known direction %.1f deg...", getName(), knownWindDirection);
    setState(ComponentState::INITIALIZING);
    // ... simulation ...
    LOG_INFO("WindDriver", "Direction calibration finished for %s.", getName());
    setState(ComponentState::IDLE);
    return ErrorCode::INVALID_PARAMETER;
}

bool WindDriver::selfTest() {
    LOG_INFO("WindDriver", "Performing self-test for %s...", getName());
    // Logique d'auto-test matériel ici (si applicable)
    // Pour la simulation, on retourne toujours true
    LOG_INFO("WindDriver", "Self-test for %s PASSED (simulated).", getName());
    return true;
}

void WindDriver::onEnable() {
    LOG_INFO("WindDriver", "%s enabled. Initializing if not already.", getName());
    if (getState() == ComponentState::UNINITIALIZED || getState() == ComponentState::COMPONENT_DISABLED || getState() == ComponentState::ERROR) {
        initialize(); 
    } else {
        setState(ComponentState::IDLE); // Prêt à lire
    }
}

void WindDriver::onDisable() {
    LOG_INFO("WindDriver", "%s disabled.", getName());
    // Actions spécifiques à la désactivation si nécessaire (ex: mettre le capteur en veille)
    _processedData.isValid = false;
    setState(ComponentState::COMPONENT_DISABLED);
}

// Implémentations des nouvelles méthodes d'accès
bool WindDriver::dataAvailable() const {
    return _processedData.isValid && (getState() == ComponentState::ACTIVE || getState() == ComponentState::IDLE);
}

float WindDriver::getWindSpeed() const {
    return _processedData.isValid ? _processedData.speed : 0.0f;
}

float WindDriver::getWindDirection() const {
    return _processedData.isValid ? _processedData.direction : 0.0f;
}

// Pas de REGISTER_COMPONENT ici, sera géré par un service ou main.cpp
