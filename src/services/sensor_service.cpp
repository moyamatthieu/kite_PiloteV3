#include "services/sensor_service.h"
#include "core/logging.h"
#include "utils/error_handler.h"
#include <Arduino.h>

SensorService::SensorService(IMUDriver* imu, WindDriver* wind,
                             TensionDriver* tensionL, TensionDriver* tensionR,
                             LineLengthDriver* lineL, LineLengthDriver* lineR,
                             PotentiometerDriver* potMain, PotentiometerDriver* potTrim)
    : ManagedComponent("SensorService", true),
      imu_driver(imu), wind_driver(wind),
      tension_left_driver(tensionL), tension_right_driver(tensionR),
      line_length_left_driver(lineL), line_length_right_driver(lineR),
      pot_main_driver(potMain), pot_trim_driver(potTrim) {
    data_mutex = xSemaphoreCreateMutex();
    if (data_mutex == nullptr) {
        ErrorHandler::getInstance()->handleError(ErrorCode::CRITICAL, getComponentName(), "Échec de création du mutex pour SensorService.");
    }
}

SensorService::~SensorService() {
    if (data_mutex != nullptr) {
        vSemaphoreDelete(data_mutex);
        data_mutex = nullptr;
    }
    // Les pilotes de capteurs sont gérés (possédés et supprimés) par TaskManager ou le créateur de SensorService.
    // SensorService ne les supprime pas.
}

ErrorCode SensorService::initialize() {
    if (isInitialized()) {
        LOG_WARNING(getComponentName(), "Déjà initialisé.");
        return ErrorCode::ALREADY_INITIALIZED;
    }
    LOG_INFO(getComponentName(), "Initialisation...");
    if (data_mutex == nullptr) {
        LOG_ERROR(getComponentName(), "Mutex non créé. Initialisation échouée.");
        setState(ComponentState::ERROR);
        return ErrorCode::GENERAL_ERROR;
    }
    setState(ComponentState::ACTIVE);
    LOG_INFO(getComponentName(), "Initialisation terminée.");
    return ErrorCode::OK;
}

void SensorService::run() {
    if (getState() != ComponentState::ACTIVE) {
        return;
    }

    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)) == pdTRUE) { // Attente max 10ms pour le mutex
        current_sensor_data.timestamp = millis();

        updateImuData();
        updateWindData();
        updateTensionData();
        updateLineLengthData();
        updatePotentiometerData();

        xSemaphoreGive(data_mutex);
        
        // Optionnel: Log des données agrégées à des fins de débogage (peut être verbeux)
        // LOG_DEBUG(getComponentName(), "Données capteurs agrégées. IMU Pitch: %.2f", current_sensor_data.imu_pitch);

    } else {
        LOG_WARNING(getComponentName(), "Impossible d'acquérir le mutex pour mettre à jour les données capteurs.");
    }
    // Ce service s'exécute généralement dans sa propre tâche à une fréquence définie par TaskManager.
    // La fréquence de lecture réelle des capteurs dépendra de la fréquence d'appel de cette méthode `run()`.
}

ErrorCode SensorService::shutdown() {
    LOG_INFO(getComponentName(), "Arrêt...");
    setState(ComponentState::COMPONENT_DISABLED);
    LOG_INFO(getComponentName(), "Arrêt terminé.");
    return ErrorCode::OK;
}

AggregatedSensorData SensorService::getAggregatedData() const {
    AggregatedSensorData data_copy;
    if (data_mutex == nullptr) {
        LOG_ERROR(getComponentName(), "Mutex non initialisé dans getAggregatedData.");
        return AggregatedSensorData(); // Retourne une structure vide/par défaut
    }
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        data_copy = current_sensor_data;
        xSemaphoreGive(data_mutex);
    } else {
        LOG_ERROR(getComponentName(), "Impossible d'acquérir le mutex pour lire les données capteurs.");
        // Retourner des données potentiellement périmées ou une structure vide
        // Pourrait être amélioré avec une gestion d'erreur plus robuste
    }
    return data_copy;
}

bool SensorService::isImuDataAvailable() const {
    if (data_mutex == nullptr) return false;
    bool available = false;
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        available = current_sensor_data.imu_available;
        xSemaphoreGive(data_mutex);
    }
    return available;
}

// --- Méthodes privées de mise à jour --- 

void SensorService::updateImuData() {
    // Vérifier si le driver IMU est initialisé et actif
    if (imu_driver && imu_driver->isInitialized() && imu_driver->getState() == ComponentState::ACTIVE) {
        // Supposons que IMUDriver a une méthode pour vérifier si de nouvelles données sont disponibles
        // et des méthodes pour récupérer les données.
        // Ces méthodes devront être implémentées dans IMUDriver.
        // if (imu_driver->dataAvailable()) { 
        //     current_sensor_data.imu_pitch = imu_driver->getPitch();
        //     current_sensor_data.imu_roll = imu_driver->getRoll();
        //     current_sensor_data.imu_yaw = imu_driver->getYaw();
        //     current_sensor_data.imu_accX = imu_driver->getAccelX();
        //     current_sensor_data.imu_accY = imu_driver->getAccelY();
        //     current_sensor_data.imu_accZ = imu_driver->getAccelZ();
        //     current_sensor_data.imu_gyroX = imu_driver->getGyroX();
        //     current_sensor_data.imu_gyroY = imu_driver->getGyroY();
        //     current_sensor_data.imu_gyroZ = imu_driver->getGyroZ();
        //     current_sensor_data.imu_temperature = imu_driver->getTemperature();
        // }
    }
}

void SensorService::updateWindData() {
    // Vérifier si le driver Vent est initialisé et actif
    if (wind_driver && wind_driver->isInitialized() && wind_driver->getState() == ComponentState::ACTIVE) {
        // Supposons que WindDriver a des méthodes pour récupérer les données.
        // Ces méthodes devront être implémentées dans WindDriver.
        // if (wind_driver->dataAvailable()) { 
        //     current_sensor_data.wind_speed = wind_driver->getWindSpeed();
        //     current_sensor_data.wind_direction = wind_driver->getWindDirection();
        // }
    }
}

void SensorService::updateTensionData() {
    // Capteur de tension gauche
    if (tension_left_driver && tension_left_driver->isInitialized() && tension_left_driver->getState() == ComponentState::ACTIVE) {
        // current_sensor_data.tension_left = tension_left_driver->getTension();
    } else {
        current_sensor_data.tension_left = 0.0f; // Valeur par défaut ou d'erreur
    }

    // Capteur de tension droit
    if (tension_right_driver && tension_right_driver->isInitialized() && tension_right_driver->getState() == ComponentState::ACTIVE) {
        // current_sensor_data.tension_right = tension_right_driver->getTension();
    } else {
        current_sensor_data.tension_right = 0.0f; // Valeur par défaut ou d'erreur
    }
}

void SensorService::updateLineLengthData() {
    // Capteur de longueur de ligne gauche
    if (line_length_left_driver && line_length_left_driver->isInitialized() && line_length_left_driver->getState() == ComponentState::ACTIVE) {
        // current_sensor_data.line_length_left = line_length_left_driver->getLineLength();
    } else {
        current_sensor_data.line_length_left = 0.0f; // Valeur par défaut
    }

    // Capteur de longueur de ligne droit
    if (line_length_right_driver && line_length_right_driver->isInitialized() && line_length_right_driver->getState() == ComponentState::ACTIVE) {
        // current_sensor_data.line_length_right = line_length_right_driver->getLineLength();
    } else {
        current_sensor_data.line_length_right = 0.0f; // Valeur par défaut
    }
}

void SensorService::updatePotentiometerData() {
    // Potentiomètre principal
    if (pot_main_driver && pot_main_driver->isInitialized() && pot_main_driver->getState() == ComponentState::ACTIVE) {
        // current_sensor_data.pot_main_command = pot_main_driver->getValue(PotentiometerType::MAIN_COMMAND);
    } else {
        current_sensor_data.pot_main_command = 0.0f; // Valeur par défaut
    }

    // Potentiomètre de trim
    if (pot_trim_driver && pot_trim_driver->isInitialized() && pot_trim_driver->getState() == ComponentState::ACTIVE) {
        // current_sensor_data.pot_trim = pot_trim_driver->getValue(PotentiometerType::TRIM_ADJUST);
    } else {
        current_sensor_data.pot_trim = 0.0f; // Valeur par défaut
    }
}
