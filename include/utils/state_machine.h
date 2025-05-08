#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

/**
 * Classe de base pour l'implémentation standardisée de machines à états finis (FSM)
 * Cette classe fournit un cadre commun pour toutes les FSM du projet
 * en garantissant la gestion des timeouts et une approche non-bloquante
 */
class StateMachine {
protected:
    int currentState;              // État actuel de la FSM
    int previousState;             // État précédent de la FSM
    unsigned long stateEntryTime;  // Moment d'entrée dans l'état actuel
    unsigned long stateTimeout;    // Durée maximum autorisée dans l'état actuel
    bool timeoutEnabled;           // Indique si le timeout est activé
    
    // Nom de la FSM (pour journalisation)
    const char* machineName;
    
    // État par défaut en cas d'erreur ou de timeout
    int errorState;
    int timeoutState;
    
    // Indique si une transition d'état vient de se produire dans ce cycle
    bool transitionOccurred;

public:
    /**
     * Constructeur de machine à états
     * @param name Nom de la machine à états (pour journalisation)
     * @param initialState État initial
     * @param defaultTimeout Timeout par défaut en millisecondes (0 = pas de timeout)
     * @param defaultErrorState État d'erreur par défaut
     */
    StateMachine(const char* name, int initialState, unsigned long defaultTimeout = 0, int defaultErrorState = -1);
    
    /**
     * Destructeur virtuel
     */
    virtual ~StateMachine();
    
    /**
     * Mise à jour de la machine à états
     * Cette méthode doit être appelée régulièrement pour faire avancer la FSM
     * Elle gère le timeout et appelle les handlers d'état appropriés
     * @return État actuel après la mise à jour
     */
    virtual int update();
    
    /**
     * Vérifie si un timeout s'est produit
     * @return true si le temps passé dans l'état actuel dépasse le timeout configuré
     */
    bool hasTimedOut() const;
    
    /**
     * Effectue une transition vers un nouvel état
     * @param newState Nouvel état
     * @param customTimeout Timeout spécifique pour ce nouvel état (0 pour utiliser le timeout par défaut)
     * @param reason Raison de la transition (pour journalisation)
     */
    void transitionTo(int newState, unsigned long customTimeout = 0, const char* reason = nullptr);
    
    /**
     * Récupère l'état actuel
     * @return État actuel
     */
    int getCurrentState() const;
    
    /**
     * Récupère l'état précédent
     * @return État précédent
     */
    int getPreviousState() const;
    
    /**
     * Récupère le temps passé dans l'état actuel (en ms)
     * @return Durée en millisecondes depuis l'entrée dans l'état actuel
     */
    unsigned long getTimeInCurrentState() const;
    
    /**
     * Active ou désactive le timeout
     * @param enabled true pour activer, false pour désactiver
     */
    void enableTimeout(bool enabled);
    
    /**
     * Définit l'état d'erreur par défaut
     * @param state État d'erreur
     */
    void setErrorState(int state);
    
    /**
     * Définit l'état de timeout par défaut
     * @param state État de timeout
     */
    void setTimeoutState(int state);
    
    /**
     * Indique si une transition s'est produite lors du dernier cycle
     * @return true si une transition a eu lieu
     */
    bool didTransitionOccur() const;

protected:
    /**
     * Méthode appelée lors de l'entrée dans un état
     * @param state État dans lequel on entre
     * @param fromState État précédent
     */
    virtual void onEnterState(int state, int fromState);
    
    /**
     * Méthode appelée lors de la sortie d'un état
     * @param state État que l'on quitte
     * @param toState État vers lequel on va
     */
    virtual void onExitState(int state, int toState);
    
    /**
     * Méthode appelée quand un timeout se produit
     * @param state État dans lequel le timeout s'est produit
     */
    virtual void onTimeout(int state);
    
    /**
     * Méthode de traitement pour chaque état
     * @param state État à traiter
     * @return État après traitement (peut être différent si une transition s'est produite)
     */
    virtual int processState(int state) = 0;
};

#endif // STATE_MACHINE_H