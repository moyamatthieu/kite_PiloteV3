#include "hal/drivers/winch_driver.h"
#include "core/logging.h"
#include <Arduino.h> // Pour pinMode, digitalWrite, analogWrite

// Constructeur
WinchDriver::WinchDriver(const char* name)
    : ActuatorComponent(name),
      _isInitialized(false),
      _currentSpeedSetting(0.0f),
      _currentMotorState(WinchCommand::STOP) {
    // Initialisation des membres si nécessaire, _driverConfig sera défini par configure()
}

// Destructeur
WinchDriver::~WinchDriver() {
    if (_isInitialized && getState() != ComponentState::COMPONENT_DISABLED) {
        stopMotor();
        // Autres nettoyages si nécessaire
    }
}

// Initialisation du pilote
ErrorCode WinchDriver::initialize() {
    if (_isInitialized) {
        LOG_WARNING(getName(), "Déjà initialisé.");
        return ErrorCode::ALREADY_INITIALIZED;
    }
    setState(ComponentState::INITIALIZING);
    LOG_INFO(getName(), "Initialisation...");

    // Vérifier si la configuration a été appliquée
    if (_driverConfig.motorPinA == 0xFF || _driverConfig.motorPinB == 0xFF) { // 0xFF comme indicateur de non configuré
        LOG_ERROR(getName(), "Configuration des broches du moteur non valide.");
        setState(ComponentState::ERROR);
        return ErrorCode::INVALID_PARAMETER;
    }

    // Configurer les broches du moteur
    pinMode(_driverConfig.motorPinA, OUTPUT);
    pinMode(_driverConfig.motorPinB, OUTPUT);
    digitalWrite(_driverConfig.motorPinA, LOW); // S'assurer que le moteur est arrêté initialement
    digitalWrite(_driverConfig.motorPinB, LOW);

    // Configurer la broche de contrôle de vitesse si utilisée
    if (_driverConfig.speedControlPin != 0xFF) {
        if (_driverConfig.speedControlPWM) {
            #ifdef ESP32
            // Configuration du canal PWM pour ESP32
            // ledcSetup(pwmChannel, freq, resolution);
            // ledcAttachPin(pin, pwmChannel);
            // Pour l'instant, nous supposons que la configuration PWM est gérée en dehors ou est simple
            pinMode(_driverConfig.speedControlPin, OUTPUT);
            analogWrite(_driverConfig.speedControlPin, 0); // Vitesse nulle
            LOG_INFO(getName(), "Broche de vitesse PWM configurée : %d", _driverConfig.speedControlPin);
            #else
            pinMode(_driverConfig.speedControlPin, OUTPUT);
            analogWrite(_driverConfig.speedControlPin, 0); // Vitesse nulle pour les Arduinos non-ESP32
            #endif
        } else {
            pinMode(_driverConfig.speedControlPin, OUTPUT);
            digitalWrite(_driverConfig.speedControlPin, LOW); // Ou HIGH selon la logique de contrôle de vitesse non-PWM
            LOG_INFO(getName(), "Broche de vitesse (non-PWM) configurée : %d", _driverConfig.speedControlPin);
        }
    }

    _isInitialized = true;
    setState(ComponentState::ACTIVE);
    LOG_INFO(getName(), "Initialisation terminée.");
    return ErrorCode::OK;
}

// Configuration du pilote
ErrorCode WinchDriver::configure(const WinchDriverConfig& config) {
    LOG_INFO(getName(), "Configuration du pilote de treuil...");
    _driverConfig = config;

    // Valider la configuration (exemple simple)
    if (_driverConfig.maxSpeed < _driverConfig.minSpeed) {
        LOG_ERROR(getName(), "Vitesse max < Vitesse min dans la configuration.");
        return ErrorCode::INVALID_PARAMETER;
    }
    
    // Si le composant est déjà initialisé, il pourrait avoir besoin d'être réinitialisé ou les paramètres appliqués dynamiquement
    if (_isInitialized) {
        LOG_INFO(getName(), "Reconfiguration en cours d'exécution. Arrêt du moteur par précaution.");
        stopMotor();
        // Réappliquer la configuration des broches si elles ont changé (nécessite une logique plus complexe)
        // Pour l'instant, nous supposons que l'initialisation gère la configuration des broches.
        // Il pourrait être nécessaire d'appeler à nouveau initialize() ou une partie de celui-ci.
    }
    
    LOG_INFO(getName(), "Configuration appliquée.");
    return ErrorCode::OK;
}

ErrorCode WinchDriver::reconfigure(const WinchDriverConfig& newConfig) {
    setState(ComponentState::INITIALIZING);
    ErrorCode ec = configure(newConfig);
    if (ec == ErrorCode::OK) {
        // Si le pilote était opérationnel, il faut peut-être le réinitialiser ou le redémarrer
        if (getState() == ComponentState::ACTIVE) {
            LOG_INFO(getName(), "Redémarrage après reconfiguration.");
            onDisable(); // Arrête le moteur
            // Il n'est pas nécessaire de réinitialiser complètement si seules les limites de vitesse ont changé
            // Mais si les broches ont changé, une réinitialisation est nécessaire.
            // Pour cet exemple, nous supposons que les broches ne changent pas via reconfigure.
            onEnable(); // Prêt à fonctionner avec la nouvelle configuration
        }
         setState(ComponentState::ACTIVE); // Retour à l'état initialisé mais pas encore activé
    } else {
        setState(ComponentState::ERROR);
    }
    return ec;
}

// Envoyer une commande au treuil
ErrorCode WinchDriver::sendCommand(WinchCommand command) {
    if (getState() != ComponentState::ACTIVE) {
        LOG_WARNING(getName(), "Impossible d'envoyer la commande, le pilote n'est pas activé.");
        return ErrorCode::RESOURCE_ERROR;
    }

    _currentMotorState = command;
    switch (command) {
        case WinchCommand::STOP:
            LOG_DEBUG(getName(), "Commande : STOP");
            return stopMotor();
        case WinchCommand::WIND_IN:
            LOG_DEBUG(getName(), "Commande : WIND_IN (Vitesse : %.2f%%)", _currentSpeedSetting);
            // Appliquer la vitesse actuelle pour enrouler
            return applySpeed(_currentSpeedSetting); // Vitesse positive pour enrouler
        case WinchCommand::WIND_OUT:
            LOG_DEBUG(getName(), "Commande : WIND_OUT (Vitesse : %.2f%%)", _currentSpeedSetting);
            // Appliquer la vitesse actuelle pour dérouler
            return applySpeed(-_currentSpeedSetting); // Vitesse négative pour dérouler
        case WinchCommand::SET_SPEED: // Cette commande est implicitement gérée par setSpeed
            LOG_DEBUG(getName(), "Commande : SET_SPEED (utilisez setSpeed() directement)");
            return ErrorCode::INVALID_PARAMETER; // Ou simplement ne rien faire
        case WinchCommand::HOLD:
            LOG_DEBUG(getName(), "Commande : HOLD");
            // Pourrait impliquer un frein actif ou simplement arrêter le moteur
            return stopMotor(); // Simplification : HOLD = STOP
        default:
            LOG_WARNING(getName(), "Commande de treuil inconnue : %d", static_cast<int>(command));
            return ErrorCode::INVALID_PARAMETER;
    }
    return ErrorCode::OK;
}

// Définir la vitesse du treuil
ErrorCode WinchDriver::setSpeed(float speedPercentage) {
    if (getState() != ComponentState::ACTIVE) {
        LOG_WARNING(getName(), "Impossible de définir la vitesse, le pilote n'est pas dans un état approprié.");
        return ErrorCode::RESOURCE_ERROR;
    }

    // Contraindre la vitesse aux limites configurées
    if (speedPercentage > _driverConfig.maxSpeed) speedPercentage = _driverConfig.maxSpeed;
    if (speedPercentage < -_driverConfig.maxSpeed) speedPercentage = -_driverConfig.maxSpeed; // Permettre la vitesse négative pour le déroulement
    
    // Gérer la zone morte ou la vitesse minimale si nécessaire
    // if (abs(speedPercentage) < _driverConfig.minSpeed && speedPercentage != 0.0f) {
    //     speedPercentage = (speedPercentage > 0) ? _driverConfig.minSpeed : -_driverConfig.minSpeed;
    // }

    _currentSpeedSetting = speedPercentage;
    LOG_DEBUG(getName(), "Vitesse définie à : %.2f%%", _currentSpeedSetting);

    // Si le moteur est déjà en mouvement, appliquer la nouvelle vitesse
    if (_currentMotorState == WinchCommand::WIND_IN) {
        return applySpeed(_currentSpeedSetting);
    } else if (_currentMotorState == WinchCommand::WIND_OUT) {
        return applySpeed(-_currentSpeedSetting);
    }
    // Si le moteur est à l'arrêt, la vitesse sera appliquée lors de la prochaine commande WIND_IN/OUT

    return ErrorCode::OK;
}

// Arrêter le moteur du treuil
ErrorCode WinchDriver::stopMotor() {
    LOG_DEBUG(getName(), "Arrêt du moteur.");
    digitalWrite(_driverConfig.motorPinA, LOW);
    digitalWrite(_driverConfig.motorPinB, LOW);
    if (_driverConfig.speedControlPin != 0xFF && _driverConfig.speedControlPWM) {
        #ifdef ESP32
        // ledcWrite(_driverConfig.pwmChannel, 0);
        analogWrite(_driverConfig.speedControlPin, 0); // Simplifié
        #else
        analogWrite(_driverConfig.speedControlPin, 0);
        #endif
    } else if (_driverConfig.speedControlPin != 0xFF && !_driverConfig.speedControlPWM) {
        digitalWrite(_driverConfig.speedControlPin, LOW); // Ou HIGH selon la logique
    }
    _currentMotorState = WinchCommand::STOP;
    // _currentSpeedSetting = 0; // Optionnel : réinitialiser la vitesse de consigne à l'arrêt
    return ErrorCode::OK;
}

// Obtenir la vitesse actuelle (consigne)
float WinchDriver::getCurrentSpeed() const {
    if (_currentMotorState == WinchCommand::STOP) return 0.0f;
    if (_currentMotorState == WinchCommand::WIND_OUT) return -_currentSpeedSetting;
    return _currentSpeedSetting;
}

// Appelé lorsque le composant est activé
void WinchDriver::onEnable() {
    if (!_isInitialized) {
        LOG_ERROR(getName(), "Impossible d'activer, non initialisé.");
        setState(ComponentState::ERROR);
        return;
    }
    LOG_INFO(getName(), "Activation.");
    // Actions à effectuer à l'activation, par exemple, s'assurer que le moteur est prêt
    // Pourrait effectuer un auto-test ou vérifier les connexions
    stopMotor(); // S'assurer que le moteur est arrêté lors de l'activation
    setState(ComponentState::ACTIVE);
}

// Appelé lorsque le composant est désactivé
void WinchDriver::onDisable() {
    LOG_INFO(getName(), "Désactivation.");
    stopMotor(); // Action principale lors de la désactivation
    setState(ComponentState::COMPONENT_DISABLED);
}

// Fonction interne pour appliquer la vitesse au matériel
ErrorCode WinchDriver::applySpeed(float speedPercentage) {
    if (getState() != ComponentState::ACTIVE) {
        LOG_WARNING(getName(), "Impossible d'appliquer la vitesse, le pilote n'est pas activé.");
        return ErrorCode::RESOURCE_ERROR;
    }

    // Convertir le pourcentage de vitesse en valeur PWM ou autre contrôle
    // S'assurer que la vitesse est dans les limites de -100 à 100
    speedPercentage = constrain(speedPercentage, -_driverConfig.maxSpeed, _driverConfig.maxSpeed);

    int pwmValue = 0;
    if (_driverConfig.speedControlPWM && _driverConfig.speedControlPin != 0xFF) {
        // Supposons une plage PWM de 0-255 pour Arduino standard
        // Pour ESP32, ledcWrite prend une valeur de cycle de service basée sur la résolution configurée
        // Exemple : si résolution de 8 bits (0-255), alors c'est direct.
        // Si résolution de 10 bits (0-1023), alors multiplier par (1023/100.0)
        #ifdef ESP32
        // int dutyCycle = map(abs(speedPercentage), 0, 100, 0, pow(2, LEDC_TIMER_BIT_WIDTH) - 1);
        // Pour la simplicité, utilisons analogWrite qui mappe à la résolution par défaut pour ESP32 si non configuré via ledc
        pwmValue = map(abs(speedPercentage), 0, 100, 0, 255); // Ou une valeur max PWM appropriée
        analogWrite(_driverConfig.speedControlPin, pwmValue);
        #else
        pwmValue = map(abs(speedPercentage), 0, 100, 0, 255);
        analogWrite(_driverConfig.speedControlPin, pwmValue);
        #endif
        LOG_DEBUG(getName(), "Application PWM : %d (pour vitesse %.2f%%)", pwmValue, speedPercentage);
    } else if (!_driverConfig.speedControlPWM && _driverConfig.speedControlPin != 0xFF) {
        // Logique pour le contrôle de vitesse non-PWM (par exemple, relais, niveaux logiques discrets)
        // Ceci est un exemple et dépend fortement du matériel
        if (abs(speedPercentage) > 0) {
            digitalWrite(_driverConfig.speedControlPin, HIGH); // Activer le mouvement
        } else {
            digitalWrite(_driverConfig.speedControlPin, LOW); // Désactiver le mouvement
        }
    }
    // Si pas de speedControlPin, la vitesse est uniquement contrôlée par la direction (ON/OFF)

    // Définir la direction du moteur
    if (speedPercentage > 0) { // Enrouler (Forward)
        digitalWrite(_driverConfig.motorPinA, HIGH);
        digitalWrite(_driverConfig.motorPinB, LOW);
        _currentMotorState = WinchCommand::WIND_IN;
        LOG_DEBUG(getName(), "Moteur : Enroulement (PinA:HIGH, PinB:LOW)");
    } else if (speedPercentage < 0) { // Dérouler (Backward)
        digitalWrite(_driverConfig.motorPinA, LOW);
        digitalWrite(_driverConfig.motorPinB, HIGH);
        _currentMotorState = WinchCommand::WIND_OUT;
        LOG_DEBUG(getName(), "Moteur : Déroulement (PinA:LOW, PinB:HIGH)");
    } else { // Stop
        digitalWrite(_driverConfig.motorPinA, LOW);
        digitalWrite(_driverConfig.motorPinB, LOW);
        _currentMotorState = WinchCommand::STOP;
        LOG_DEBUG(getName(), "Moteur : Arrêt (PinA:LOW, PinB:LOW)");
    }

    return ErrorCode::OK;
}
