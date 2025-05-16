#include "utils/error_handler.h"
#include "core/logging.h" // Pour LOG_ERROR, LOG_INFO
#include <stdarg.h> // Pour va_list, va_start, va_end
#include <stdio.h>  // Pour vsnprintf

// Définition de l'instance singleton
ErrorHandler* ErrorHandler::_instance = nullptr;

// Constructeur privé
ErrorHandler::ErrorHandler() :
    _initialized(false),
    _errorCount(0) {
    _errorMutex = xSemaphoreCreateMutex();
    if (_errorMutex == NULL) {
        // Gérer l'échec de la création du mutex, peut-être logger directement si possible
        // ou marquer un état d'erreur critique interne.
        // Pour l'instant, on ne fait rien de spécial ici, mais c'est crucial.
    }
    // Initialiser _lastError
    _lastError.code = ErrorCode::OK;
    strncpy(_lastError.component, "None", sizeof(_lastError.component) - 1);
    _lastError.component[sizeof(_lastError.component) - 1] = '\0';
    strncpy(_lastError.message, "No error", sizeof(_lastError.message) - 1);
    _lastError.message[sizeof(_lastError.message) - 1] = '\0';
    _lastError.timestamp = 0;
}

// Destructeur
ErrorHandler::~ErrorHandler() {
    if (_errorMutex != NULL) {
        vSemaphoreDelete(_errorMutex);
        _errorMutex = NULL;
    }
}

// Obtention de l'instance unique
ErrorHandler* ErrorHandler::getInstance() {
    if (_instance == nullptr) {
        _instance = new ErrorHandler();
    }
    return _instance;
}

// Initialisation du gestionnaire d'erreurs
bool ErrorHandler::init() {
    if (_initialized) {
        return true;
    }
    if (_errorMutex == NULL) {
        // Ne pas initialiser si le mutex n'a pas pu être créé
        LOG_ERROR("ErrorHandler", "Initialisation échouée: Mutex non créé.");
        return false;
    }
    _initialized = true;
    _errorCount = 0;
    // Log l'initialisation
    LOG_INFO("ErrorHandler", "Gestionnaire d'erreurs initialisé.");
    return true;
}

// Gère une erreur système
void ErrorHandler::handleError(ErrorCode code, const char* component, const char* format, ...) {
    if (!_initialized || _errorMutex == NULL) {
        // Ne pas traiter les erreurs si non initialisé ou mutex invalide
        // On pourrait logger sur Serial directement en dernier recours
        return;
    }

    if (xSemaphoreTake(_errorMutex, portMAX_DELAY) == pdTRUE) {
        va_list args;
        va_start(args, format);
        vHandleError(code, component, format, args);
        va_end(args);
        xSemaphoreGive(_errorMutex);
    }
}

// Gère une erreur système avec va_list
void ErrorHandler::vHandleError(ErrorCode code, const char* component, const char* format, va_list args) {
    _errorCount++;
    _lastError.code = code;
    _lastError.timestamp = millis(); // Ou une autre source de temps si disponible

    if (component) {
        strncpy(_lastError.component, component, sizeof(_lastError.component) - 1);
        _lastError.component[sizeof(_lastError.component) - 1] = '\0';
    } else {
        strncpy(_lastError.component, "Unknown", sizeof(_lastError.component) - 1);
        _lastError.component[sizeof(_lastError.component) - 1] = '\0';
    }

    if (format) {
        vsnprintf(_lastError.message, MAX_ERROR_MESSAGE_LENGTH, format, args);
    } else {
        strncpy(_lastError.message, "No message provided", MAX_ERROR_MESSAGE_LENGTH - 1);
        _lastError.message[MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
    }

    // Journaliser l'erreur en utilisant le LoggingModule
    // S'assurer que LoggingModule est initialisé et fonctionnel
    if (LoggingModule::getInstance() && LoggingModule::getInstance()->isInitialized()) {
        LOG_ERROR(component ? component : "ErrorHandler", "Code: %d, Msg: %s", static_cast<int>(code), _lastError.message);
    } else {
        // Fallback si le logger n'est pas dispo: print direct sur Serial
        Serial.printf("[ERROR_FALLBACK] [%s] Code: %d, Msg: %s\n", component ? component : "ErrorHandler", static_cast<int>(code), _lastError.message);
    }

    // Ici, on pourrait ajouter une logique pour les erreurs critiques,
    // par exemple, notifier le SystemStateManager ou déclencher un mode sécurisé.
    // if (code == ErrorCode::CRITICAL_ERROR_EXAMPLE) {
    //     SystemStateManager::getInstance()->setSystemState(SystemState::EMERGENCY_STOP);
    // }
}


// Récupère la dernière erreur enregistrée
ErrorInfo ErrorHandler::getLastError() const {
    ErrorInfo tempError;
    if (_initialized && _errorMutex != NULL && xSemaphoreTake(_errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        tempError = _lastError;
        xSemaphoreGive(_errorMutex);
    } else {
        // Retourner une erreur par défaut ou indiquer un problème d'accès
        tempError.code = ErrorCode::GENERAL_ERROR;
        strncpy(tempError.component, "ErrorHandler", sizeof(tempError.component) -1);
        tempError.component[sizeof(tempError.component) -1] = '\0';
        strncpy(tempError.message, "Could not retrieve last error", sizeof(tempError.message) -1);
        tempError.message[sizeof(tempError.message) -1] = '\0';
        tempError.timestamp = millis();
    }
    return tempError;
}

// Récupère le nombre total d'erreurs enregistrées
uint32_t ErrorHandler::getErrorCount() const {
    uint32_t tempCount = 0;
     if (_initialized && _errorMutex != NULL && xSemaphoreTake(_errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        tempCount = _errorCount;
        xSemaphoreGive(_errorMutex);
    }
    return tempCount;
}

// Vérifie si le module est initialisé
bool ErrorHandler::isInitialized() const {
    return _initialized;
}

// Fonction globale pour faciliter le signalement des erreurs
void reportError(ErrorCode code, const char* component, const char* format, ...) {
    ErrorHandler* handler = ErrorHandler::getInstance();
    if (handler && handler->isInitialized()) {
        va_list args;
        va_start(args, format);
        // On appelle vHandleError directement pour éviter de reprendre le mutex deux fois
        // si handleError est appelée depuis reportError.
        // Cependant, pour la sécurité du mutex, il est préférable que handleError gère le mutex.
        // Donc, on passe par handleError.
        
        // Créer un buffer temporaire pour le message formaté
        char messageBuffer[MAX_ERROR_MESSAGE_LENGTH];
        vsnprintf(messageBuffer, MAX_ERROR_MESSAGE_LENGTH, format, args);
        va_end(args);
        
        // Appeler handleError avec le message déjà formaté
        handler->handleError(code, component, "%s", messageBuffer);
    } else {
        // Fallback si ErrorHandler n'est pas disponible
        va_list args;
        va_start(args, format);
        char tempBuffer[MAX_ERROR_MESSAGE_LENGTH + 64]; // Espace pour préfixe
        vsnprintf(tempBuffer + sprintf(tempBuffer, "[ERROR_HANDLER_UNAVAILABLE] [%s] Code %d: ", component ? component : "Unknown", static_cast<int>(code)), MAX_ERROR_MESSAGE_LENGTH, format, args);
        va_end(args);
        Serial.println(tempBuffer);
    }
}
