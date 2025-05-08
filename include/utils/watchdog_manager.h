#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#include <Arduino.h>
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "core/system_state_manager.h"

/**
 * Types de composants surveillés par le watchdog
 */
enum class ComponentType {
    SYSTEM,
    WIFI,
    DISPLAY,
    IMU,
    SERVOS,
    WINCH,
    AUTOPILOT,
    WEBSERVER,
    LINE_SENSOR,
    BUTTONS
};

/**
 * Actions à effectuer lors d'un watchdog timeout
 */
enum class WatchdogAction {
    NONE,                 // Ne rien faire
    LOG_ONLY,             // Uniquement journaliser l'erreur
    RESET_COMPONENT,      // Essayer de réinitialiser le composant
    RESET_SYSTEM,         // Redémarrer le système
    SAFE_MODE             // Passer en mode sécurisé
};

/**
 * Gestionnaire de watchdog amélioré pour surveiller les composants système
 * Cette classe permet de détecter les composants qui ne répondent plus et
 * de prendre des mesures appropriées.
 */
class WatchdogManager {
private:
    // Instance singleton
    static WatchdogManager* instance;
    
    // Mutex pour la protection des accès concurrents
    SemaphoreHandle_t watchdogMutex;
    
    // Handle pour la tâche watchdog
    TaskHandle_t watchdogTaskHandle;
    
    // Timestamp du dernier heartbeat pour chaque composant
    std::map<ComponentType, unsigned long> lastHeartbeats;
    
    // Seuils de timeout pour chaque composant (ms)
    std::map<ComponentType, unsigned long> timeoutThresholds;
    
    // Actions à effectuer en cas de timeout pour chaque composant
    std::map<ComponentType, WatchdogAction> timeoutActions;
    
    // Compteurs d'erreurs pour chaque composant
    std::map<ComponentType, uint32_t> errorCounts;
    
    // État d'activité pour chaque composant
    std::map<ComponentType, bool> componentActive;
    
    // Temps de surveillance (ms)
    unsigned long checkInterval;
    
    // ESP32 Hardware Watchdog activé?
    bool hardwareWatchdogEnabled;
    
    // Constructeur privé (pattern Singleton)
    WatchdogManager();
    
    // Fonction statique pour la tâche FreeRTOS
    static void watchdogTaskFunction(void* parameters);
    
    // Fonction interne pour vérifier tous les composants
    void checkAllComponents();
    
    // Fonction interne pour effectuer l'action appropriée sur un composant défaillant
    void performTimeoutAction(ComponentType component);

public:
    // Destructeur
    ~WatchdogManager();
    
    // Point d'accès pour l'instance Singleton
    static WatchdogManager* getInstance();
    
    /**
     * Initialise le watchdog manager et démarre la tâche de surveillance
     * @param checkIntervalMs Intervalle entre les vérifications en ms
     * @return true si l'initialisation a réussi, false sinon
     */
    bool init(unsigned long checkIntervalMs = 1000);
    
    /**
     * Enregistre un composant à surveiller
     * @param component Type de composant
     * @param timeoutMs Seuil de timeout en ms
     * @param action Action à effectuer en cas de timeout
     * @return true si l'enregistrement a réussi, false sinon
     */
    bool registerComponent(ComponentType component, unsigned long timeoutMs, WatchdogAction action = WatchdogAction::LOG_ONLY);
    
    /**
     * Désenregistre un composant (arrête sa surveillance)
     * @param component Type de composant
     * @return true si le désenregistrement a réussi, false sinon
     */
    bool unregisterComponent(ComponentType component);
    
    /**
     * Signale que le composant est toujours actif (heartbeat)
     * @param component Type de composant
     * @return true si le heartbeat a été enregistré, false sinon
     */
    bool heartbeat(ComponentType component);
    
    /**
     * Modifie l'intervalle entre les vérifications
     * @param intervalMs Nouvel intervalle en ms
     */
    void setCheckInterval(unsigned long intervalMs);
    
    /**
     * Active ou désactive le watchdog matériel de l'ESP32
     * @param enable true pour activer, false pour désactiver
     * @param timeoutSeconds Timeout en secondes pour le watchdog matériel
     * @return true si le changement a réussi, false sinon
     */
    bool enableHardwareWatchdog(bool enable, uint32_t timeoutSeconds = 30);
    
    /**
     * Vérifie si un composant est actuellement surveillé
     * @param component Type de composant
     * @return true si le composant est surveillé, false sinon
     */
    bool isComponentMonitored(ComponentType component);
    
    /**
     * Récupère le nombre d'erreurs pour un composant
     * @param component Type de composant
     * @return Nombre d'erreurs détectées
     */
    uint32_t getErrorCount(ComponentType component);
    
    /**
     * Récupère le temps écoulé depuis le dernier heartbeat
     * @param component Type de composant
     * @return Temps écoulé en ms
     */
    unsigned long getTimeSinceLastHeartbeat(ComponentType component);
    
    /**
     * Réinitialise le compteur d'erreurs pour un composant
     * @param component Type de composant
     */
    void resetErrorCount(ComponentType component);
    
    /**
     * Suspend temporairement la surveillance d'un composant
     * @param component Type de composant
     * @param suspend true pour suspendre, false pour reprendre
     * @return true si le changement a réussi, false sinon
     */
    bool suspendComponentMonitoring(ComponentType component, bool suspend);
};

#endif // WATCHDOG_MANAGER_H