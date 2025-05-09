/*
  -----------------------
  Kite PiloteV3 - Module Line Length Sensor (Implémentation)
  -----------------------
  
  Implémentation du module de gestion du capteur de longueur de ligne.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/line_length.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <string>

class LineLengthSensorModule : public SensorModule {
public:
    LineLengthSensorModule() : SensorModule("LineLength"), fsm("LineLength_FSM"), errorManager("LineLength") {}

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
        if (!lineLengthInit()) {
            errorManager.reportError("LineLength init failed");
            setState(State::ERROR);
            return;
        }
        LineLengthData data;
        if (!lineLengthReadProcessedData(&data)) {
            errorManager.reportError("LineLength read failed");
            setState(State::ERROR);
            return;
        }
        setState(State::ENABLED);
    }
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration LineLength à partir d'un JSON
    }
    const char* description() const override { return "Capteur longueur de ligne"; }
private:
    StateMachine fsm;
    ErrorManager errorManager;
};

// Instanciation globale et enregistrement
static LineLengthSensorModule lineLengthModule;
REGISTER_MODULE(&lineLengthModule);