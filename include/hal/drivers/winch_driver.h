#pragma once

#include "hal/hal_component.h"
#include "common/global_enums.h"
#include "core/config.h" // Pour les broches et autres configurations

// Enum pour les commandes du treuil
enum class WinchCommand : uint8_t {
    STOP = 0,
    WIND_IN = 1,    // Enrouler
    WIND_OUT = 2,   // Dérouler
    SET_SPEED = 3,  // Définir la vitesse (pourrait nécessiter une valeur supplémentaire)
    HOLD = 4        // Maintenir la position/tension actuelle (si applicable)
};

// Structure pour la configuration du pilote de treuil
struct WinchDriverConfig {
    // Broches pour le contrôle du moteur du treuil
    // Exemple : pour un pont en H simple
    uint8_t motorPinA;          // Broche de contrôle A du moteur
    uint8_t motorPinB;          // Broche de contrôle B du moteur
    uint8_t speedControlPin;    // Broche PWM pour le contrôle de la vitesse (si applicable)
    bool speedControlPWM;       // True si speedControlPin utilise PWM
    int pwmChannel;             // Canal PWM (pour ESP32)

    // Limites opérationnelles
    float maxSpeed;             // Vitesse maximale (par exemple, en % ou RPM)
    float minSpeed;             // Vitesse minimale
    // Autres paramètres comme les limites de courant, les rampes d'accélération/décélération, etc.

    WinchDriverConfig() : 
        motorPinA(WINCH_DEFAULT_MOTOR_PIN_A), motorPinB(WINCH_DEFAULT_MOTOR_PIN_B),
        speedControlPin(WINCH_DEFAULT_SPEED_PIN), speedControlPWM(true), pwmChannel(WINCH_DEFAULT_PWM_CHANNEL),
        maxSpeed(100.0f), minSpeed(0.0f) {}
};

class WinchDriver : public ActuatorComponent {
public:
    WinchDriver(const char* name = "WinchDriver");
    ~WinchDriver();

    ErrorCode initialize() override;
    ErrorCode configure(const WinchDriverConfig& config);
    ErrorCode reconfigure(const WinchDriverConfig& newConfig); // Pour les changements de configuration à chaud

    // Commandes de base du treuil
    ErrorCode sendCommand(WinchCommand command);
    ErrorCode setSpeed(float speedPercentage); // Vitesse en pourcentage (-100% à 100%)
    ErrorCode stopMotor();

    // Fonctions de retour d'état (si des capteurs sont intégrés ou lus directement)
    float getCurrentSpeed() const;      // Pourrait nécessiter un encodeur ou un capteur de vitesse
    // float getCurrentPosition() const; // Pourrait nécessiter un capteur de position
    // float getCurrentLoad() const;     // Pourrait nécessiter un capteur de courant/tension

    void actuate() override {} // Implémentation vide pour lever l'abstraction

protected:
    void onEnable() override;
    void onDisable() override;
    // void run() override; // Si le composant a une tâche de fond

private:
    ErrorCode applySpeed(float speedPercentage); // Fonction interne pour appliquer la vitesse au matériel

    WinchDriverConfig _driverConfig;
    bool _isInitialized;
    float _currentSpeedSetting; // Vitesse actuellement définie
    // État du moteur (par exemple, WINDING_IN, WINDING_OUT, STOPPED)
    WinchCommand _currentMotorState;
};
