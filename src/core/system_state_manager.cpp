/*
  -----------------------
  Kite PiloteV3 - Module de gestion d'états système (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire centralisé des états du système.
  
  Version: 3.0.1 
  Date: 13 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module implémente un gestionnaire d'états qui centralise la gestion de l'état global
  du système. Il utilise le pattern Singleton pour assurer une instance unique et 
  thread-safe grâce à des mutex FreeRTOS.
  
  Principes de fonctionnement :
  1. Gestion centralisée de l'état système global via un Singleton
  2. Protection thread-safe avec mutex pour environnement multitâche
  3. Validation des transitions d'état via une matrice de transitions
  4. Journalisation des changements d'état pour diagnostics
  
  Interactions avec d'autres modules :
  - Les ManagedComponents peuvent interroger l'état global du système.
  - Le SystemOrchestrator (ou équivalent) utilisera ce manager pour les décisions de haut niveau.
*/

#include "core/system_state_manager.h"
#include "core/logging.h"

// Initialisation de l'instance singleton
SystemStateManager* SystemStateManager::instance = nullptr;

/**
 * Constructeur privé (Pattern Singleton)
 */
SystemStateManager::SystemStateManager() {
    // Initialisation de l'état système
    currentState = SystemState::INIT;
    
    // Création du mutex pour la protection des accès concurrents
    stateMutex = xSemaphoreCreateMutex();
    if (stateMutex == nullptr) {
        LOG_ERROR("STATE", "Échec critique: Impossible de créer stateMutex.");
    }
    
    // Initialisation du timestamp
    lastStateChangeTime = millis();
    
    // Initialisation de la raison du dernier changement
    strncpy(lastStateChangeReason, "Initialisation du système", sizeof(lastStateChangeReason) - 1);
    lastStateChangeReason[sizeof(lastStateChangeReason) - 1] = '\0';
    
    // Initialisation du compteur de transitions
    transitionCount = 0;
    
    // Initialisation de la matrice des transitions valides
    initializeTransitionMatrix();
    
    LOG_INFO("STATE", "Gestionnaire d'états système initialisé");
}

/**
 * Destructeur
 */
SystemStateManager::~SystemStateManager() {
    // Libération du mutex
    if (stateMutex != nullptr) {
        vSemaphoreDelete(stateMutex);
        stateMutex = nullptr;
    }
    LOG_INFO("STATE", "Gestionnaire d'états système détruit.");
}

/**
 * Point d'accès pour l'instance Singleton
 * @return Pointeur vers l'instance unique du gestionnaire d'états
 */
SystemStateManager* SystemStateManager::getInstance() {
    if (instance == nullptr) {
        instance = new SystemStateManager();
    }
    
    return instance;
}

/**
 * Initialise la matrice des transitions d'état valides
 */
void SystemStateManager::initializeTransitionMatrix() {
    // Définition des transitions valides
    
    // Depuis INIT
    validTransitions[transitionCount++] = {SystemState::INIT, SystemState::READY, true, "Initialisation terminée"};
    validTransitions[transitionCount++] = {SystemState::INIT, SystemState::ERROR, true, "Erreur durant l'initialisation"};
    
    // Depuis READY
    validTransitions[transitionCount++] = {SystemState::READY, SystemState::RUNNING, true, "Démarrage du système"};
    validTransitions[transitionCount++] = {SystemState::READY, SystemState::CALIBRATION, true, "Début de calibration"};
    validTransitions[transitionCount++] = {SystemState::READY, SystemState::ERROR, true, "Erreur détectée"};
    validTransitions[transitionCount++] = {SystemState::READY, SystemState::UPDATE, true, "Début de mise à jour"};
    validTransitions[transitionCount++] = {SystemState::READY, SystemState::SHUTDOWN, true, "Arrêt demandé"};
    
    // Depuis RUNNING
    validTransitions[transitionCount++] = {SystemState::RUNNING, SystemState::READY, true, "Arrêt du fonctionnement"};
    validTransitions[transitionCount++] = {SystemState::RUNNING, SystemState::ERROR, true, "Erreur durant l'exécution"};
    validTransitions[transitionCount++] = {SystemState::RUNNING, SystemState::POWER_SAVE, true, "Mode économie d'énergie activé"};
    validTransitions[transitionCount++] = {SystemState::RUNNING, SystemState::SAFE_MODE, true, "Mode sécurisé activé"};
    validTransitions[transitionCount++] = {SystemState::RUNNING, SystemState::SHUTDOWN, true, "Arrêt demandé"};
    
    // Depuis ERROR
    validTransitions[transitionCount++] = {SystemState::ERROR, SystemState::SAFE_MODE, true, "Passage en mode sécurisé"};
    validTransitions[transitionCount++] = {SystemState::ERROR, SystemState::INIT, true, "Réinitialisation après erreur"};
    validTransitions[transitionCount++] = {SystemState::ERROR, SystemState::SHUTDOWN, true, "Arrêt après erreur"};
    
    // Depuis POWER_SAVE
    validTransitions[transitionCount++] = {SystemState::POWER_SAVE, SystemState::RUNNING, true, "Sortie du mode économie d'énergie"};
    validTransitions[transitionCount++] = {SystemState::POWER_SAVE, SystemState::ERROR, true, "Erreur en mode économie d'énergie"};
    
    // Depuis UPDATE
    validTransitions[transitionCount++] = {SystemState::UPDATE, SystemState::INIT, true, "Redémarrage après mise à jour"};
    validTransitions[transitionCount++] = {SystemState::UPDATE, SystemState::ERROR, true, "Erreur durant la mise à jour"};
    
    // Depuis CALIBRATION
    validTransitions[transitionCount++] = {SystemState::CALIBRATION, SystemState::READY, true, "Calibration terminée"};
    validTransitions[transitionCount++] = {SystemState::CALIBRATION, SystemState::ERROR, true, "Erreur durant la calibration"};
    
    // Depuis SAFE_MODE
    validTransitions[transitionCount++] = {SystemState::SAFE_MODE, SystemState::INIT, true, "Réinitialisation depuis mode sécurisé"};
    validTransitions[transitionCount++] = {SystemState::SAFE_MODE, SystemState::SHUTDOWN, true, "Arrêt depuis mode sécurisé"};
}

/**
 * Vérifie si une transition est valide
 * @param from État source
 * @param to État destination
 * @return true si la transition est valide, false sinon
 */
bool SystemStateManager::isValidTransition(SystemState from, SystemState to) {
    for (int i = 0; i < transitionCount; i++) {
        if (validTransitions[i].from == from && validTransitions[i].to == to) {
            return validTransitions[i].isValid;
        }
    }
    
    // Transition non trouvée dans la matrice
    return false;
}

/**
 * Tente une transition vers un nouvel état système
 * @param newState Nouvel état souhaité
 * @param reason Raison du changement d'état (optionnel)
 * @return true si la transition a réussi, false sinon
 */
bool SystemStateManager::transitionTo(SystemState newState, const char* reason) {
    // Vérification de la validité de la transition
    bool success = false;
    
    // Acquisition du mutex
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour la transition d'état");
        return false;
    }
    
    // Vérification de la validité de la transition
    if (isValidTransition(currentState, newState)) {
        // Enregistrement de l'ancien état pour le log
        SystemState oldState = currentState;
        
        // Mise à jour de l'état
        currentState = newState;
        lastStateChangeTime = millis();
        
        // Mise à jour de la raison
        if (reason != nullptr) {
            strncpy(lastStateChangeReason, reason, sizeof(lastStateChangeReason) - 1);
            lastStateChangeReason[sizeof(lastStateChangeReason) - 1] = '\0';
        } else {
            // Recherche de la raison par défaut dans la matrice des transitions
            for (int i = 0; i < transitionCount; i++) {
                if (validTransitions[i].from == oldState && validTransitions[i].to == newState) {
                    strncpy(lastStateChangeReason, validTransitions[i].reason, sizeof(lastStateChangeReason) - 1);
                    lastStateChangeReason[sizeof(lastStateChangeReason) - 1] = '\0';
                    break;
                }
            }
        }
        
        // Journalisation du changement d'état
        LOG_INFO("STATE", "Transition d'état: %d -> %d (%s)", 
                 static_cast<int>(oldState), static_cast<int>(newState), lastStateChangeReason);
        
        success = true;
    } else {
        LOG_ERROR("STATE", "Transition d'état invalide: %d -> %d", 
                 static_cast<int>(currentState), static_cast<int>(newState));
        success = false;
    }
    
    // Libération du mutex
    xSemaphoreGive(stateMutex);
    
    return success;
}

/**
 * Récupère l'état système actuel
 * @return État système actuel
 */
SystemState SystemStateManager::getCurrentState() {
    // Acquisition du mutex
    SystemState state;
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        state = currentState;
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour la lecture de l'état système");
        // Retourner un état par défaut en cas d'échec
        state = SystemState::ERROR;
    }
    
    return state;
}

/**
 * Récupère la raison du dernier changement d'état
 * @return Raison du dernier changement d'état
 */
const char* SystemStateManager::getLastStateChangeReason() {
    char* reason = nullptr;
    
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        reason = lastStateChangeReason;
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour la lecture de la raison du changement d'état");
    }
    
    return reason;
}

/**
 * Récupère le temps écoulé depuis le dernier changement d'état (ms)
 * @return Temps écoulé en millisecondes
 */
unsigned long SystemStateManager::getTimeSinceLastStateChange() {
    unsigned long time = 0;
    
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        time = millis() - lastStateChangeTime;
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour la lecture du temps depuis le changement d'état");
    }
    
    return time;
}

/**
 * Enregistre les états actuels dans le journal
 */
void SystemStateManager::logSystemState() {
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        LOG_INFO("STATE", "État système actuel: %d (%s)", 
                 static_cast<int>(currentState), lastStateChangeReason);
        
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour journaliser l'état système");
    }
}