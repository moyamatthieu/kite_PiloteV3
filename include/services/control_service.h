#ifndef CONTROL_SERVICE_H
#define CONTROL_SERVICE_H

#include "core/component.h"
#include "common/global_enums.h"
#include "core/config.h"
#include "control/pid.h"
#include "services/sensor_service.h" // Pour accéder aux données agrégées des capteurs
#include "services/actuator_service.h" // <<< AJOUTÉ

// Déclaration anticipée si ActuatorService est utilisé directement pour les commandes
// class ActuatorService; 

// Énumération pour les modes de contrôle spécifiques (exemple)
enum class ControlStrategy : uint8_t {
    MANUAL,
    LINE_TENSION_HOLD,
    ALTITUDE_HOLD,
    AUTO_FLIGHT_FIGURE_EIGHT, // Exemple de mode plus complexe
    LANDING,
    EMERGENCY_STOP
};

typedef struct {
    float targetLineTension;    // Newton
    float targetAltitude;       // Mètres
    float targetAzimuth;        // Degrés (par rapport au vent ou absolu)
    // ... autres paramètres de consigne
} ControlTargets_t;


class ControlService : public ManagedComponent {
public:
    ControlService();
    ~ControlService() override;

    // Méthodes de ManagedComponent
    ErrorCode initialize() override;
    void run(); // Supprimer override
    ErrorCode shutdown(); // Supprimer override

    // Interface publique du ControlService
    bool setControlStrategy(ControlStrategy strategy);
    ControlStrategy getCurrentControlStrategy() const;

    bool setControlTargets(const ControlTargets_t& targets);
    ControlTargets_t getControlTargets() const;
    
    // Méthode pour recevoir les données agrégées des capteurs
    void updateSensorData(const AggregatedSensorData& data);

private:
    void executeControlLoop(const AggregatedSensorData& sensors, ControlStrategy strategy, const ControlTargets_t& targets); // Ajout des paramètres manquants
    void applyActuatorCommands(float winchCommand, float servoLeftCommand, float servoRightCommand);

    // Stratégie de contrôle actuelle
    ControlStrategy currentStrategy;
    ControlTargets_t currentTargets;

    // Instances de PID (exemples)
    PIDController tensionPid;
    PIDController altitudePid;
    // PIDController azimuthPid;

    // Données capteurs locales (copie des dernières données reçues)
    AggregatedSensorData latestSensorData;
    bool sensorDataAvailable;
    unsigned long lastSensorDataTime;

    // Pointeur vers SensorService (si nécessaire pour des requêtes directes, sinon via callback/update)
    // SensorService* sensorServicePtr; 

    // Pointeur vers ActuatorService (sera nécessaire pour envoyer les commandes)
    ActuatorService* actuatorServicePtr; // <<< AJOUTÉ

    // Mutex pour protéger l'accès concurrentiel aux données partagées (stratégie, cibles)
    SemaphoreHandle_t dataMutex; 
};

#endif // CONTROL_SERVICE_H
