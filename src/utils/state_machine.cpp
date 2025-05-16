/*
  -----------------------
  Kite PiloteV3 - Framework de machines à états finis (Implémentation)
  -----------------------
  
  Implémentation de la classe de base pour les machines à états finis.
  
  Version: 3.0.0
  Date: 8 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module fournit un framework standardisé pour implémenter des machines à états
  finis dans le projet. Il garantit une approche non-bloquante et une gestion
  systématique des timeouts pour éviter les états bloqués.
  
  Principes de fonctionnement :
  1. Séparation claire entre la logique de transition et le traitement des états
  2. Gestion automatique des timeouts pour éviter les blocages
  3. Hooks pour les événements d'entrée/sortie d'état
  4. Journalisation standardisée des transitions
  
  Interactions avec d'autres modules :
  - Tous les modules utilisant des FSM : Base pour l'implémentation de FSM spécifiques
*/

#include "utils/state_machine.h"
#include "core/logging.h"

/**
 * Constructeur
 * @param name Nom de la machine à états (pour journalisation)
 * @param initialState État initial
 * @param defaultTimeout Timeout par défaut en millisecondes (0 = pas de timeout)
 * @param defaultErrorState État d'erreur par défaut
 */
StateMachine::StateMachine(const char* name, int initialState, unsigned long defaultTimeout, int defaultErrorState)
    : currentState(initialState),
      previousState(initialState),
      stateEntryTime(millis()),
      stateTimeout(defaultTimeout),
      timeoutEnabled(defaultTimeout > 0),
      machineName(name),
      errorState(defaultErrorState),
      timeoutState(defaultErrorState),
      transitionOccurred(false) {
    
    LOG_INFO("FSM", "Machine à états '%s' initialisée à l'état %d", machineName, initialState);
    
    // Appel du handler d'entrée pour l'état initial
    onEnterState(initialState, initialState);
}

/**
 * Destructeur
 */
StateMachine::~StateMachine() {
    LOG_INFO("FSM", "Machine à états '%s' détruite", machineName);
}

/**
 * Mise à jour de la machine à états
 * Cette méthode doit être appelée régulièrement pour faire avancer la FSM
 * @return État actuel après la mise à jour
 */
int StateMachine::update() {
    transitionOccurred = false;
    
    // Vérification du timeout
    if (timeoutEnabled && hasTimedOut()) {
        LOG_WARNING("FSM", "Timeout dans l'état %d de la machine '%s' après %lu ms",
                   currentState, machineName, millis() - stateEntryTime);
        
        // Appel du handler de timeout
        onTimeout(currentState);
        
        // Transition vers l'état de timeout si défini
        if (timeoutState >= 0) {
            transitionTo(timeoutState, 0, "Timeout");
        }
    }
    
    // Traitement de l'état courant
    int newState = processState(currentState);
    
    // Vérification si le traitement a demandé une transition
    if (newState != currentState && !transitionOccurred) {
        transitionTo(newState);
    }
    
    return currentState;
}

/**
 * Vérifie si un timeout s'est produit
 * @return true si le temps passé dans l'état actuel dépasse le timeout configuré
 */
bool StateMachine::hasTimedOut() const {
    return timeoutEnabled && (millis() - stateEntryTime > stateTimeout);
}

/**
 * Effectue une transition vers un nouvel état
 * @param newState Nouvel état
 * @param customTimeout Timeout spécifique pour ce nouvel état (0 pour utiliser le timeout par défaut)
 * @param reason Raison de la transition (pour journalisation)
 */
void StateMachine::transitionTo(int newState, unsigned long customTimeout, const char* reason) {
    // Éviter les transitions inutiles vers le même état
    if (newState == currentState) {
        return;
    }
    
    // Appel du handler de sortie pour l'état actuel
    onExitState(currentState, newState);
    
    // Journalisation de la transition
    if (reason) {
        LOG_INFO("FSM", "Transition dans '%s': %d -> %d (%s)",
                machineName, currentState, newState, reason);
    } else {
        LOG_INFO("FSM", "Transition dans '%s': %d -> %d",
                machineName, currentState, newState);
    }
    
    // Mise à jour de l'état
    previousState = currentState;
    currentState = newState;
    stateEntryTime = millis();
    
    // Mise à jour du timeout si spécifié
    if (customTimeout > 0) {
        stateTimeout = customTimeout;
        timeoutEnabled = true;
    }
    
    // Appel du handler d'entrée pour le nouvel état
    onEnterState(newState, previousState);
    
    // Marquer qu'une transition s'est produite
    transitionOccurred = true;
}

/**
 * Récupère l'état actuel
 * @return État actuel
 */
int StateMachine::getCurrentState() const {
    return currentState;
}

/**
 * Récupère l'état précédent
 * @return État précédent
 */
int StateMachine::getPreviousState() const {
    return previousState;
}

/**
 * Récupère le temps passé dans l'état actuel (en ms)
 * @return Durée en millisecondes depuis l'entrée dans l'état actuel
 */
unsigned long StateMachine::getTimeInCurrentState() const {
    return millis() - stateEntryTime;
}

/**
 * Active ou désactive le timeout
 * @param enabled true pour activer, false pour désactiver
 */
void StateMachine::enableTimeout(bool enabled) {
    timeoutEnabled = enabled;
}

/**
 * Définit l'état d'erreur par défaut
 * @param state État d'erreur
 */
void StateMachine::setErrorState(int state) {
    errorState = state;
}

/**
 * Définit l'état de timeout par défaut
 * @param state État de timeout
 */
void StateMachine::setTimeoutState(int state) {
    timeoutState = state;
}

/**
 * Indique si une transition s'est produite lors du dernier cycle
 * @return true si une transition a eu lieu
 */
bool StateMachine::didTransitionOccur() const {
    return transitionOccurred;
}

/**
 * Méthode appelée lors de l'entrée dans un état
 * Cette implémentation par défaut ne fait rien, à surcharger dans les classes dérivées
 * @param state État dans lequel on entre
 * @param fromState État précédent
 */
void StateMachine::onEnterState(int state, int fromState) {
    // Implémentation par défaut vide
}

/**
 * Méthode appelée lors de la sortie d'un état
 * Cette implémentation par défaut ne fait rien, à surcharger dans les classes dérivées
 * @param state État que l'on quitte
 * @param toState État vers lequel on va
 */
void StateMachine::onExitState(int state, int toState) {
    // Implémentation par défaut vide
}

/**
 * Méthode appelée quand un timeout se produit
 * Cette implémentation par défaut ne fait que journaliser, à surcharger dans les classes dérivées
 * @param state État dans lequel le timeout s'est produit
 */
void StateMachine::onTimeout(int state) {
    LOG_WARNING("FSM", "Timeout non géré dans l'état %d de la machine '%s'", state, machineName);
}