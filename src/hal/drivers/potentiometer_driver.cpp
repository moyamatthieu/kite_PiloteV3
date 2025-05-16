#include "hal/drivers/potentiometer_driver.h"
#include "core/logging.h"
#include <Arduino.h> // Pour analogRead, map, pinMode (si nécessaire pour la configuration ADC)

// bool PotentiometerDriver::_adcInitialized = false; // Décommenter si une initialisation ADC globale est nécessaire

// Constructeur
PotentiometerDriver::PotentiometerDriver(const char* name)
    : InputComponent(name), // Correction : héritage correct
      _numConfiguredPots(0),
      _isInitialized(false) {
    for (int i = 0; i < MAX_POTENTIOMETERS; ++i) {
        _potConfigs[i].pin = 0xFF; // Marquer comme non configuré
        _potConfigs[i].logicalType = PotentiometerType::NONE;
    }
}

// Destructeur
PotentiometerDriver::~PotentiometerDriver() {
    // Aucun nettoyage spécifique requis pour les lectures ADC simples.
}

// Initialisation du pilote
ErrorCode PotentiometerDriver::initialize() {
    if (_isInitialized) {
        LOG_WARNING(getName(), "Déjà initialisé.");
        return ErrorCode::ALREADY_INITIALIZED;
    }
    setState(ComponentState::INITIALIZING);
    LOG_INFO(getName(), "Initialisation...");

    // Configuration ADC globale (si nécessaire et non déjà faite)
    // Par exemple, pour ESP32, configurer la résolution et l'atténuation ADC
    // if (!_adcInitialized) {
    // #ifdef ESP32
    //     analogReadResolution(ADC_RESOLUTION); // Défini dans config.h, par ex. 12 bits
    //     // adcAttachPin(pin) et analogSetPinAttenuation(pin, ADC_ATTENUATION) sont faits par potentiomètre
    // #endif
    //     _adcInitialized = true;
    //     LOG_INFO(getName(), "Configuration ADC globale appliquée.");
    // }

    // Initialiser les valeurs lissées pour chaque potentiomètre configuré
    for (int i = 0; i < _numConfiguredPots; ++i) {
        if (_potConfigs[i].pin != 0xFF) {
            // Lire la valeur initiale pour démarrer le lissage correctement
            _potConfigs[i].lastRawValue = analogRead(_potConfigs[i].pin);
            _potConfigs[i].smoothedValue = map(_potConfigs[i].lastRawValue,
                                               _potConfigs[i].rawMin, _potConfigs[i].rawMax,
                                               _potConfigs[i].outputMin, _potConfigs[i].outputMax);
            LOG_DEBUG(getName(), "Pot %d (broche %d) valeur initiale brute: %d, lissée: %.2f", 
                static_cast<int>(_potConfigs[i].logicalType), _potConfigs[i].pin, 
                _potConfigs[i].lastRawValue, _potConfigs[i].smoothedValue);
        }
    }

    _isInitialized = true;
    setState(ComponentState::IDLE);
    LOG_INFO(getName(), "Initialisation terminée.");
    return ErrorCode::OK;
}

// Configurer un potentiomètre individuel
ErrorCode PotentiometerDriver::configurePotentiometer(PotentiometerType type, uint8_t pin, int rawMin, int rawMax, float outMin, float outMax, float smoothing) {
    if (type == PotentiometerType::NONE) {
        LOG_ERROR(getName(), "Type de potentiomètre non valide pour la configuration.");
        return ErrorCode::INVALID_PARAMETER;
    }

    int potIndex = -1;
    for (int i = 0; i < MAX_POTENTIOMETERS; ++i) {
        if (_potConfigs[i].logicalType == type) { // Potentiomètre déjà configuré, le mettre à jour
            potIndex = i;
            break;
        }
        if (_potConfigs[i].pin == 0xFF && potIndex == -1) { // Emplacement vide trouvé
            potIndex = i;
        }
    }

    if (potIndex == -1 && _numConfiguredPots >= MAX_POTENTIOMETERS) {
        LOG_ERROR(getName(), "Impossible de configurer le potentiomètre %d : nombre maximum atteint.", static_cast<int>(type));
        return ErrorCode::RESOURCE_ERROR;
    }
    if(potIndex == -1) { // Devrait être trouvé si pas plein
         potIndex = _numConfiguredPots; // Prendre le prochain disponible
    }

    PotentiometerConfig& cfg = _potConfigs[potIndex];
    cfg.pin = pin;
    cfg.logicalType = type;
    cfg.rawMin = rawMin;
    cfg.rawMax = rawMax;
    cfg.outputMin = outMin;
    cfg.outputMax = outMax;
    cfg.smoothingFactor = constrain(smoothing, 0.0f, 0.99f); // 0.99f pour éviter que la valeur ne soit jamais mise à jour si 1.0f

    // Configuration spécifique à la broche ADC (par exemple, pour ESP32)
#ifdef ESP32
    // analogSetPinAttenuation(cfg.pin, ADC_ATTENUATION); // ADC_ATTENUATION défini dans config.h, par ex. ADC_11db
    // adcAttachPin(cfg.pin); // Peut être nécessaire pour certaines configurations ESP32
    pinMode(cfg.pin, INPUT); // Assurer que la broche est en mode INPUT pour la lecture analogique
#else
    pinMode(cfg.pin, INPUT); // Pour les Arduinos standards, pinMode INPUT est suffisant pour analogRead
#endif

    // Lire la valeur initiale pour le lissage
    cfg.lastRawValue = analogRead(cfg.pin);
    cfg.smoothedValue = map(cfg.lastRawValue, cfg.rawMin, cfg.rawMax, cfg.outputMin, cfg.outputMax);

    LOG_INFO(getName(), "Potentiomètre %d configuré sur la broche ADC %d. Plage brute [%d-%d], sortie [%.2f-%.2f], lissage: %.2f",
             static_cast<int>(type), pin, rawMin, rawMax, outMin, outMax, smoothing);

    if (potIndex == _numConfiguredPots) { // Si c'est un nouveau potentiomètre et pas une mise à jour
      _numConfiguredPots++;
    }
    
    // Si déjà initialisé, la nouvelle configuration prendra effet lors du prochain appel à update()
    return ErrorCode::OK;
}

// Lire la valeur mappée et lissée
float PotentiometerDriver::getValue(PotentiometerType type) {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        return 0.0f; // Ou une valeur d'erreur/NaN
    }
    for (int i = 0; i < _numConfiguredPots; ++i) {
        if (_potConfigs[i].logicalType == type) {
            // L'appel à update() est supposé être fait régulièrement ailleurs
            // Si un lissage en temps réel est nécessaire ici sans appel explicite à update(), il faudrait le faire ici.
            // Pour l'instant, on retourne la dernière valeur lissée calculée par update().
            return _potConfigs[i].smoothedValue;
        }
    }
    LOG_DEBUG(getName(), "Demande de valeur pour un potentiomètre non configuré : %d", static_cast<int>(type));
    return 0.0f; // Ou une valeur d'erreur/NaN
}

// Lire la valeur brute de l'ADC
int PotentiometerDriver::getRawValue(PotentiometerType type) {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        return 0; // Ou une valeur d'erreur
    }
    for (int i = 0; i < _numConfiguredPots; ++i) {
        if (_potConfigs[i].logicalType == type) {
            // Retourne la dernière valeur brute lue, mise à jour par update()
            return _potConfigs[i].lastRawValue;
        }
    }
    LOG_DEBUG(getName(), "Demande de valeur brute pour un potentiomètre non configuré : %d", static_cast<int>(type));
    return 0; // Ou une valeur d'erreur
}

// Mettre à jour les lectures (doit être appelé régulièrement)
void PotentiometerDriver::update() {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        return;
    }
    if(getState() == ComponentState::IDLE) {
        setState(ComponentState::ACTIVE);
    }

    for (int i = 0; i < _numConfiguredPots; ++i) {
        if (_potConfigs[i].pin != 0xFF) {
            int rawValue = analogRead(_potConfigs[i].pin);
            _potConfigs[i].lastRawValue = rawValue;

            // Appliquer le lissage
            // Vt = (1-alpha)*Vt-1 + alpha*St
            // alpha est smoothingFactor
            float mappedValue = map(rawValue, 
                                    _potConfigs[i].rawMin, _potConfigs[i].rawMax, 
                                    _potConfigs[i].outputMin, _potConfigs[i].outputMax);
            
            _potConfigs[i].smoothedValue = (1.0f - _potConfigs[i].smoothingFactor) * _potConfigs[i].smoothedValue 
                                         + _potConfigs[i].smoothingFactor * mappedValue;
            
            // LOG_DEBUG(getName(), "Pot %d (pin %d) raw: %d, mapped: %.2f, smoothed: %.2f", 
            //     static_cast<int>(_potConfigs[i].logicalType), _potConfigs[i].pin, 
            //     rawValue, mappedValue, _potConfigs[i].smoothedValue);
        }
    }
}

// Vérifier si les données sont disponibles pour un potentiomètre spécifique
bool PotentiometerDriver::dataAvailable(PotentiometerType type) {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        return false;
    }
    for (int i = 0; i < _numConfiguredPots; ++i) {
        if (_potConfigs[i].logicalType == type && _potConfigs[i].pin != 0xFF) {
            // On considère les données disponibles si le potentiomètre est configuré et le driver actif.
            // La validité de la lecture elle-même (ex: ADC non bruité) n'est pas vérifiée ici.
            return true; 
        }
    }
    return false;
}

// Appelé lorsque le composant est activé
void PotentiometerDriver::onEnable() {
    LOG_INFO(getName(), "%s activé.", getName());
    if (!_isInitialized) {
        initialize();
    } else {
         // S'assurer que les valeurs initiales sont lues si on réactive
        for (int i = 0; i < _numConfiguredPots; ++i) {
            if (_potConfigs[i].pin != 0xFF) {
                _potConfigs[i].lastRawValue = analogRead(_potConfigs[i].pin);
                 _potConfigs[i].smoothedValue = map(_potConfigs[i].lastRawValue,
                                               _potConfigs[i].rawMin, _potConfigs[i].rawMax,
                                               _potConfigs[i].outputMin, _potConfigs[i].outputMax);
            }
        }
        setState(ComponentState::IDLE); // Prêt à fonctionner
    }
}

// Appelé lorsque le composant est désactivé
void PotentiometerDriver::onDisable() {
    LOG_INFO(getName(), "%s désactivé.", getName());
    setState(ComponentState::COMPONENT_DISABLED);
    // Pas d'action spécifique sur les broches ADC lors de la désactivation pour l'instant.
}
