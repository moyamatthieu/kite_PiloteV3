#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstring> // For strcmp
#include "../common/global_enums.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * Classe de base pour tous les composants gérés dynamiquement dans le système.
 * Fournit une interface commune pour la gestion de l'état, l'activation,
 * le nommage et le reporting.
 */
class ManagedComponent {
public:
    ManagedComponent(const char* name, bool enabledByDefault = true)
        : _name(name),
          _enabled(enabledByDefault),
          _state(enabledByDefault ? ComponentState::ACTIVE : ComponentState::COMPONENT_DISABLED),
          _taskHandle(nullptr) {}

    virtual ~ManagedComponent() = default;

    const char* getName() const { return _name; }

    virtual void enable() {
        _enabled = true;
        setState(ComponentState::ACTIVE);
        onEnable();
    }

    virtual void disable() {
        _enabled = false;
        setState(ComponentState::COMPONENT_DISABLED);
        onDisable();
    }

    bool isEnabled() const { return _enabled; }
    ComponentState getState() const { return _state; }

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

    // Méthode d'initialisation, à surcharger par les classes dérivées
    virtual ErrorCode initialize() {
        setState(ComponentState::INITIALIZING);
        setState(ComponentState::IDLE);
        return ErrorCode::OK;
    }

    // Méthode de mise à jour cyclique (optionnelle)
    virtual void update() {}

    // Description courte pour affichage/logs
    virtual const char* description() const { return ""; }

    // Méthodes pour la gestion des tâches FreeRTOS
    virtual bool startTask() { 
        return true; 
    }

    virtual void stopTask() {
        if (_taskHandle != nullptr) {
            vTaskDelete(_taskHandle);
            _taskHandle = nullptr;
        }
    }

    virtual TaskHandle_t getTaskHandle() const { 
        return _taskHandle;
    }

    virtual bool isTaskRunning() const {
        return _taskHandle != nullptr && eTaskGetState(_taskHandle) != eDeleted;
    }
    
    bool isInitialized() const {
        return _state == ComponentState::IDLE || 
               _state == ComponentState::ACTIVE || 
               _state == ComponentState::SUSPENDED;
    }
    
    const char* getComponentName() const { return getName(); }

protected:
    virtual void onEnable()  {}
    virtual void onDisable() {}
    void setState(ComponentState s) { _state = s; }
    TaskHandle_t _taskHandle;

private:
    const char* _name;
    bool _enabled;
    ComponentState _state;
};

/**
 * Registre global pour tous les ManagedComponent (Singleton).
 * Permet d'enregistrer, d'itérer et de manipuler les composants.
 */
class ComponentRegistry {
public:
    static ComponentRegistry& instance() {
        static ComponentRegistry reg;
        return reg;
    }
    void registerComponent(ManagedComponent* c) { _components.push_back(c); }
    const std::vector<ManagedComponent*>& components() const { return _components; }
    ManagedComponent* getByName(const char* name) {
        for (auto* c : _components) {
            if (c && c->getName() && name && strcmp(c->getName(), name) == 0) {
                return c;
            }
        }
        return nullptr;
    }
private:
    std::vector<ManagedComponent*> _components;
    ComponentRegistry() = default;
    ~ComponentRegistry() = default;
    ComponentRegistry(const ComponentRegistry&) = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;
};

// Macro pour l'enregistrement automatique d'un composant global
#define REGISTER_COMPONENT(COMPONENT_NAME, COMPONENT_PTR) \
    static struct ComponentAutoRegister_##COMPONENT_NAME##_t { \
        ComponentAutoRegister_##COMPONENT_NAME##_t() { ComponentRegistry::instance().registerComponent(COMPONENT_PTR); } \
    } _autoRegister_##COMPONENT_NAME;


// Classes de base pour des types spécifiques de composants
// NOTE: Ces classes sont maintenant définies dans hal_component.h
// Les définitions ci-dessous sont conservées temporairement pour compatibilité
// avec le code existant, mais seront supprimées dans une future version.
// Pour les nouveaux développements, utilisez les classes dans hal_component.h

/*
class SensorComponent : public ManagedComponent {
public:
    SensorComponent(const char* name, bool enabledByDefault = true)
        : ManagedComponent(name, enabledByDefault) {}
    
    virtual ErrorCode configure(const std::string& jsonConfig) { return ErrorCode::OK; }
    virtual void readSensor() = 0; // Méthode virtuelle pure
};

class ActuatorComponent : public ManagedComponent {
public:
    ActuatorComponent(const char* name, bool enabledByDefault = true)
        : ManagedComponent(name, enabledByDefault) {}

    virtual ErrorCode configure(const std::string& jsonConfig) { return ErrorCode::OK; }
    virtual void actuate() = 0; // Méthode virtuelle pure
};
*/

// Les exemples IMUSensorComponent et ServoActuatorComponent ont été supprimés.
// Ils seront implémentés comme des pilotes HAL dédiés dans leurs propres fichiers.
// Par exemple : include/hal/drivers/imu_driver.h et include/hal/drivers/servo_driver.h

