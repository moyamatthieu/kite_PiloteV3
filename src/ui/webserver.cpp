#include "ui/webserver.h"
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include <string>

class WebserverCommunicationModule : public Module {
public:
    WebserverCommunicationModule() : Module("Webserver"), fsm("Webserver_FSM"), errorManager("Webserver") {}
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
        // handleWebserver();
    }
    void configure(const std::string& jsonConfig) {
        // Appliquer la configuration Webserver à partir d'un JSON
    }
    const char* description() const override { return "Serveur Web (communication)"; }
private:
    StateMachine fsm;
    ErrorManager errorManager;
};
static WebserverCommunicationModule webserverModule;
REGISTER_MODULE(&webserverModule);
