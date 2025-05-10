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
#include "utils/error_manager.h"
#include <string>

// Implémentation des fonctions du module wind
bool windInit() {
    // Initialisation du capteur de vent
    return true;
}

WindData windReadProcessedData() {
    WindData data;
    
    // Simuler la lecture des données
    data.speed = 5.0f;       // 5 m/s
    data.direction = 180.0f; // 180 degrés (sud)
    data.gust = 7.5f;        // 7.5 m/s
    data.timestamp = millis();
    data.isValid = true;
    
    return data;
}

// Classe pour le module de vent
class WindModule : public SensorModule {
public:
    WindModule() : SensorModule("Wind") {}

    void readSensor() override {
        if (!windInit()) {
            // Utiliser l'instance singleton d'ErrorManager
            ErrorManager::getInstance()->reportError(
                ErrorCode::SENSOR_ERROR,
                "Wind",
                "Wind initialization failed"
            );
            setState(State::MODULE_ERROR);
            return;
        }
        
        WindData data = windReadProcessedData();
        if (!data.isValid) {
            ErrorManager::getInstance()->reportError(
                ErrorCode::SENSOR_ERROR,
                "Wind",
                "Wind read failed"
            );
            setState(State::MODULE_ERROR);
            return;
        }
        
        setState(State::MODULE_ENABLED);
    }
    
    void configure(const std::string& jsonConfig) override {
        // Appliquer la configuration Wind à partir d'un JSON
    }
    
    const char* description() const override {
        return "Capteur de vent";
    }
};

// Instanciation globale et enregistrement
static WindModule windModule;
REGISTER_MODULE(windModule, &windModule);