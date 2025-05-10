#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <Arduino.h>
#include <map>
#include <string>
#include <vector>
#include <functional>

/**
 * @brief Enum représentant l'état de santé d'un composant
 */
enum class ComponentHealth {
    UNKNOWN,    // État inconnu
    HEALTHY,    // Fonctionne correctement
    WARNING,    // Problèmes mineurs détectés
    CRITICAL,   // Problèmes critiques détectés
    FAILURE     // Ne fonctionne pas
};

/**
 * @brief Structure contenant les informations de diagnostic d'un composant
 */
struct ComponentStatus {
    ComponentHealth health;
    String details;
    uint32_t lastUpdated;
    uint16_t errorCount;
    uint16_t successCount;
};

/**
 * @brief Gestionnaire de diagnostics du système
 * 
 * Cette classe permet de centraliser les informations de diagnostic
 * de tous les composants du système (capteurs, actionneurs, communication, etc.)
 */
class DiagnosticsManager {
public:
    /**
     * @brief Obtenir l'instance unique du gestionnaire de diagnostics
     * @return DiagnosticsManager& Instance unique
     */
    static DiagnosticsManager& getInstance();

    /**
     * @brief Initialiser le gestionnaire de diagnostics
     */
    void init();

    /**
     * @brief Mettre à jour l'état d'un composant
     * @param componentName Nom du composant
     * @param health État de santé du composant
     * @param details Informations détaillées sur l'état
     */
    void updateComponentStatus(const String& componentName, ComponentHealth health, const String& details = "");

    /**
     * @brief Signaler une erreur sur un composant
     * @param componentName Nom du composant
     * @param errorDetails Description de l'erreur
     */
    void reportError(const String& componentName, const String& errorDetails);

    /**
     * @brief Signaler un succès sur un composant
     * @param componentName Nom du composant
     */
    void reportSuccess(const String& componentName);

    /**
     * @brief Obtenir l'état d'un composant
     * @param componentName Nom du composant
     * @return ComponentStatus État du composant
     */
    ComponentStatus getComponentStatus(const String& componentName);

    /**
     * @brief Obtenir la liste de tous les composants surveillés
     * @return std::vector<String> Liste des noms des composants
     */
    std::vector<String> getAllComponents();

    /**
     * @brief Obtenir un JSON avec l'état de tous les composants
     * @return String JSON avec l'état de tous les composants
     */
    String getSystemStatusJson();

    /**
     * @brief Enregistrer une fonction de rappel à appeler lorsque l'état d'un composant change
     * @param callback Fonction à appeler
     */
    void registerStatusChangeCallback(std::function<void(const String&, ComponentHealth)> callback);

private:
    DiagnosticsManager() {}
    ~DiagnosticsManager() {}
    DiagnosticsManager(const DiagnosticsManager&) = delete;
    DiagnosticsManager& operator=(const DiagnosticsManager&) = delete;

    std::map<String, ComponentStatus> _componentStatus;
    std::vector<std::function<void(const String&, ComponentHealth)>> _statusChangeCallbacks;
};

// Constantes pour les noms des composants
namespace DiagnosticsComponents {
    // Capteurs
    constexpr const char* IMU = "IMU";
    constexpr const char* TENSION = "TENSION";
    constexpr const char* LINE_LENGTH = "LINE_LENGTH";
    constexpr const char* WIND = "WIND";
    
    // Actionneurs
    constexpr const char* SERVO = "SERVO";
    constexpr const char* WINCH = "WINCH";
    constexpr const char* GENERATOR = "GENERATOR";
    
    // Communication
    constexpr const char* WIFI = "WIFI";
    constexpr const char* ESPNOW = "ESPNOW";
    constexpr const char* WEBSERVER = "WEBSERVER";
    
    // Système
    constexpr const char* MEMORY = "MEMORY";
    constexpr const char* FILE_SYSTEM = "FILE_SYSTEM";
    constexpr const char* BATTERY = "BATTERY";
}

class DisplayManager;

class OTAManager {
public:
    static void handleOTAEnd(bool success, int ledPin, DisplayManager& display);
};

#endif // DIAGNOSTICS_H