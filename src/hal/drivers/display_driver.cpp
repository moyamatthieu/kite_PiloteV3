#include "hal/drivers/display_driver.h"
#include "core/logging.h"
#include <Wire.h> // Nécessaire pour LiquidCrystal_I2C

// Constructeur
DisplayDriver::DisplayDriver(const char* name)
    : OutputComponent(name),
      _lcd(nullptr),
      _isInitialized(false),
      _backlightState(true) {}

// Destructeur
DisplayDriver::~DisplayDriver() {
    if (_lcd != nullptr) {
        _lcd->noBacklight(); // Éteindre le rétroéclairage
        _lcd->clear();       // Effacer l'écran
        delete _lcd;
        _lcd = nullptr;
    }
}

// Initialisation du pilote
ErrorCode DisplayDriver::initialize() {
    if (_isInitialized) {
        LOG_WARNING(getComponentName(), "Déjà initialisé.");
        return ErrorCode::ALREADY_INITIALIZED;
    }
    setState(ComponentState::INITIALIZING);
    LOG_INFO(getComponentName(), "Initialisation...");

    // Vérifier si la configuration a été appliquée
    if (_driverConfig.columns == 0 || _driverConfig.rows == 0) {
        LOG_ERROR(getComponentName(), "Configuration des dimensions de l'écran non valide.");
        setState(ComponentState::ERROR);
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }

    // Créer et initialiser l'objet LCD
    _lcd = new LiquidCrystal_I2C(_driverConfig.i2cAddress, _driverConfig.columns, _driverConfig.rows);
    if (_lcd == nullptr) {
        LOG_ERROR(getComponentName(), "Échec de l'allocation mémoire pour LCD.");
        setState(ComponentState::ERROR);
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }

    _lcd->init(); // Initialiser l'écran LCD
    LOG_INFO(getComponentName(), "LCD initialisé à l'adresse 0x%X (%dx%d)", _driverConfig.i2cAddress, _driverConfig.columns, _driverConfig.rows);

    if (_backlightState) {
        _lcd->backlight();
    }
    _lcd->clear();
    _lcd->print("KitePilot V3..."); // Message de démarrage

    _isInitialized = true;
    setState(ComponentState::IDLE);
    LOG_INFO(getComponentName(), "Initialisation terminée.");
    return ErrorCode::OK;
}

// Configuration du pilote
ErrorCode DisplayDriver::configure(const DisplayDriverConfig& config) {
    LOG_INFO(getComponentName(), "Configuration du pilote d'affichage...");
    _driverConfig = config;

    if (_isInitialized) {
        LOG_WARNING(getComponentName(), "Reconfiguration : l'écran sera réinitialisé.");
        delete _lcd;
        _lcd = nullptr;
        _isInitialized = false;
        return initialize(); // Réinitialiser avec la nouvelle configuration
    }
    LOG_INFO(getComponentName(), "Configuration appliquée.");
    return ErrorCode::OK;
}

// Effacer l'écran
ErrorCode DisplayDriver::clear() {
    if (getState() != ComponentState::ACTIVE || !_lcd) {
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    _lcd->clear();
    LOG_DEBUG(getComponentName(), "Écran effacé.");
    return ErrorCode::OK;
}

// Définir la position du curseur
ErrorCode DisplayDriver::setCursor(uint8_t col, uint8_t row) {
    if (getState() != ComponentState::ACTIVE || !_lcd) {
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    if (col >= _driverConfig.columns || row >= _driverConfig.rows) {
        LOG_WARNING(getComponentName(), "Position du curseur hors limites (%d,%d).", col, row);
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    _lcd->setCursor(col, row);
    return ErrorCode::OK;
}

// Imprimer un message
ErrorCode DisplayDriver::print(const char* message) {
    if (getState() != ComponentState::ACTIVE || !_lcd) {
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    _lcd->print(message);
    LOG_DEBUG(getComponentName(), "Impression : %s", message);
    return ErrorCode::OK;
}

// Imprimer sur une ligne entière
ErrorCode DisplayDriver::printLine(uint8_t row, const char* message, bool clearLine) {
    if (getState() != ComponentState::ACTIVE || !_lcd) {
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    if (row >= _driverConfig.rows) {
        LOG_WARNING(getComponentName(), "Ligne hors limites pour printLine (%d).", row);
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }

    if (clearLine) {
        setCursor(0, row);
        for (uint8_t i = 0; i < _driverConfig.columns; ++i) _lcd->print(' ');
    }
    setCursor(0, row);
    _lcd->print(message);
    LOG_DEBUG(getComponentName(), "Impression ligne %d : %s", row, message);
    return ErrorCode::OK;
}

// Contrôler le rétroéclairage
ErrorCode DisplayDriver::setBacklight(bool state) {
    if (getState() != ComponentState::ACTIVE || !_lcd) {
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    if(!_isInitialized || !_lcd) return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    _backlightState = state;
    if (state) {
        _lcd->backlight();
        LOG_INFO(getComponentName(), "Rétroéclairage activé.");
    } else {
        _lcd->noBacklight();
        LOG_INFO(getComponentName(), "Rétroéclairage désactivé.");
    }
    return ErrorCode::OK;
}

// Définir un caractère personnalisé
ErrorCode DisplayDriver::customChar(uint8_t location, uint8_t* charmap) {
    if (getState() != ComponentState::ACTIVE || !_lcd) {
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    if (location >= 8) { // Les LCD permettent généralement 8 caractères personnalisés (0-7)
        LOG_WARNING(getComponentName(), "Emplacement du caractère personnalisé hors limites (%d).", location);
        return ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    _lcd->createChar(location, charmap);
    LOG_DEBUG(getComponentName(), "Caractère personnalisé %d créé.", location);
    return ErrorCode::OK;
}

// Appelé lorsque le composant est activé
void DisplayDriver::onEnable() {
    if (!_isInitialized) {
        LOG_ERROR(getComponentName(), "Impossible d'activer, non initialisé.");
        setState(ComponentState::ERROR);
        return;
    }
    setState(ComponentState::ACTIVE);
    LOG_INFO(getComponentName(), "Activé.");
}

// Appelé lorsque le composant est désactivé
void DisplayDriver::onDisable() {
    setState(ComponentState::COMPONENT_DISABLED);
    LOG_INFO(getComponentName(), "Désactivé.");
}
