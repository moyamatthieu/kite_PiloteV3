#include "hardware/actuators/winch.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <string>

class WinchActuatorModule : public ActuatorModule {
public:
    WinchActuatorModule() : ActuatorModule("Winch"), fsm("Winch_FSM"), errorManager("Winch") {}

    void enable() override {
        ActuatorModule::enable();
        fsm.setState("IDLE");
    }
    void disable() override {
        ActuatorModule::disable();
        fsm.setState("DISABLED");
    }
    void update() override {
        fsm.tick();
        if (!isEnabled()) return;
        actuate();
    }
    void actuate() override {
        if (!winchInit()) {
            errorManager.reportError("Winch init failed");
            setState(State::ERROR);
            return;
        }
        // Exécution de la commande treuil
        // ...
        setState(State::ENABLED);
    }
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration Winch à partir d'un JSON
    }
    const char* description() const override { return "Actionneur Treuil"; }
private:
    StateMachine fsm;
    ErrorManager errorManager;
};

// Instanciation globale et enregistrement
static WinchActuatorModule winchModule;
REGISTER_MODULE(&winchModule);
