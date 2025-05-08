#ifndef SYSTEM_STATE_MANAGER_H
#define SYSTEM_STATE_MANAGER_H

#include <Arduino.h>
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Énumération des états système principaux
enum class SystemState {
    INIT,           // Initialisation en cours
    READY,          // Prêt à fonctionner
    RUNNING,        // En fonctionnement normal
    ERROR,          // Erreur détectée
    POWER_SAVE,     // Mode économie d'énergie
    UPDATE,         // Mise à jour en cours
    CALIBRATION,    // Calibration en cours
    SAFE_MODE,      // Mode sécurisé
    SHUTDOWN        // Arrêt en cours
};

// Énumération des composants système
enum class SystemComponent {
    WIFI,           // Module WiFi
    SERVOS,         // Servomoteurs
    WINCH,          // Treuil
    IMU,            // Unité de mesure inertielle
    LCD_DISPLAY,    // Affichage LCD (renommé pour éviter le conflit avec Arduino)
    BUTTONS,        // Interface boutons
    AUTOPILOT,      // Pilote automatique
    POWER,          // Gestion de l'alimentation
    WEBSERVER,      // Serveur web
    LINE_SENSOR     // Capteur de longueur de ligne
};

// États possibles pour les composants
enum class ComponentState {
    NOT_INITIALIZED,    // Non initialisé
    INITIALIZING,       // En cours d'initialisation
    OPERATIONAL,        // Opérationnel
    ERROR,              // En erreur
    RECOVERING,         // En cours de récupération
    INACTIVE,           // Désactivé volontairement
    POWER_SAVE,         // En mode économie d'énergie
    CALIBRATING         // En cours de calibration
}; // Fin de l'énumération ComponentState

// Structure pour les transitions d'état
struct StateTransition {
    SystemState from;
    SystemState to;
    bool isValid;
    const char* reason;
};

/**
 * Gestionnaire centralisé des états du système
 * Implémente un pattern Singleton pour assurer une instance unique
 */
class SystemStateManager {
private:
    // Instance singleton
    static SystemStateManager* instance;
    
    // État système actuel
    SystemState currentState;
    
    // États des différents composants
    std::map<SystemComponent, ComponentState> componentStates;
    
    // Mutex pour la protection des accès concurrents
    SemaphoreHandle_t stateMutex;
    
    // Timestamp de la dernière transition d'état
    unsigned long lastStateChangeTime;
    
    // Raison du dernier changement d'état
    char lastStateChangeReason[64];
    
    // Constructeur privé (Singleton)
    SystemStateManager();
    
    // Matrice des transitions d'état valides
    StateTransition validTransitions[20];
    int transitionCount;
    
    // Initialise la matrice des transitions valides
    void initializeTransitionMatrix();
    
    // Vérifie si une transition est valide
    bool isValidTransition(SystemState from, SystemState to);

public:
    // Destructeur
    ~SystemStateManager();
    
    // Point d'accès pour l'instance Singleton
    static SystemStateManager* getInstance();
    
    // Tente une transition vers un nouvel état système
    bool transitionTo(SystemState newState, const char* reason = nullptr);
    
    // Met à jour l'état d'un composant
    bool updateComponentState(SystemComponent component, ComponentState state, const char* reason = nullptr);
    
    // Récupère l'état système actuel
    SystemState getCurrentState();
    
    // Récupère l'état d'un composant spécifique
    ComponentState getComponentState(SystemComponent component);
    
    // Récupère la raison du dernier changement d'état
    const char* getLastStateChangeReason();
    
    // Récupère le temps écoulé depuis le dernier changement d'état (ms)
    unsigned long getTimeSinceLastStateChange();
    
    // Vérifie si tous les composants sont dans un état spécifique
    bool areAllComponentsInState(ComponentState state);
    
    // Vérifie si un composant spécifique est dans un état donné
    bool isComponentInState(SystemComponent component, ComponentState state);
    
    // Réinitialise les états des composants
    void resetComponentStates();
    
    // Enregistre les états actuels dans le journal
    void logSystemState();
};

#endif // SYSTEM_STATE_MANAGER_H