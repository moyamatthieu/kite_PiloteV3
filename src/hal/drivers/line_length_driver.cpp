#include "hal/drivers/line_length_driver.h"
#include "core/logging.h"
#include <Arduino.h> // Pour attachInterrupt, digitalRead, etc.

// Initialisation des membres statiques
volatile long LineLengthDriver::_isrPulseCount = 0;
volatile int8_t LineLengthDriver::_isrLastEncoderAPinState = LOW;
LineLengthDriver* LineLengthDriver::_instance = nullptr;

// ISR pour l'encodeur (version simple, front montant sur PinA)
// Pour un encodeur en quadrature, cette ISR serait plus complexe
void IRAM_ATTR LineLengthDriver::encoderISR() {
    if (_instance && _instance->_config.encoderPinA != 0xFF) {
        // Détection de front montant simple pour compter les impulsions
        // Une implémentation plus robuste utiliserait PinA et PinB pour la direction
        // et une machine à états pour gérer les transitions de l'encodeur en quadrature.
        int currentStateA = digitalRead(_instance->_config.encoderPinA);
        if (currentStateA == HIGH && _isrLastEncoderAPinState == LOW) {
            if (_instance->_config.reverseDirection) {
                _isrPulseCount--;
            } else {
                _isrPulseCount++;
            }
        }
        _isrLastEncoderAPinState = currentStateA;
    }
}

LineLengthDriver::LineLengthDriver(const char* name)
    : SensorComponent(name, true), // Activé par défaut
      _lastPulseCountForSpeed(0),
      _lastSpeedCalcTime(0) {
    _instance = this; // Enregistrer cette instance pour l'ISR
    memset(&_processedData, 0, sizeof(ProcessedLineLengthData));
    _processedData.isValid = false;
}

LineLengthDriver::~LineLengthDriver() {
    if (_config.encoderPinA != 0xFF) {
        detachInterrupt(digitalPinToInterrupt(_config.encoderPinA));
    }
    if (_instance == this) {
        _instance = nullptr;
    }
}

ErrorCode LineLengthDriver::initialize() {
    setState(ComponentState::INITIALIZING);
    LOG_INFO("LineLengthDriver", "Initializing %s...", getName());

    if (_config.encoderPinA == 0xFF) {
        LOG_ERROR("LineLengthDriver", "Encoder Pin A not configured for %s.", getName());
        setState(ComponentState::ERROR);
        return ErrorCode::INVALID_PARAMETER;
    }
    if (_config.pulsesPerRevolution <= 0 || _config.drumCircumference <= 0) {
        LOG_ERROR("LineLengthDriver", "Invalid pulsesPerRevolution or drumCircumference for %s.", getName());
        setState(ComponentState::ERROR);
        return ErrorCode::INVALID_PARAMETER;
    }

    pinMode(_config.encoderPinA, INPUT_PULLUP); // Ou INPUT selon le câblage
    if (_config.encoderPinB != 0xFF) {
        pinMode(_config.encoderPinB, INPUT_PULLUP); // Si PinB est utilisé
    }

    // Attacher l'interruption
    // L'ISR est appelée sur chaque changement d'état (CHANGE) ou front (RISING/FALLING)
    // Pour un encodeur simple, RISING sur PinA peut suffire.
    // Pour quadrature, on surveille les deux broches.
    _isrPulseCount = 0; // Réinitialiser le compteur au cas où
    _isrLastEncoderAPinState = digitalRead(_config.encoderPinA); // Initialiser l'état pour la détection de front
    attachInterrupt(digitalPinToInterrupt(_config.encoderPinA), encoderISR, CHANGE); // CHANGE est plus robuste pour certains encodeurs

    _lastSpeedCalcTime = millis();
    _lastPulseCountForSpeed = _isrPulseCount; // Utiliser la valeur actuelle après reset
    _processedData.pulseCount = _isrPulseCount;

    LOG_INFO("LineLengthDriver", "Line Length sensor %s initialized on PinA %d. Pulses/Rev: %.1f, Circum: %.2fm.", 
        getName(), _config.encoderPinA, _config.pulsesPerRevolution, _config.drumCircumference);
    setState(ComponentState::IDLE);
    return ErrorCode::OK;
}

ErrorCode LineLengthDriver::applyConfig() {
    LOG_INFO("LineLengthDriver", "Applying configuration for %s...", getName());
    if (_config.encoderPinA == 0xFF || _config.pulsesPerRevolution <= 0 || _config.drumCircumference <= 0) {
        LOG_ERROR("LineLengthDriver", "Cannot apply config: Invalid parameters for %s.", getName());
        return ErrorCode::INVALID_PARAMETER;
    }
    // Si l'état est déjà initialisé ou actif, il faut potentiellement détacher/rattacher l'interruption
    // si les broches ou la configuration de l'ISR changent.
    // Pour l'instant, on suppose que l'initialisation gère cela.
    LOG_INFO("LineLengthDriver", "Configuration applied for %s.", getName());
    return ErrorCode::OK;
}

ErrorCode LineLengthDriver::configure(const LineLengthDriverConfig& config) {
    LOG_INFO("LineLengthDriver", "New configuration received for %s.", getName());
    
    // Détacher l'ancienne interruption si la broche change et qu'elle était valide
    if (_config.encoderPinA != 0xFF && _config.encoderPinA != config.encoderPinA && (getState() == ComponentState::IDLE || getState() == ComponentState::ACTIVE)) {
        detachInterrupt(digitalPinToInterrupt(_config.encoderPinA));
        LOG_INFO("LineLengthDriver", "Detached interrupt from old pin %d for %s.", _config.encoderPinA, getName());
    }

    _config = config;

    if (getState() == ComponentState::UNINITIALIZED || getState() == ComponentState::COMPONENT_DISABLED || getState() == ComponentState::ERROR) {
        // La configuration sera appliquée lors du prochain initialize()
        LOG_WARNING("LineLengthDriver", "Component %s not initialized. Config stored, will apply on initialize().", getName());
    } else if (getState() == ComponentState::IDLE || getState() == ComponentState::ACTIVE) {
        ErrorCode status = applyConfig();
        if (status != ErrorCode::OK) {
            setState(ComponentState::ERROR);
            return status;
        }
        // Réinitialiser et rattacher l'interruption avec la nouvelle configuration
        LOG_INFO("LineLengthDriver", "Re-initializing interrupt for %s due to new config.", getName());
        pinMode(_config.encoderPinA, INPUT_PULLUP);
        if (_config.encoderPinB != 0xFF) pinMode(_config.encoderPinB, INPUT_PULLUP);
        
        _isrPulseCount = _processedData.pulseCount; // Conserver le compte actuel si possible, ou réinitialiser
        _isrLastEncoderAPinState = digitalRead(_config.encoderPinA);
        attachInterrupt(digitalPinToInterrupt(_config.encoderPinA), encoderISR, CHANGE);
        LOG_INFO("LineLengthDriver", "Interrupt re-attached to pin %d for %s.", _config.encoderPinA, getName());
    } else {
         LOG_WARNING("LineLengthDriver", "Component %s in state %s. Config stored.", getName(), stateString());
    }
    return ErrorCode::OK;
}

void LineLengthDriver::calculateMetrics() {
    long currentPulseCountSnapshot;
    uint32_t currentTime = millis();

    // Accéder au compteur volatile de manière atomique (désactiver les interruptions brièvement)
    noInterrupts();
    currentPulseCountSnapshot = _isrPulseCount;
    interrupts();

    _processedData.pulseCount = currentPulseCountSnapshot;
    
    // Calcul de la longueur
    // Longueur = (Nombre d'impulsions / Impulsions par tour) * Circonférence du tambour
    _processedData.length = (static_cast<float>(currentPulseCountSnapshot) / _config.pulsesPerRevolution) * _config.drumCircumference;

    // Calcul de la vitesse
    uint32_t deltaTime = currentTime - _lastSpeedCalcTime;
    if (deltaTime > 0) { // Éviter la division par zéro et calculer si assez de temps s'est écoulé
        long deltaPulses = currentPulseCountSnapshot - _lastPulseCountForSpeed;
        float distanceMoved = (static_cast<float>(deltaPulses) / _config.pulsesPerRevolution) * _config.drumCircumference;
        _processedData.speed = (distanceMoved * 1000.0f) / static_cast<float>(deltaTime); // m/s
        
        _lastPulseCountForSpeed = currentPulseCountSnapshot;
        _lastSpeedCalcTime = currentTime;
    } else if (currentTime < _lastSpeedCalcTime) { // Rollover de millis()
        _lastSpeedCalcTime = currentTime; // Réinitialiser pour le prochain calcul
        _lastPulseCountForSpeed = currentPulseCountSnapshot;
        _processedData.speed = 0.0f; // Vitesse indéterminée pour ce cycle
    }
    // Si deltaTime == 0, on garde la vitesse précédente (ou 0 si premier calcul)

    _processedData.timestamp = currentTime;
    _processedData.isValid = true;
}

void LineLengthDriver::readSensor() {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        LOG_WARNING("LineLengthDriver", "%s not active or idle, cannot read sensor.", getName());
        _processedData.isValid = false;
        return;
    }
    if(getState() == ComponentState::IDLE) {
        setState(ComponentState::ACTIVE);
    }

    calculateMetrics();

    // LOG_DEBUG("LineLengthDriver", "Line Length: %.2fm, Speed: %.2fm/s, Pulses: %ld", 
    //           _processedData.length, _processedData.speed, _processedData.pulseCount);
}

ErrorCode LineLengthDriver::resetLength(float currentLength) {
    LOG_INFO("LineLengthDriver", "Resetting line length for %s to %.2f m.", getName(), currentLength);
    setState(ComponentState::INITIALIZING); // Ou un état approprié
    
    noInterrupts();
    // Calculer le nombre d'impulsions correspondant à currentLength
    _isrPulseCount = static_cast<long>((currentLength / _config.drumCircumference) * _config.pulsesPerRevolution);
    _lastPulseCountForSpeed = _isrPulseCount;
    interrupts();
    
    _processedData.length = currentLength;
    _processedData.pulseCount = _isrPulseCount;
    _processedData.speed = 0.0f; // La vitesse est réinitialisée car on change la référence
    _lastSpeedCalcTime = millis();
    _processedData.timestamp = _lastSpeedCalcTime;
    _processedData.isValid = true;

    LOG_INFO("LineLengthDriver", "Line length reset. New pulse count: %ld for %s.", _isrPulseCount, getName());
    setState(ComponentState::IDLE);
    return ErrorCode::OK;
}

bool LineLengthDriver::selfTest() {
    LOG_INFO("LineLengthDriver", "Performing self-test for %s...", getName());
    // Un auto-test pourrait vérifier si les interruptions sont déclenchées
    // ou si les broches sont dans un état attendu.
    // Pour l'instant, simulation simple.
    if (_config.encoderPinA == 0xFF) {
        LOG_ERROR("LineLengthDriver", "Self-test FAILED for %s: Encoder Pin A not configured.", getName());
        return false;
    }
    // On pourrait essayer de lire l'état de la broche, mais sans mouvement, c'est limité.
    LOG_INFO("LineLengthDriver", "Self-test for %s PASSED (basic config check).", getName());
    return true;
}

void LineLengthDriver::onEnable() {
    LOG_INFO("LineLengthDriver", "%s enabled. Initializing if not already.", getName());
    if (getState() == ComponentState::UNINITIALIZED || getState() == ComponentState::COMPONENT_DISABLED || getState() == ComponentState::ERROR) {
        initialize(); 
    } else {
        // Si déjà initialisé, s'assurer que l'interruption est active
        if (_config.encoderPinA != 0xFF) {
            _isrLastEncoderAPinState = digitalRead(_config.encoderPinA);
            attachInterrupt(digitalPinToInterrupt(_config.encoderPinA), encoderISR, CHANGE);
            LOG_INFO("LineLengthDriver", "Interrupt re-enabled for %s on pin %d.", getName(), _config.encoderPinA);
        }
        setState(ComponentState::IDLE);
    }
}

void LineLengthDriver::onDisable() {
    LOG_INFO("LineLengthDriver", "%s disabled. Detaching interrupt.", getName());
    if (_config.encoderPinA != 0xFF) {
        detachInterrupt(digitalPinToInterrupt(_config.encoderPinA));
        LOG_INFO("LineLengthDriver", "Interrupt detached from pin %d for %s.", _config.encoderPinA, getName());
    }
    _processedData.isValid = false;
    setState(ComponentState::COMPONENT_DISABLED);
}

// Implémentations des nouvelles méthodes d'accès
bool LineLengthDriver::dataAvailable() const {
    return _processedData.isValid && (getState() == ComponentState::ACTIVE || getState() == ComponentState::IDLE);
}

float LineLengthDriver::getLineLength() const {
    return _processedData.isValid ? _processedData.length : 0.0f;
}

float LineLengthDriver::getLineSpeed() const {
    return _processedData.isValid ? _processedData.speed : 0.0f;
}
