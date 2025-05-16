#include "hal/drivers/buttons_driver.h"
#include "core/logging.h"
#include <Arduino.h> // Pour digitalRead, millis, pinMode

// Constructeur
ButtonsDriver::ButtonsDriver(const char* name)
    : InputComponent(name), // Hérite d'InputComponent qui hérite de HALComponent
      _numConfiguredButtons(0),
      _eventCallback(nullptr),
      _isInitialized(false) {
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        _buttonConfigs[i].pin = 0xFF; // Marquer comme non configuré
        _buttonConfigs[i].logicalType = ButtonType::NONE;
    }
}

// Destructeur
ButtonsDriver::~ButtonsDriver() {
    // Aucun nettoyage spécifique requis pour les broches GPIO simples
    // si ce n'est de les remettre dans un état par défaut si nécessaire.
}

// Initialisation du pilote
ErrorCode ButtonsDriver::initialize() {
    if (_isInitialized) {
        LOG_WARNING(getComponentName(), "Déjà initialisé.");
        return ErrorCode::ALREADY_INITIALIZED;
    }
    setState(ComponentState::INITIALIZING);
    LOG_INFO(getComponentName(), "Initialisation...");

    // Les broches sont configurées dans configureButton
    // S'assurer qu'au moins un bouton est configuré peut être une vérification ici si nécessaire.
    if (_numConfiguredButtons == 0) {
        LOG_INFO(getComponentName(), "Aucun bouton configuré pour l'initialisation.");
    }

    _isInitialized = true;
    setState(ComponentState::IDLE);
    LOG_INFO(getComponentName(), "Initialisation terminée.");
    return ErrorCode::OK;
}

// Configurer un bouton individuel
ErrorCode ButtonsDriver::configureButton(ButtonType type, uint8_t pin, bool activeLow, uint16_t debounceMs, uint16_t longPressMs) {
    if (type == ButtonType::NONE) {
        LOG_ERROR(getComponentName(), "Type de bouton non valide pour la configuration.");
        return ErrorCode::INVALID_PARAMETER;
    }

    // Trouver un emplacement de configuration ou le bouton existant
    int buttonIndex = -1;
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        if (_buttonConfigs[i].logicalType == type) { // Bouton déjà configuré, le mettre à jour
            buttonIndex = i;
            break;
        }
        if (_buttonConfigs[i].pin == 0xFF && buttonIndex == -1) { // Emplacement vide trouvé
            buttonIndex = i;
        }
    }

    if (buttonIndex == -1 && _numConfiguredButtons >= MAX_BUTTONS) {
        LOG_ERROR(getComponentName(), "Impossible de configurer le bouton %d : nombre maximum de boutons atteint.", static_cast<int>(type));
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    if(buttonIndex == -1) { // Devrait être trouvé si pas plein
         buttonIndex = _numConfiguredButtons; // Prendre le prochain disponible
    }

    ButtonConfig& cfg = _buttonConfigs[buttonIndex];
    cfg.pin = pin;
    cfg.activeLow = activeLow;
    cfg.debounceDelayMs = debounceMs;
    cfg.longPressDelayMs = longPressMs;
    cfg.logicalType = type;
    cfg.currentStateRough = cfg.activeLow ? digitalRead(cfg.pin) : !digitalRead(cfg.pin); // Lire l'état initial
    cfg.debouncedState = cfg.currentStateRough;
    cfg.lastDebounceTime = 0;
    cfg.pressStartTime = 0;
    cfg.reportedState = ButtonState::RELEASED;

    pinMode(cfg.pin, cfg.activeLow ? INPUT_PULLUP : INPUT);
    LOG_INFO(getComponentName(), "Bouton %d configuré sur la broche %d (ActiveLow: %s)", static_cast<int>(type), pin, activeLow ? "true" : "false");

    if (buttonIndex == _numConfiguredButtons) { // Si c'est un nouveau bouton et pas une mise à jour
      _numConfiguredButtons++;
    }
    
    return ErrorCode::OK;
}

// Obtenir l'état rapporté d'un bouton
ButtonState ButtonsDriver::getButtonState(ButtonType type) {
    if (ManagedComponent::getState() != ComponentState::ACTIVE && ManagedComponent::getState() != ComponentState::IDLE) {
        return ButtonState::RELEASED; // Ou un état d'erreur
    }
    for (int i = 0; i < _numConfiguredButtons; ++i) {
        if (_buttonConfigs[i].logicalType == type) {
            return _buttonConfigs[i].reportedState;
        }
    }
    LOG_DEBUG(getComponentName(), "Demande d'état pour un bouton non configuré : %d", static_cast<int>(type));
    return ButtonState::RELEASED; // Ou un état indiquant non configuré
}

// Raccourci pour vérifier si un bouton est pressé
bool ButtonsDriver::isPressed(ButtonType type) {
    ButtonState state = getButtonState(type);
    return (state == ButtonState::PRESSED || state == ButtonState::LONG_PRESSED);
}

// Enregistrer un callback pour les événements de bouton
void ButtonsDriver::registerCallback(ButtonCallback callback) {
    _eventCallback = callback;
    LOG_INFO(getComponentName(), "Callback d'événement de bouton enregistré.");
}

// Mettre à jour l'état de tous les boutons (doit être appelé régulièrement)
void ButtonsDriver::update() {
    if (ManagedComponent::getState() != ComponentState::ACTIVE) {
        return; // Ne rien faire si le pilote n'est pas activé
    }

    unsigned long currentTime = millis();
    for (int i = 0; i < _numConfiguredButtons; ++i) {
        ButtonConfig& button = _buttonConfigs[i];
        if (button.pin == 0xFF) continue; // Non configuré

        bool reading = digitalRead(button.pin);
        if (button.activeLow) {
            reading = !reading; // Inverser si actif à l'état bas (pressé = true)
        }

        // Anti-rebond
        if (reading != button.currentStateRough) {
            button.lastDebounceTime = currentTime;
            button.currentStateRough = reading;
        }

        if ((currentTime - button.lastDebounceTime) > button.debounceDelayMs) {
            // L'état a été stable plus longtemps que le délai d'anti-rebond
            if (reading != button.debouncedState) {
                button.debouncedState = reading;
                ButtonState previousReportedState = button.reportedState;

                if (button.debouncedState) { // Le bouton est pressé (après anti-rebond)
                    button.pressStartTime = currentTime;
                    button.reportedState = ButtonState::PRESSED;
                    if (_eventCallback && previousReportedState == ButtonState::RELEASED) {
                        _eventCallback(button.logicalType, ButtonState::PRESSED);
                    }
                } else { // Le bouton est relâché (après anti-rebond)
                    if (button.reportedState == ButtonState::LONG_PRESSED) {
                        button.reportedState = ButtonState::RELEASED;
                        if (_eventCallback) {
                            _eventCallback(button.logicalType, ButtonState::RELEASED); // Événement de relâchement après un appui long
                        }
                    } else if (button.reportedState == ButtonState::PRESSED) {
                        button.reportedState = ButtonState::CLICKED; // D'abord signalé comme cliqué
                        if (_eventCallback) {
                            _eventCallback(button.logicalType, ButtonState::CLICKED);
                        }
                        // Après avoir signalé CLICKED, il revient immédiatement à RELEASED
                        // car l'état physique est relâché.
                        button.reportedState = ButtonState::RELEASED;
                        if (_eventCallback) {
                             _eventCallback(button.logicalType, ButtonState::RELEASED); // Puis événement de relâchement
                        }
                    }
                    button.pressStartTime = 0; // Réinitialiser le temps de pression
                }
            }
        }

        // Vérifier l'appui long si le bouton est actuellement maintenu enfoncé
        if (button.debouncedState && button.reportedState == ButtonState::PRESSED) {
            if ((currentTime - button.pressStartTime) > button.longPressDelayMs) {
                button.reportedState = ButtonState::LONG_PRESSED;
                if (_eventCallback) {
                    _eventCallback(button.logicalType, ButtonState::LONG_PRESSED);
                }
            }
        }
    }
}

// Appelé lorsque le composant est activé
void ButtonsDriver::onEnable() {
    if (!_isInitialized) {
        LOG_ERROR(getComponentName(), "Impossible d'activer, non initialisé.");
        setState(ComponentState::ERROR);
        return;
    }
    // Réinitialiser l'état des boutons lors de l'activation
    for (int i = 0; i < _numConfiguredButtons; ++i) {
        if (_buttonConfigs[i].pin != 0xFF) {
            _buttonConfigs[i].currentStateRough = _buttonConfigs[i].activeLow ? digitalRead(_buttonConfigs[i].pin) : !digitalRead(_buttonConfigs[i].pin);
            _buttonConfigs[i].debouncedState = _buttonConfigs[i].currentStateRough;
            _buttonConfigs[i].reportedState = ButtonState::RELEASED;
            _buttonConfigs[i].lastDebounceTime = 0;
            _buttonConfigs[i].pressStartTime = 0;
        }
    }
    setState(ComponentState::ACTIVE);
    LOG_INFO(getComponentName(), "Activé. Appelez update() régulièrement.");
}

// Appelé lorsque le composant est désactivé
void ButtonsDriver::onDisable() {
    setState(ComponentState::COMPONENT_DISABLED);
    LOG_INFO(getComponentName(), "Désactivé.");
}
