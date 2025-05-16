#include "services/control_service.h"
#include "core/logging.h"
#include "utils/error_handler.h"
#include "core/task_manager.h"
#include "services/actuator_service.h" // <<< Assurez-vous qu'il est inclus

ControlService::ControlService()
    : ManagedComponent("ControlService", true),
      currentStrategy(ControlStrategy::MANUAL),
      sensorDataAvailable(false),
      lastSensorDataTime(0),
      actuatorServicePtr(nullptr)
{
    memset(&currentTargets, 0, sizeof(ControlTargets_t));
    memset(&latestSensorData, 0, sizeof(AggregatedSensorData));

    dataMutex = xSemaphoreCreateMutex();
    if (dataMutex == nullptr) {
        ErrorHandler::getInstance()->handleError(ErrorCode::CRITICAL, getComponentName(), "Échec de création du mutex de données.");
    }

    tensionPid.setTunings(PID_DEFAULT_KP, PID_DEFAULT_KI, PID_DEFAULT_KD);
    tensionPid.setOutputLimits(PID_DEFAULT_MIN_OUTPUT, PID_DEFAULT_MAX_OUTPUT);

    altitudePid.setTunings(PID_DEFAULT_KP, PID_DEFAULT_KI, PID_DEFAULT_KD);
    altitudePid.setOutputLimits(PID_DEFAULT_MIN_OUTPUT, PID_DEFAULT_MAX_OUTPUT);

    LOG_INFO(getComponentName(), "Constructeur appelé.");

    actuatorServicePtr = ActuatorService::getInstance();
    if (actuatorServicePtr == nullptr) {
        ErrorHandler::getInstance()->handleError(ErrorCode::CRITICAL, getComponentName(), "Échec de l'obtention de l'instance ActuatorService.");
    }
}

ControlService::~ControlService() {
    if (dataMutex != nullptr) {
        vSemaphoreDelete(dataMutex);
        dataMutex = nullptr;
    }
    LOG_INFO(getComponentName(), "Destructeur appelé.");
}

ErrorCode ControlService::initialize() {
    if (isInitialized()) {
        LOG_WARNING(getComponentName(), "Déjà initialisé.");
        return ErrorCode::ALREADY_INITIALIZED;
    }
    LOG_INFO(getComponentName(), "Initialisation...");

    if (dataMutex == nullptr) {
        LOG_ERROR(getComponentName(), "Mutex non créé. Initialisation échouée.");
        setState(ComponentState::ERROR);
        return ErrorCode::GENERAL_ERROR;
    }

    setState(ComponentState::ACTIVE);
    LOG_INFO(getComponentName(), "Initialisation terminée.");
    return ErrorCode::OK;
}

void ControlService::run() {
    if (getState() != ComponentState::ACTIVE) {
        return;
    }

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        bool dataFresh = sensorDataAvailable && (millis() - lastSensorDataTime < 1000);
        AggregatedSensorData currentSensors = latestSensorData;
        ControlStrategy currentStrategyCopy = currentStrategy;

        xSemaphoreGive(dataMutex);

        if (dataFresh) {
            if (currentStrategyCopy == ControlStrategy::MANUAL) {
                // En mode manuel, les commandes sont directes (potentiellement via UI ou comms)
                // Pour l'instant, ne fait rien ici, attend des commandes externes.
            } else if (currentStrategyCopy == ControlStrategy::AUTO_FLIGHT_FIGURE_EIGHT) {
                // Logique pour le mode automatique (ex: maintien d'altitude, suivi de ligne)
                // Ceci est un placeholder, la logique réelle doit être implémentée.
            }
        }
    }
}

ErrorCode ControlService::shutdown() {
    LOG_INFO(getComponentName(), "Arrêt...");
    setState(ComponentState::COMPONENT_DISABLED);
    LOG_INFO(getComponentName(), "Arrêt terminé.");
    return ErrorCode::OK;
}

