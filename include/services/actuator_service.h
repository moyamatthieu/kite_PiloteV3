#ifndef ACTUATOR_SERVICE_H
#define ACTUATOR_SERVICE_H

#include "core/component.h"
#include "common/global_enums.h"
#include "core/config.h"

// Inclure les en-têtes des drivers HAL pour les actionneurs
// Utilisation de chemins relatifs à la racine du dossier 'include'
#include "hal/drivers/servo_driver.h"
#include "hal/drivers/winch_driver.h"

#include <vector>
#include <memory>
#include <freertos/FreeRTOS.h> // Pour SemaphoreHandle_t
#include <freertos/semphr.h>   // Pour xSemaphoreCreateMutex, etc.

class ActuatorService : public ManagedComponent {
public:
    // Singleton instance accessor
    static ActuatorService* getInstance();

    // Deleted copy constructor and assignment operator
    ActuatorService(const ActuatorService&) = delete;
    ActuatorService& operator=(const ActuatorService&) = delete;

    // Méthodes de ManagedComponent
    ErrorCode initialize() override;
    void run();
    ErrorCode shutdown();

    // Interface publique pour contrôler les actionneurs
    // Les commandes sont généralement normalisées (ex: -1.0 à 1.0) ou en unités physiques (degrés, RPM)

    // Treuil
    ErrorCode setWinchSpeed(float normalizedSpeed); // -1.0 (dérouler max) à 1.0 (enrouler max)
    float getWinchSpeed() const; // Retourne la dernière consigne de vitesse
    // Potentiellement: bool setWinchPosition(float position); si le treuil a un encodeur

    // Servos (identifiés par un ID ou un index)
    ErrorCode setServoAngle(uint8_t servoId, float angleDegrees); // Angle en degrés
    float getServoAngle(uint8_t servoId) const; // Retourne la dernière consigne d'angle
    ErrorCode setServoNormalizedPosition(uint8_t servoId, float normalizedPosition); // -1.0 à 1.0

    // Méthode pour mettre tous les actionneurs dans un état sûr/neutre
    void setSafeState();

private:
    // Private constructor and destructor for singleton
    ActuatorService();
    ~ActuatorService() override;

    static ActuatorService* instance;
    static SemaphoreHandle_t instanceMutex;

    // Instances des drivers HAL pour les actionneurs
    // Utilisation de unique_ptr pour gérer la durée de vie et éviter les fuites de mémoire
    std::unique_ptr<WinchDriver> winchMotor;
    std::vector<std::unique_ptr<ServoDriver>> servos;

    // Variables pour stocker les dernières consignes (optionnel, mais utile pour getX())
    float currentWinchSpeedCommand;
    std::vector<float> currentServoAngleCommands; // Stocke les angles en degrés

    // Mutex pour protéger l'accès concurrentiel aux commandes si nécessaire
    // (Surtout si run() modifie des états basés sur des timers, etc.)
    // SemaphoreHandle_t commandMutex;
};

#endif // ACTUATOR_SERVICE_H
