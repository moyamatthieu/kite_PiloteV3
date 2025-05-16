#pragma once

#include "hal/hal_component.h" // Modifié pour inclure le bon fichier d'en-tête
#include "common/global_enums.h"
#include "core/config.h" // Pour les broches des boutons, etc.
#include <functional> // Pour std::function

#define MAX_BUTTONS 5 // Nombre maximum de boutons gérés par ce pilote

// Énumération pour identifier les boutons logiques
enum class ButtonType : uint8_t {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    SELECT = 4,
    // Ajoutez d'autres types de boutons si nécessaire
    NONE = 0xFF
};

// États d'un bouton
enum class ButtonState : uint8_t {
    RELEASED = 0,
    PRESSED = 1,
    CLICKED = 2,      // Appui court
    LONG_PRESSED = 3  // Appui long détecté
};

// Structure pour la configuration d'un bouton individuel
struct ButtonConfig {
    uint8_t pin;
    bool activeLow; // True si le bouton est actif à l'état bas (pull-up externe/interne)
    uint16_t debounceDelayMs;
    uint16_t longPressDelayMs;
    ButtonType logicalType;

    // État interne
    volatile bool currentStateRough; // État brut lu depuis la broche
    volatile bool debouncedState;
    volatile unsigned long lastDebounceTime;
    volatile unsigned long pressStartTime;
    volatile ButtonState reportedState;

    ButtonConfig() :
        pin(0xFF), activeLow(true), debounceDelayMs(BUTTON_DEFAULT_DEBOUNCE_MS),
        longPressDelayMs(BUTTON_DEFAULT_LONG_PRESS_MS), logicalType(ButtonType::NONE),
        currentStateRough(false), debouncedState(false), lastDebounceTime(0),
        pressStartTime(0), reportedState(ButtonState::RELEASED) {}
};

// Type de fonction pour les callbacks d'événements de bouton
// Le ButtonType indique quel bouton a déclenché l'événement, ButtonState l'événement spécifique
using ButtonCallback = std::function<void(ButtonType, ButtonState)>;

class ButtonsDriver : public InputComponent { // Hérite de InputComponent qui hérite de HALComponent
public:
    ButtonsDriver(const char* name = "ButtonsDriver");
    ~ButtonsDriver() override;

    ErrorCode initialize() override;
    // Configure les boutons individuellement ou tous ensemble
    ErrorCode configureButton(ButtonType type, uint8_t pin, bool activeLow = true, uint16_t debounceMs = BUTTON_DEFAULT_DEBOUNCE_MS, uint16_t longPressMs = BUTTON_DEFAULT_LONG_PRESS_MS);
    
    // Interroge l'état d'un bouton (non bloquant)
    ButtonState getState(ButtonType type);
    ButtonState getButtonState(ButtonType type);
    bool isPressed(ButtonType type); // Raccourci pour getState() == PRESSED || getState() == LONG_PRESSED

    // Gestion des callbacks (optionnel, si on veut un système événementiel)
    void registerCallback(ButtonCallback callback);

    // Doit être appelé régulièrement pour mettre à jour l'état des boutons (par exemple, depuis une tâche ou la boucle principale)
    void update(); 

protected:
    void onEnable() override;
    void onDisable() override;
    // void run() override; // Si ce composant a sa propre tâche pour la scrutation

private:
    void processButton(ButtonConfig& button); // Logique de détection d'appui, relâchement, appui long

    ButtonConfig _buttonConfigs[MAX_BUTTONS];
    uint8_t _numConfiguredButtons;
    ButtonCallback _eventCallback;
    bool _isInitialized;
};
