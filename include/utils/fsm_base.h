#ifndef FSM_BASE_H
#define FSM_BASE_H

#include <Arduino.h>

/**
 * Classe de base pour les machines à états finis (FSM).
 */
class FSMBase {
protected:
    int currentState;
    unsigned long stateEntryTime;
    unsigned long stateTimeout;

public:
    FSMBase(int initialState, unsigned long defaultTimeout)
        : currentState(initialState), stateEntryTime(millis()), stateTimeout(defaultTimeout) {}

    virtual void update() = 0; // Méthode virtuelle pure pour la mise à jour de la FSM

    bool hasTimedOut() const {
        return (millis() - stateEntryTime) >= stateTimeout;
    }

    void transitionTo(int newState) {
        currentState = newState;
        stateEntryTime = millis();
    }

    int getCurrentState() const {
        return currentState;
    }
};

#endif // FSM_BASE_H