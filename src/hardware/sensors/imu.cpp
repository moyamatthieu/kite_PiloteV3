/*
  -----------------------
  Kite PiloteV3 - Module IMU (Implémentation)
  -----------------------
  
  Implémentation du module de gestion de l'unité de mesure inertielle (IMU).
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#if MODULE_SENSORS_ENABLED

#include "hardware/sensors/imu.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <string>

class IMUSensorModule : public SensorModule {
public:
    IMUSensorModule() : SensorModule("IMU"), fsm("IMU_FSM"), errorManager("IMU") {}

    void enable() override {
        SensorModule::enable();
        fsm.setState("IDLE");
    }
    void disable() override {
        SensorModule::disable();
        fsm.setState("DISABLED");
    }
    void update() override {
        fsm.tick();
        if (!isEnabled()) return;
        readSensor();
    }
    void readSensor() override {
        if (!imuInit(nullptr)) {
            errorManager.reportError("IMU init failed");
            setState(State::ERROR);
            return;
        }
        IMUData data;
        if (!imuReadProcessedData(&data)) {
            errorManager.reportError("IMU read failed");
            setState(State::ERROR);
            return;
        }
        setState(State::ENABLED);
    }
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration IMU à partir d'un JSON
    }
    const char* description() const override { return "Capteur IMU (orientation)"; }
private:
    StateMachine fsm;
    ErrorManager errorManager;
};

// Instanciation globale et enregistrement
static IMUSensorModule imuModule;
REGISTER_MODULE(&imuModule);

#else
// Module capteurs désactivé à la compilation (MODULE_SENSORS_ENABLED=0)
#endif
