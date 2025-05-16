#include "hal/drivers/servo_driver.h"
#include "core/logging.h" // Pour le logging
#include "core/config.h"    // Pour les constantes de configuration (broches, largeurs d'impulsion)

// Inclure la bibliothèque Servo appropriée pour la plateforme
#if defined(ESP32)
#include <ESP32Servo.h>
#else
// Implémentation de remplacement pour les plateformes non-ESP32 ou si ESP32Servo n'est pas utilisé
// Cette classe Servo de remplacement est très basique.
// Vous devrez la remplacer par une implémentation de bibliothèque servo réelle pour votre matériel.
class Servo {
public:
    void attach(int pin, int minPulse = DEFAULT_SERVO_MIN_PULSE, int maxPulse = DEFAULT_SERVO_MAX_PULSE) {
        this->pin = pin;
        this->minPulse = minPulse;
        this->maxPulse = maxPulse;
        this->is_attached = true;
    }
    void write(int value) { // 'value' peut être un angle ou une largeur d'impulsion selon l'utilisation
        if (!is_attached) return;
        current_angle = value;
    }
    void writeMicroseconds(int pulseWidth) {
        if (!is_attached) return;
        if (maxPulse == minPulse) current_angle = 0;
        else current_angle = (pulseWidth - minPulse) * 180.0f / (maxPulse - minPulse);
    }
    void detach() {
        is_attached = false;
    }
    bool attached() {
        return is_attached;
    }
    int read() { // Lit l'angle actuel
        return current_angle;
    }
private:
    int pin = -1;
    bool is_attached = false;
    int minPulse = DEFAULT_SERVO_MIN_PULSE; // Utiliser les valeurs de config.h
    int maxPulse = DEFAULT_SERVO_MAX_PULSE; // Utiliser les valeurs de config.h
    int current_angle = 90; // Angle par défaut
};
#endif

ServoDriver::ServoDriver(const char* name)
    : ActuatorComponent(name), _isInitialized(false) {
    // Initialisation des angles courants
    for (int i = 0; i < MAX_SERVOS_PER_DRIVER; ++i) {
        _config.servoConfigs[i].currentAngle = _config.servoConfigs[i].defaultAngleDeg;
        _config.servoConfigs[i].attached = false;
    }
}

ServoDriver::~ServoDriver() {
    shutdown();
}

ErrorCode ServoDriver::initialize() {
    setState(ComponentState::INITIALIZING);
    LOG_INFO(getName(), "Initialisation du ServoDriver...");
    for (int i = 0; i < MAX_SERVOS_PER_DRIVER; ++i) {
        auto& config = _config.servoConfigs[i];
        if (config.pin == 0xFF) continue;
        if (config.espServoInstance.attached()) {
            config.espServoInstance.detach();
        }
        int minPulse = (config.minPulseWidthUs > 0) ? config.minPulseWidthUs : SERVO_DEFAULT_MIN_PULSE_WIDTH_US;
        int maxPulse = (config.maxPulseWidthUs > 0) ? config.maxPulseWidthUs : SERVO_DEFAULT_MAX_PULSE_WIDTH_US;
        config.espServoInstance.attach(config.pin, minPulse, maxPulse);
        config.attached = true;
        setAngle(static_cast<ServoLogicalType>(i), config.defaultAngleDeg);
        config.currentAngle = config.defaultAngleDeg;
        LOG_INFO(getName(), "Servo %d attaché à la broche %d (Pulse: %d-%d us)", i, config.pin, minPulse, maxPulse);
    }
    _isInitialized = true;
    setState(ComponentState::ACTIVE);
    LOG_INFO(getName(), "Initialisation terminée.");
    return ErrorCode::OK;
}

ErrorCode ServoDriver::shutdown() {
    setState(ComponentState::COMPONENT_DISABLED);
    LOG_INFO(getName(), "Arrêt du ServoDriver...");
    for (int i = 0; i < MAX_SERVOS_PER_DRIVER; ++i) {
        auto& config = _config.servoConfigs[i];
        if (config.espServoInstance.attached()) {
            config.espServoInstance.detach();
            config.attached = false;
            LOG_INFO(getName(), "Servo %d détaché.", i);
        }
    }
    _isInitialized = false;
    LOG_INFO(getName(), "Arrêt terminé.");
    return ErrorCode::OK;
}

ErrorCode ServoDriver::selfTest() {
    LOG_INFO(getName(), "Self-test...");
    if (getState() != ComponentState::ACTIVE) {
        LOG_ERROR(getName(), "Impossible de faire le self-test: driver non actif.");
        return ErrorCode::RESOURCE_ERROR;
    }
    for (int i = 0; i < MAX_SERVOS_PER_DRIVER; ++i) {
        auto& config = _config.servoConfigs[i];
        if (config.pin == 0xFF) continue;
        setAngle(static_cast<ServoLogicalType>(i), config.minAngleDeg);
        setAngle(static_cast<ServoLogicalType>(i), config.maxAngleDeg);
        setAngle(static_cast<ServoLogicalType>(i), config.defaultAngleDeg);
        LOG_DEBUG(getName(), "Servo %d test sequence envoyée.", i);
    }
    LOG_INFO(getName(), "Self-test OK.");
    return ErrorCode::OK;
}

ErrorCode ServoDriver::configure(const ServoDriverConfig& config) {
    _config = config;
    LOG_INFO(getName(), "Configuration du ServoDriver mise à jour.");
    return ErrorCode::OK;
}

ErrorCode ServoDriver::setAngle(ServoLogicalType type, float angleDeg) {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return ErrorCode::INVALID_PARAMETER;
    auto& config = _config.servoConfigs[idx];
    if (!config.attached) return ErrorCode::RESOURCE_ERROR;
    float angle = angleDeg;
    if (angle < config.minAngleDeg) angle = config.minAngleDeg;
    if (angle > config.maxAngleDeg) angle = config.maxAngleDeg;
    int pulse = mapAngleToPulseWidth(angle, config);
    config.espServoInstance.writeMicroseconds(pulse);
    config.currentAngle = angle;
    return ErrorCode::OK;
}

ErrorCode ServoDriver::setPulseWidth(ServoLogicalType type, int pulseWidthUs) {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return ErrorCode::INVALID_PARAMETER;
    auto& config = _config.servoConfigs[idx];
    if (!config.attached) return ErrorCode::RESOURCE_ERROR;
    int pulse = pulseWidthUs;
    if (pulse < config.minPulseWidthUs) pulse = config.minPulseWidthUs;
    if (pulse > config.maxPulseWidthUs) pulse = config.maxPulseWidthUs;
    config.espServoInstance.writeMicroseconds(pulse);
    config.currentAngle = mapPulseWidthToAngle(pulse, config);
    return ErrorCode::OK;
}

float ServoDriver::getAngle(ServoLogicalType type) const {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return 0.0f;
    return _config.servoConfigs[idx].currentAngle;
}

int ServoDriver::getPulseWidth(ServoLogicalType type) const {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return 0;
    auto& config = _config.servoConfigs[idx];
    float angle = config.currentAngle;
    return mapAngleToPulseWidth(angle, config);
}

bool ServoDriver::isAttached(ServoLogicalType type) const {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return false;
    // Cast non-const pour appeler attached() (non const dans ESP32Servo)
    return const_cast<Servo&>(_config.servoConfigs[idx].espServoInstance).attached();
}

ErrorCode ServoDriver::attachServo(ServoLogicalType type) {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return ErrorCode::INVALID_PARAMETER;
    auto& config = _config.servoConfigs[idx];
    if (!config.attached) {
        int minPulse = (config.minPulseWidthUs > 0) ? config.minPulseWidthUs : SERVO_DEFAULT_MIN_PULSE_WIDTH_US;
        int maxPulse = (config.maxPulseWidthUs > 0) ? config.maxPulseWidthUs : SERVO_DEFAULT_MAX_PULSE_WIDTH_US;
        config.espServoInstance.attach(config.pin, minPulse, maxPulse);
        config.attached = true;
        LOG_INFO(getName(), "Servo %d attaché à la broche %d.", idx, config.pin);
    }
    return ErrorCode::OK;
}

ErrorCode ServoDriver::detachServo(ServoLogicalType type) {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return ErrorCode::INVALID_PARAMETER;
    auto& config = _config.servoConfigs[idx];
    if (config.espServoInstance.attached()) {
        config.espServoInstance.detach();
        config.attached = false;
        LOG_INFO(getName(), "Servo %d détaché.", idx);
    }
    return ErrorCode::OK;
}

void ServoDriver::onEnable() {
    setState(ComponentState::ACTIVE);
    LOG_INFO(getName(), "ServoDriver activé.");
}

void ServoDriver::onDisable() {
    setState(ComponentState::COMPONENT_DISABLED);
    LOG_INFO(getName(), "ServoDriver désactivé.");
}

ErrorCode ServoDriver::applyCurrentConfig() {
    // Peut être utilisé pour reconfigurer dynamiquement
    return ErrorCode::OK;
}

int ServoDriver::mapAngleToPulseWidth(float angleDeg, const ServoInstanceConfig& cfg) const {
    float clamped = angleDeg;
    if (clamped < cfg.minAngleDeg) clamped = cfg.minAngleDeg;
    if (clamped > cfg.maxAngleDeg) clamped = cfg.maxAngleDeg;
    float norm = (clamped - cfg.minAngleDeg) / (cfg.maxAngleDeg - cfg.minAngleDeg);
    return static_cast<int>(cfg.minPulseWidthUs + norm * (cfg.maxPulseWidthUs - cfg.minPulseWidthUs));
}

float ServoDriver::mapPulseWidthToAngle(int pulseWidthUs, const ServoInstanceConfig& cfg) const {
    int clamped = pulseWidthUs;
    if (clamped < cfg.minPulseWidthUs) clamped = cfg.minPulseWidthUs;
    if (clamped > cfg.maxPulseWidthUs) clamped = cfg.maxPulseWidthUs;
    float norm = static_cast<float>(clamped - cfg.minPulseWidthUs) / (cfg.maxPulseWidthUs - cfg.minPulseWidthUs);
    return cfg.minAngleDeg + norm * (cfg.maxAngleDeg - cfg.minAngleDeg);
}

ServoInstanceConfig* ServoDriver::getServoInstanceConfig(ServoLogicalType type) {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return nullptr;
    return &_config.servoConfigs[idx];
}

const ServoInstanceConfig* ServoDriver::getServoInstanceConfig(ServoLogicalType type) const {
    int idx = static_cast<int>(type);
    if (idx < 0 || idx >= MAX_SERVOS_PER_DRIVER) return nullptr;
    return &_config.servoConfigs[idx];
}
