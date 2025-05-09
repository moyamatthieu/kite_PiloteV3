/*
  -----------------------
  Kite PiloteV3 - Module Wind Sensor (Implémentation)
  -----------------------
  
  Implémentation du module de gestion du capteur de vent.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/wind.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <string>

class WindSensorModule : public SensorModule {
public:
    WindSensorModule() : SensorModule("Wind"), fsm("Wind_FSM"), errorManager("Wind") {}

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
        // Logique de lecture du capteur de vent, gestion d'erreur
        if (!windInit()) {
            errorManager.reportError("Wind init failed");
            setState(State::ERROR);
            return;
        }
        WindData data;
        if (!windReadProcessedData(&data)) {
            errorManager.reportError("Wind read failed");
            setState(State::ERROR);
            return;
        }
        setState(State::ENABLED);
    }
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration Wind à partir d'un JSON
    }
    const char* description() const override { return "Capteur de vent"; }
private:
    StateMachine fsm;
    ErrorManager errorManager;
};

// Instanciation globale et enregistrement
static WindSensorModule windModule;
REGISTER_MODULE(&windModule);