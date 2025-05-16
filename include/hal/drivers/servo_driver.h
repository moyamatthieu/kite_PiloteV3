// filepath: /workspaces/kite_PiloteV3/include/hal/drivers/servo_driver.h
#pragma once

#include "hal/hal_component.h" // Modifié pour inclure le bon fichier d'en-tête
#include "common/global_enums.h"
#include "core/config.h" // Pour les configurations globales comme MAX_SERVOS_PER_DRIVER
#include <ESP32Servo.h> 

// Identification des servos
enum class ServoLogicalType : uint8_t {
    DIRECTION = 0,
    TRIM = 1,
    LINE_MODULATION = 2,
    SERVO_1 = 0, // Alias génériques
    SERVO_2 = 1,
    SERVO_3 = 2,
    UNKNOWN = 0xFF
};

// Structure pour la configuration d'un servomoteur individuel
struct ServoInstanceConfig {
    uint8_t pin;                // Broche du servo. 0xFF si non utilisé.
    int minPulseWidthUs;        // Largeur d'impulsion minimale en microsecondes (us)
    int maxPulseWidthUs;        // Largeur d'impulsion maximale en microsecondes (us)
    float minAngleDeg;          // Angle/valeur logique minimum (par ex. -90 degrés, 0%)
    float maxAngleDeg;          // Angle/valeur logique maximum (par ex. 90 degrés, 100%)
    float defaultAngleDeg;      // Angle/valeur par défaut à l'initialisation
    bool invertDirection;       // Inverser la direction logique de l'angle
    float currentAngle;         // Angle courant du servo (stocké pour référence future)

    // Champs d'état interne
    bool attached;              
    Servo espServoInstance;     

    ServoInstanceConfig() : 
        pin(0xFF), minPulseWidthUs(SERVO_DEFAULT_MIN_PULSE_WIDTH_US), maxPulseWidthUs(SERVO_DEFAULT_MAX_PULSE_WIDTH_US), 
        minAngleDeg(SERVO_DEFAULT_MIN_ANGLE_DEG), maxAngleDeg(SERVO_DEFAULT_MAX_ANGLE_DEG), defaultAngleDeg(SERVO_DEFAULT_NEUTRAL_ANGLE_DEG), 
        invertDirection(false), currentAngle(SERVO_DEFAULT_NEUTRAL_ANGLE_DEG), attached(false) {}
};

// Structure pour la configuration globale du pilote de servos
struct ServoDriverConfig {
    ServoInstanceConfig servoConfigs[MAX_SERVOS_PER_DRIVER];
    // Autres configurations globales pour le pilote si nécessaire
};

class ServoDriver : public ActuatorComponent {
public:
    ServoDriver(const char* name = "ServoDriver");
    ~ServoDriver() override;

    ErrorCode initialize() override;
    ErrorCode shutdown();
    ErrorCode selfTest();

    // Implémentation de la méthode virtuelle pure actuate()
    void actuate() override {}

    ErrorCode configure(const ServoDriverConfig& config);

    // Méthodes de contrôle
    ErrorCode setAngle(ServoLogicalType type, float angleDeg);
    ErrorCode setPulseWidth(ServoLogicalType type, int pulseWidthUs);
    float getAngle(ServoLogicalType type) const;
    int getPulseWidth(ServoLogicalType type) const;
    bool isAttached(ServoLogicalType type) const;
    ErrorCode attachServo(ServoLogicalType type);
    ErrorCode detachServo(ServoLogicalType type);

protected:
    void onEnable() override;
    void onDisable() override;

private:
    ServoDriverConfig _config;
    bool _isInitialized = false;

    // Méthodes internes
    ErrorCode applyCurrentConfig();
    int mapAngleToPulseWidth(float angleDeg, const ServoInstanceConfig& cfg) const;
    float mapPulseWidthToAngle(int pulseWidthUs, const ServoInstanceConfig& cfg) const;
    ServoInstanceConfig* getServoInstanceConfig(ServoLogicalType type);
    const ServoInstanceConfig* getServoInstanceConfig(ServoLogicalType type) const;
};