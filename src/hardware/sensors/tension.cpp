/*
  -----------------------
  Kite PiloteV3 - Module Tension Sensor (Implémentation)
  -----------------------
  
  Implémentation du module de gestion du capteur de tension.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/tension.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <string>

class TensionSensorModule : public SensorModule {
public:
    TensionSensorModule() : SensorModule("Tension"), fsm("Tension_FSM"), errorManager("Tension") {}

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
        if (!tensionInit()) {
            errorManager.reportError("Tension init failed");
            setState(State::ERROR);
            return;
        }
        TensionData data;
        if (!tensionReadProcessedData(&data)) {
            errorManager.reportError("Tension read failed");
            setState(State::ERROR);
            return;
        }
        setState(State::ENABLED);
    }
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration Tension à partir d'un JSON
    }
    const char* description() const override { return "Capteur de tension"; }
private:
    StateMachine fsm;
    ErrorManager errorManager;
};

// Instanciation globale et enregistrement
static TensionSensorModule tensionModule;
REGISTER_MODULE(&tensionModule);