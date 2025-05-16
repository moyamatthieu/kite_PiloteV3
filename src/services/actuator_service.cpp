#include "services/actuator_service.h"
#include "core/logging.h"
#include "utils/error_handler.h"
#include "core/config.h" // Pour les pins et les configurations par défaut

// Static member initialization
ActuatorService* ActuatorService::instance = nullptr;
SemaphoreHandle_t ActuatorService::instanceMutex = nullptr;

ActuatorService::ActuatorService()
    : ManagedComponent("ActuatorService", true),
      currentWinchSpeedCommand(0.0f)
{
    // Initialize the static instanceMutex if this is the first instance creation
    if (ActuatorService::instanceMutex == nullptr) {
        ActuatorService::instanceMutex = xSemaphoreCreateMutex();
        if (ActuatorService::instanceMutex == nullptr) {
            // This is a critical failure, ErrorHandler might not be available yet.
            // Consider a more direct form of error reporting or halt.
            // For now, logging if possible, but this state is problematic.
            if (LoggingModule::getInstance()) { // Check if logging is up
                LOG_ERROR(getComponentName(), "Échec de création du instanceMutex pour ActuatorService singleton.");
            }
        }
    }

    LOG_INFO(getComponentName(), "Constructeur appelé.");

    // Initialisation du treuil
    // Assurez-vous que WINCH_PWM_PIN, WINCH_DIR_PIN, etc. sont définis dans config.h
    winchMotor.reset(new WinchDriver("WinchPrincipal"));

    // Initialisation des servos
    // Supposons 2 servos pour l'instant: GAUCHE et DROITE
    // Assurez-vous que SERVO_LEFT_PIN, SERVO_RIGHT_PIN, etc. sont définis dans config.h
    servos.resize(2); // Pour SERVO_LEFT_ID et SERVO_RIGHT_ID
    currentServoAngleCommands.resize(2, SERVO_DEFAULT_NEUTRAL_ANGLE);

    if (servos.size() > SERVO_LEFT_ID) {
        servos[SERVO_LEFT_ID].reset(new ServoDriver("ServoGauche"));
    }
    if (servos.size() > SERVO_RIGHT_ID) {
        servos[SERVO_RIGHT_ID].reset(new ServoDriver("ServoDroit"));
    }
    // commandMutex = xSemaphoreCreateMutex(); // Décommenter si une synchronisation complexe est nécessaire
}

ActuatorService::~ActuatorService() {
    LOG_INFO(getComponentName(), "Destructeur appelé.");
    // Les unique_ptr s'occupent de la suppression des objets HAL
    // If instanceMutex was created by this class instance (which it is, as static),
    // it should ideally be deleted if the singleton instance is destroyed.
    // However, typical singletons live for the program duration.
    // If a static destroyInstance() method is added, it could delete the mutex.
    // For now, matching SystemOrchestrator, which doesn't explicitly delete its static mutex in the destructor.
}

ActuatorService* ActuatorService::getInstance() {
    if (ActuatorService::instanceMutex == nullptr) {
        // This implies that the constructor (which creates the mutex) has not been called,
        // or it failed to create the mutex. This is a critical state.
        // Returning nullptr indicates a severe issue.
        // Logging here is difficult if ErrorHandler/LoggingModule also rely on singletons.
        return nullptr;
    }

    xSemaphoreTake(ActuatorService::instanceMutex, portMAX_DELAY);
    if (ActuatorService::instance == nullptr) {
        ActuatorService::instance = new ActuatorService();
        // Note: If `new ActuatorService()` fails (e.g., out of memory), `instance` will remain nullptr.
        // The caller should check the returned pointer.
    }
    xSemaphoreGive(ActuatorService::instanceMutex);
    return ActuatorService::instance;
}

ErrorCode ActuatorService::initialize() {
    if (isInitialized()) {
        LOG_WARNING(getComponentName(), "Déjà initialisé.");
        return ErrorCode::ALREADY_INITIALIZED;
    }
    LOG_INFO(getComponentName(), "Initialisation...");

    ErrorCode status = ErrorCode::OK;
    if (winchMotor) {
        ErrorCode winchStatus = winchMotor->initialize();
        if (winchStatus != ErrorCode::OK) {
            ErrorHandler::getInstance()->handleError(ErrorCode::HARDWARE_FAILURE, getComponentName(), "Échec de l'initialisation du WinchDriver.");
            status = winchStatus;
        }
    } else {
        ErrorHandler::getInstance()->handleError(ErrorCode::CRITICAL, getComponentName(), "WinchDriver non instancié.");
        status = ErrorCode::NOT_INITIALIZED;
    }

    for (size_t i = 0; i < servos.size(); ++i) {
        if (servos[i]) {
            ErrorCode servoStatus = servos[i]->initialize();
            if (servoStatus != ErrorCode::OK) {
                char msg[100];
                snprintf(msg, sizeof(msg), "Échec de l'initialisation du ServoDriver ID: %zu", i);
                ErrorHandler::getInstance()->handleError(ErrorCode::HARDWARE_FAILURE, getComponentName(), msg);
                status = servoStatus;
            }
        } else {
            char msg[100];
            snprintf(msg, sizeof(msg), "ServoDriver ID: %zu non instancié.", i);
            ErrorHandler::getInstance()->handleError(ErrorCode::CRITICAL, getComponentName(), msg);
            status = ErrorCode::NOT_INITIALIZED;
        }
    }

    if (status == ErrorCode::OK) {
        setSafeState();
        setState(ComponentState::ACTIVE);
        LOG_INFO(getComponentName(), "Initialisation terminée.");
    } else {
        setState(ComponentState::ERROR);
        LOG_ERROR(getComponentName(), "Échec de l'initialisation d'un ou plusieurs actionneurs.");
    }
    return status;
}

void ActuatorService::run() {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        if (getState() == ComponentState::IDLE) {
            setState(ComponentState::ACTIVE);
        } else {
            return;
        }
    }
    // Cette fonction peut être utilisée pour des tâches périodiques liées aux actionneurs,
    // comme la surveillance de leur état, la gestion de la température du moteur du treuil, etc.
    // Pour l'instant, la plupart des commandes sont directes.
    // LOG_DEBUG(getComponentName(), "Cycle run().");
}

ErrorCode ActuatorService::shutdown() {
    LOG_INFO(getComponentName(), "Arrêt...");
    setState(ComponentState::COMPONENT_DISABLED);
    setSafeState();
    LOG_INFO(getComponentName(), "Arrêt terminé.");
    return ErrorCode::OK;
}

void ActuatorService::setSafeState() {
    LOG_INFO(getComponentName(), "Mise des actionneurs en état sûr.");
    setWinchSpeed(0.0f);
    for (size_t i = 0; i < servos.size(); ++i) {
        if (servos[i]) {
            // Utiliser la configuration SERVO_DEFAULT_NEUTRAL_ANGLE de config.h
            setServoAngle(static_cast<uint8_t>(i), SERVO_DEFAULT_NEUTRAL_ANGLE);
        }
    }
}

ErrorCode ActuatorService::setWinchSpeed(float normalizedSpeed) {
    if (!winchMotor) {
        LOG_ERROR(getComponentName(), "WinchDriver non disponible pour setWinchSpeed.");
        return ErrorCode::NOT_INITIALIZED;
    }

    // Brider la vitesse normalisée
    if (normalizedSpeed < WINCH_DEFAULT_MIN_SPEED) normalizedSpeed = WINCH_DEFAULT_MIN_SPEED;
    if (normalizedSpeed > WINCH_DEFAULT_MAX_SPEED) normalizedSpeed = WINCH_DEFAULT_MAX_SPEED;

    return winchMotor->setSpeed(normalizedSpeed);
}

float ActuatorService::getWinchSpeed() const {
    return currentWinchSpeedCommand;
}

ErrorCode ActuatorService::setServoAngle(uint8_t servoId, float angleDegrees) {
    if (servoId >= servos.size() || !servos[servoId]) {
        char msg[100];
        snprintf(msg, sizeof(msg), "ServoDriver ID %u non disponible/valide pour setServoAngle.", servoId);
        LOG_ERROR(getComponentName(), "%s", msg);
        return ErrorCode::NOT_INITIALIZED;
    }

    // Ici, il faut fournir le type logique attendu par ServoDriver (exemple : ServoLogicalType::DIRECTION, etc.)
    // Pour l'exemple, on suppose que l'ID correspond à l'enum
    return servos[servoId]->setAngle(static_cast<ServoLogicalType>(servoId), angleDegrees);
}

float ActuatorService::getServoAngle(uint8_t servoId) const {
    if (servoId >= currentServoAngleCommands.size()) {
        char msg[100];
        snprintf(msg, sizeof(msg), "ServoID %u invalide pour getServoAngle.", servoId);
        LOG_WARNING(getComponentName(), msg);
        return 0.0f; // Ou une valeur d'erreur
    }
    return currentServoAngleCommands[servoId];
}

ErrorCode ActuatorService::setServoNormalizedPosition(uint8_t servoId, float normalizedPosition) {
    if (servoId >= servos.size() || !servos[servoId]) {
        char msg[100];
        snprintf(msg, sizeof(msg), "ServoDriver ID %u non disponible/valide pour setServoNormalizedPosition.", servoId);
        LOG_ERROR(getComponentName(), "%s", msg);
        return ErrorCode::NOT_INITIALIZED;
    }

    // À adapter selon l'API ServoDriver
    return ErrorCode::GENERAL_ERROR; // Placeholder si la méthode n'existe pas
}

