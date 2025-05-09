/*
  -----------------------
  Kite PiloteV3 - Module API (Implémentation)
  -----------------------
  
  Implémentation de l'API REST pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "communication/api.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <ArduinoJson.h>
#include <vector>
#include <string>

class APICommunicationModule : public Module {
public:
    APICommunicationModule() : Module("API"), fsm("API_FSM"), errorManager("API") {}
    void enable() override {
        Module::enable();
        fsm.setState("IDLE");
    }
    void disable() override {
        Module::disable();
        fsm.setState("DISABLED");
    }
    void update() override {
        fsm.tick();
        if (!isEnabled()) return;
        // handleAPI();
    }
    void configure(const std::string& jsonConfig) {
        // Appliquer la configuration API à partir d'un JSON
    }
    const char* description() const override { return "API REST (communication)"; }
private:
    StateMachine fsm;
    ErrorManager errorManager;
};
static APICommunicationModule apiModule;
REGISTER_MODULE(&apiModule);
