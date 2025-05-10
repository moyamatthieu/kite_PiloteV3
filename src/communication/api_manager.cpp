/*
  -----------------------
  Kite PiloteV3 - Module API (Implémentation)
  -----------------------
  
  Implémentation de l'API REST pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "communication/api_manager.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <ArduinoJson.h>
#include <vector>
#include <string>

class APIFSM : public StateMachine {
public:
    APIFSM() : StateMachine("API_FSM", 0) {}
protected:
    int processState(int state) override {
        // Implémenter la logique d'état ici
        return state;
    }
};

class APICommunicationModule : public Module {
public:
    APICommunicationModule() : Module("API"), fsm(), errorManager(*ErrorManager::getInstance()) {}
    void enable() override {
        Module::enable();
        fsm.transitionTo(0, 0, "Enable");
    }
    void disable() override {
        Module::disable();
        fsm.transitionTo(-1, 0, "Disable");
    }
    void update() override {
        fsm.update();
        if (!isEnabled()) return;
        // handleAPI();
    }
    void configure(const std::string& jsonConfig) {
        // Appliquer la configuration API à partir d'un JSON
    }
    const char* description() const override { return "Module API (communication)"; }
private:
    APIFSM fsm;
    ErrorManager& errorManager;
};
static APICommunicationModule apiModule;
REGISTER_MODULE(apiModule, &apiModule);
