#ifndef SYSTEM_STATE_MANAGER_H
#define SYSTEM_STATE_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "common/global_enums.h" // Pour SystemState et potentiellement ErrorCode

// L'énumération SystemState est conservée telle quelle.
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
    
    // Récupère l'état système actuel
    SystemState getCurrentState();
    
    // Récupère la raison du dernier changement d'état
    const char* getLastStateChangeReason();
    
    // Récupère le temps écoulé depuis le dernier changement d'état (ms)
    unsigned long getTimeSinceLastStateChange();
    
    // Enregistre les états actuels dans le journal (uniquement l'état système global)
    void logSystemState();
};

#endif // SYSTEM_STATE_MANAGER_H