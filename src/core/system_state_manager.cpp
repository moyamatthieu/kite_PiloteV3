/*
  -----------------------
  Kite PiloteV3 - Module de gestion d'états système (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire centralisé des états du système.
  
  Version: 3.0.0
  Date: 8 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module implémente un gestionnaire d'états qui centralise la gestion des états
  du système et de ses composants. Il utilise le pattern Singleton pour assurer
  une instance unique et thread-safe grâce à des mutex FreeRTOS.
  
  Principes de fonctionnement :
  1. Gestion centralisée des états système et composants via un Singleton
  2. Protection thread-safe avec mutex pour environnement multitâche
  3. Validation des transitions d'état via une matrice de transitions
  4. Journalisation des changements d'état pour diagnostics
  
  Interactions avec d'autres modules :
  - System : Utilise ce gestionnaire pour suivre l'état global
  - TaskManager : Prend des décisions basées sur les états
  - Modules matériels : Signalent leur état via ce gestionnaire
*/

#include <map>
#include <mutex>
#include "core/system_state_manager.h"
#include "utils/logging.h"

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
    
    // Initialisation du timestamp
    lastStateChangeTime = millis();
    
    // Initialisation de la raison du dernier changement
    strcpy(lastStateChangeReason, "Initialisation du système");
    
    // Initialisation du compteur de transitions
    transitionCount = 0;
    
    // Initialisation de la matrice des transitions valides
    initializeTransitionMatrix();
    
    // Initialisation des états des composants
    resetComponentStates();
    
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
}

/**
 * Point d'accès pour l'instance Singleton
 * @return Pointeur vers l'instance unique du gestionnaire d'états
 */
SystemStateManager* SystemStateManager::getInstance() {
    // Création de l'instance si elle n'existe pas encore
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
 * Met à jour l'état d'un composant
 * @param component Composant à mettre à jour
 * @param state Nouvel état du composant
 * @param reason Raison du changement d'état (optionnel)
 * @return true si la mise à jour a réussi, false sinon
 */
bool SystemStateManager::updateComponentState(SystemComponent component, ComponentState state, const char* reason) {
    // Acquisition du mutex
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour la mise à jour de l'état du composant");
        return false;
    }
    
    // Enregistrement de l'ancien état pour le log
    ComponentState oldState = ComponentState::NOT_INITIALIZED;
    if (componentStates.find(component) != componentStates.end()) {
        oldState = componentStates[component];
    }
    
    // Mise à jour de l'état du composant
    componentStates[component] = state;
    
    // Libération du mutex
    xSemaphoreGive(stateMutex);
    
    // Journalisation du changement d'état
    const char* reasonText = (reason != nullptr) ? reason : "Non spécifié";
    LOG_INFO("STATE", "État du composant %d: %d -> %d (%s)", 
             static_cast<int>(component), static_cast<int>(oldState), 
             static_cast<int>(state), reasonText);
    
    return true;
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
 * Récupère l'état d'un composant spécifique
 * @param component Composant dont on souhaite connaître l'état
 * @return État du composant
 */
ComponentState SystemStateManager::getComponentState(SystemComponent component) {
    // Acquisition du mutex
    ComponentState state;
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Vérification de l'existence du composant dans la map
        if (componentStates.find(component) != componentStates.end()) {
            state = componentStates[component];
        } else {
            state = ComponentState::NOT_INITIALIZED;
        }
        
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour la lecture de l'état du composant");
        // Retourner un état par défaut en cas d'échec
        state = ComponentState::ERROR;
    }
    
    return state;
}

/**
 * Récupère la raison du dernier changement d'état
 * @return Raison du dernier changement d'état
 */
const char* SystemStateManager::getLastStateChangeReason() {
    // Cette méthode doit être thread-safe
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
 * Vérifie si tous les composants sont dans un état spécifique
 * @param state État à vérifier
 * @return true si tous les composants sont dans l'état spécifié, false sinon
 */
bool SystemStateManager::areAllComponentsInState(ComponentState state) {
    bool result = true;
    
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Si aucun composant n'est enregistré, retourner false
        if (componentStates.empty()) {
            result = false;
        } else {
            // Vérifier l'état de chaque composant
            for (const auto& pair : componentStates) {
                if (pair.second != state) {
                    result = false;
                    break;
                }
            }
        }
        
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour vérifier l'état des composants");
        result = false;
    }
    
    return result;
}

/**
 * Vérifie si un composant spécifique est dans un état donné
 * @param component Composant à vérifier
 * @param state État à vérifier
 * @return true si le composant est dans l'état spécifié, false sinon
 */
bool SystemStateManager::isComponentInState(SystemComponent component, ComponentState state) {
    bool result = false;
    
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Vérification de l'existence du composant dans la map
        if (componentStates.find(component) != componentStates.end()) {
            result = (componentStates[component] == state);
        }
        
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour vérifier l'état du composant");
    }
    
    return result;
}

/**
 * Réinitialise les états des composants
 */
void SystemStateManager::resetComponentStates() {
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Initialisation de tous les composants à NOT_INITIALIZED
        componentStates.clear();
        componentStates[SystemComponent::WIFI] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::SERVOS] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::WINCH] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::IMU] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::LCD_DISPLAY] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::BUTTONS] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::AUTOPILOT] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::POWER] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::WEBSERVER] = ComponentState::NOT_INITIALIZED;
        componentStates[SystemComponent::LINE_SENSOR] = ComponentState::NOT_INITIALIZED;
        
        xSemaphoreGive(stateMutex);
        
        LOG_INFO("STATE", "États des composants réinitialisés");
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour réinitialiser les états des composants");
    }
}

/**
 * Enregistre les états actuels dans le journal
 */
void SystemStateManager::logSystemState() {
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        LOG_INFO("STATE", "État système actuel: %d (%s)", 
                 static_cast<int>(currentState), lastStateChangeReason);
        
        // Log de l'état de chaque composant
        LOG_INFO("STATE", "États des composants:");
        for (const auto& pair : componentStates) {
            LOG_INFO("STATE", "  Composant %d: %d", 
                     static_cast<int>(pair.first), static_cast<int>(pair.second));
        }
        
        xSemaphoreGive(stateMutex);
    } else {
        LOG_ERROR("STATE", "Impossible d'acquérir le mutex pour journaliser l'état système");
    }
}