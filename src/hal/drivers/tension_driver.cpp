#include "hal/drivers/tension_driver.h"
#include "core/logging.h"
#include <Arduino.h> // Pour millis(), analogRead()

TensionDriver::TensionDriver(const char* name)
    : SensorComponent(name, true) { // Activé par défaut
    memset(&_processedData, 0, sizeof(ProcessedTensionData));
    _processedData.isValid = false;
    // La configuration par défaut est gérée par le constructeur de TensionDriverConfig
}

ErrorCode TensionDriver::initialize() {
    setState(ComponentState::INITIALIZING);
    LOG_INFO("TensionDriver", "Initializing %s...", getName());

    if (_config.sensorPin == 0xFF) {
        LOG_ERROR("TensionDriver", "Sensor pin not configured for %s.", getName());
        setState(ComponentState::ERROR);
        return ErrorCode::INVALID_PARAMETER;
    }

    pinMode(_config.sensorPin, INPUT);
    // Autres initialisations spécifiques au matériel (ex: HX711)

    LOG_INFO("TensionDriver", "Tension sensor %s initialized on pin %d.", getName(), _config.sensorPin);
    setState(ComponentState::IDLE);
    return ErrorCode::OK;
}

ErrorCode TensionDriver::applyConfig() {
    LOG_INFO("TensionDriver", "Applying configuration for %s...", getName());
    // Vérifier la validité de la configuration (ex: broche)
    if (_config.sensorPin == 0xFF) {
        LOG_ERROR("TensionDriver", "Cannot apply config: Sensor pin not set for %s.", getName());
        return ErrorCode::INVALID_PARAMETER;
    }
    // Si le matériel supporte des configurations dynamiques (ex: gain d'un HX711), les appliquer ici.
    LOG_INFO("TensionDriver", "Configuration applied for %s.", getName());
    return ErrorCode::OK;
}

ErrorCode TensionDriver::configure(const TensionDriverConfig& config) {
    LOG_INFO("TensionDriver", "New configuration received for %s.", getName());
    _config = config;
    if (getState() == ComponentState::ACTIVE || getState() == ComponentState::IDLE || getState() == ComponentState::INITIALIZING) {
        ErrorCode status = applyConfig();
        if (status != ErrorCode::OK) {
            setState(ComponentState::ERROR);
            return status;
        }
        // Pourrait nécessiter une réinitialisation si la broche change, par exemple.
        if (getState() == ComponentState::IDLE || getState() == ComponentState::ACTIVE) {
            LOG_INFO("TensionDriver", "Re-applying config to already initialized driver %s.", getName());
            // Si la broche a changé, il faut la reconfigurer
            pinMode(_config.sensorPin, INPUT);
        }
    } else {
        LOG_WARNING("TensionDriver", "Component %s not in a state to apply new config immediately (current state: %s). Config stored.", getName(), stateString());
    }
    return ErrorCode::OK;
}

float TensionDriver::readRawValue() {
    // Pour un simple capteur analogique
    // Pour un HX711, la lecture serait différente (librairie spécifique)
    if (_config.sensorPin == 0xFF) return 0.0f;
    return static_cast<float>(analogRead(_config.sensorPin));
}

void TensionDriver::processSensorData(float rawValue) {
    _processedData.rawValue = rawValue;
    // Formule de conversion basique : (valeur_brute - offset_zero) * facteur_calibration
    // Ceci est une simplification. Une formule plus précise dépendrait de la cellule de charge
    // et de l'électronique d'amplification (ex: pont de Wheatstone, HX711).
    // Tension (N) = ( (ValeurADC / ResolutionADC) * Vref - ZeroOffset_mV ) * CalibrationFactor_N_per_mV
    // Ou plus directement si calibrationFactor est en N/ADC_unit:
    _processedData.tension = (rawValue - _config.zeroOffset) * _config.calibrationFactor;
    _processedData.timestamp = millis();
    _processedData.isValid = true;
}

void TensionDriver::readSensor() {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        LOG_WARNING("TensionDriver", "%s not active or idle, cannot read sensor.", getName());
        _processedData.isValid = false;
        return;
    }
    if(getState() == ComponentState::IDLE) {
        setState(ComponentState::ACTIVE);
    }

    float raw = readRawValue();
    processSensorData(raw);

    // LOG_DEBUG("TensionDriver", "Tension Data: Raw=%.0f, Tension=%.2f N", 
    //           _processedData.rawValue, _processedData.tension);
}

ErrorCode TensionDriver::calibrateZero() {
    LOG_INFO("TensionDriver", "Calibrating zero offset (tare) for %s... Ensure no load.", getName());
    setState(ComponentState::INITIALIZING);
    
    const int numReadings = 50;
    float sumRawValues = 0;
    for (int i = 0; i < numReadings; ++i) {
        sumRawValues += readRawValue();
        delay(10); // Petit délai entre les lectures
    }
    _config.zeroOffset = sumRawValues / numReadings;

    LOG_INFO("TensionDriver", "Zero offset calibrated to: %.2f (raw ADC units) for %s.", _config.zeroOffset, getName());
    setState(ComponentState::IDLE);
    // TODO: Sauvegarder _config.zeroOffset en mémoire non volatile
    return ErrorCode::OK;
}

ErrorCode TensionDriver::calibrateScale(float knownWeight) {
    if (knownWeight <= 0) {
        LOG_ERROR("TensionDriver", "Known weight for scale calibration must be positive for %s.", getName());
        return ErrorCode::INVALID_PARAMETER;
    }
    LOG_INFO("TensionDriver", "Calibrating scale for %s with known weight %.2f N...", getName(), knownWeight);
    setState(ComponentState::INITIALIZING);

    const int numReadings = 50;
    float sumRawValues = 0;
    for (int i = 0; i < numReadings; ++i) {
        sumRawValues += readRawValue();
        delay(10);
    }
    float rawAtKnownWeight = sumRawValues / numReadings;
    float valueAboveZero = rawAtKnownWeight - _config.zeroOffset;

    if (valueAboveZero <= 0) {
        LOG_ERROR("TensionDriver", "Scale calibration error: raw value at known weight (%.2f) is not above zero offset (%.2f) for %s. Check load cell or connections.", rawAtKnownWeight, _config.zeroOffset, getName());
        setState(ComponentState::ERROR);
        return ErrorCode::SENSOR_FAILURE;
    }

    _config.calibrationFactor = knownWeight / valueAboveZero;

    LOG_INFO("TensionDriver", "Scale calibrated. Factor: %.4f N/ADC_unit for %s.", _config.calibrationFactor, getName());
    setState(ComponentState::IDLE);
    // TODO: Sauvegarder _config.calibrationFactor en mémoire non volatile
    return ErrorCode::OK;
}

bool TensionDriver::selfTest() {
    LOG_INFO("TensionDriver", "Performing self-test for %s...", getName());
    // Un auto-test pourrait impliquer de vérifier la connectivité ou des valeurs de repos attendues.
    // Pour une cellule de charge simple, cela pourrait être limité.
    float raw = readRawValue();
    if (raw < 0 || raw > _config.adcResolution * 1.1) { // Vérifier si la valeur est grossièrement hors limites
        LOG_ERROR("TensionDriver", "Self-test FAILED for %s: Raw value %.2f out of expected range.", getName(), raw);
        return false;
    }
    LOG_INFO("TensionDriver", "Self-test for %s PASSED (basic raw value check).", getName());
    return true;
}

void TensionDriver::onEnable() {
    LOG_INFO("TensionDriver", "%s enabled. Initializing if not already.", getName());
    if (getState() == ComponentState::UNINITIALIZED || getState() == ComponentState::COMPONENT_DISABLED || getState() == ComponentState::ERROR) {
        initialize(); 
    } else {
        setState(ComponentState::IDLE);
    }
}

void TensionDriver::onDisable() {
    LOG_INFO("TensionDriver", "%s disabled.", getName());
    _processedData.isValid = false;
    setState(ComponentState::COMPONENT_DISABLED);
}

// Implémentations des nouvelles méthodes d'accès
bool TensionDriver::dataAvailable() const {
    return _processedData.isValid && (getState() == ComponentState::ACTIVE || getState() == ComponentState::IDLE);
}

float TensionDriver::getTension() const {
    return _processedData.isValid ? _processedData.tension : 0.0f;
}
