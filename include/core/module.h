#pragma once
#include <string>
#include <vector>
#include <functional>
#include "../common/global_enums.h" // Ajout de l'include pour les enums globaux

/**
 * Classe de base pour tous les modules dynamiques du système.
 * Permet une gestion unifiée de l'état, de l'activation, du nom, et du reporting.
 */
class Module {
public:
    // Utilisation de ComponentState de global_enums.h

    Module(const char* name, bool enabledByDefault = true)
        : _name(name), 
          _enabled(enabledByDefault), 
          _state(enabledByDefault ? ComponentState::ACTIVE : ComponentState::COMPONENT_DISABLED) {}

    virtual ~Module() {}

    // Nom du module (pour affichage, logs, dashboard)
    const char* name() const { return _name; }

    // Activation/désactivation dynamique
    virtual void enable()    { _enabled = true;  _state = ComponentState::ACTIVE;   onEnable(); }
    virtual void disable()   { _enabled = false; _state = ComponentState::COMPONENT_DISABLED;  onDisable(); }
    bool isEnabled()   const { return _enabled; }
    ComponentState state()      const { return _state; }

    // Pour reporting LCD/web : état lisible
    virtual const char* stateString() const {
        switch (_state) {
            case ComponentState::UNINITIALIZED: return "UNINIT";
            case ComponentState::INITIALIZING:  return "INIT...";
            case ComponentState::IDLE:          return "IDLE";
            case ComponentState::ACTIVE:        return "ACTIVE";
            case ComponentState::SUSPENDED:     return "SUSPEND";
            case ComponentState::ERROR:         return "ERROR";
            case ComponentState::COMPONENT_DISABLED: return "OFF";
            default:                            return "?";
        }
    }

    // Méthode appelée à chaque tick (optionnel)
    virtual void update() {}

    // Pour affichage LCD/web : description courte
    virtual const char* description() const { return ""; }

protected:
    // Surchargeable pour actions spécifiques
    virtual void onEnable()  {}
    virtual void onDisable() {}
    void setState(ComponentState s) { _state = s; }

private:
    const char* _name;
    bool _enabled;
    ComponentState _state;
};

/**
 * Gestionnaire global de modules (singleton)
 * Permet d'enregistrer, d'itérer et de manipuler tous les modules dynamiquement.
 */
class ModuleRegistry {
public:
    static ModuleRegistry& instance() {
        static ModuleRegistry reg;
        return reg;
    }
    void registerModule(Module* m) { _modules.push_back(m); }
    const std::vector<Module*>& modules() const { return _modules; }
    Module* getByName(const char* name) {
        for (auto* m : _modules) if (strcmp(m->name(), name) == 0) return m;
        return nullptr;
    }
private:
    std::vector<Module*> _modules;
    ModuleRegistry() = default;
    ~ModuleRegistry() = default;
    ModuleRegistry(const ModuleRegistry&) = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;
};

// Macro pour enregistrement automatique d'un module global
#define REGISTER_MODULE(MODULE_NAME, MODULE_PTR) \
    static struct ModuleAutoRegister_##MODULE_NAME##_t { \
        ModuleAutoRegister_##MODULE_NAME##_t() { ModuleRegistry::instance().registerModule(MODULE_PTR); } \
    } _autoRegister_##MODULE_NAME;

// Classe de base pour les modules capteurs
class SensorModule : public Module {
public:
    SensorModule(const char* name, bool enabledByDefault = true)
        : Module(name, enabledByDefault) {}
    // FSM dédiée (composition)
    // StateMachine fsm; // Pourrait utiliser ComponentState
    // Gestion d'erreur dédiée
    // ErrorManager errorManager; // Pourrait utiliser ErrorCode
    // Méthodes de configuration
    virtual void configure(const std::string& jsonConfig) {}
    // Méthode polymorphe pour lecture capteur
    virtual void readSensor() = 0;
};

// Classe de base pour les modules actionneurs
class ActuatorModule : public Module {
public:
    ActuatorModule(const char* name, bool enabledByDefault = true)
        : Module(name, enabledByDefault) {}
    // FSM dédiée (composition)
    // StateMachine fsm; // Pourrait utiliser ComponentState
    // Gestion d'erreur dédiée
    // ErrorManager errorManager; // Pourrait utiliser ErrorCode
    // Méthodes de configuration
    virtual void configure(const std::string& jsonConfig) {}
    // Méthode polymorphe pour actionner
    virtual void actuate() = 0;
};

// Exemple d'extension pour un module capteur IMU
class IMUSensorModule : public SensorModule {
public:
    IMUSensorModule() : SensorModule("IMU") {}
    void readSensor() override {
        // Appeler la FSM, lire l'IMU, gérer les erreurs, etc.
    }
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration spécifique IMU
    }
};

// Exemple d'extension pour un module actionneur Servo
class ServoActuatorModule : public ActuatorModule {
public:
    ServoActuatorModule() : ActuatorModule("Servo") {}
    void actuate() override {
        // Appeler la FSM, piloter le servo, gérer les erreurs, etc.
    }
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration spécifique Servo
    }
};
