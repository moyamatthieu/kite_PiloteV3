#include "hal/drivers/imu_driver.h"
#include "core/logging.h" // Remplacer par le nouveau système de logging si différent
#include <Arduino.h> // Pour millis(), delay()
#include <cmath>     // Pour atan2, sqrt, etc.

// Ajouter les facteurs d'échelle IMU si manquants
#ifndef IMU_ACCEL_SCALE_FACTOR
#define IMU_ACCEL_SCALE_FACTOR 16384.0f // Pour ±2g (MPU6050)
#endif
#ifndef IMU_GYRO_SCALE_FACTOR
#define IMU_GYRO_SCALE_FACTOR 131.0f // Pour ±250dps (MPU6050)
#endif

// Helper pour écrire dans un registre I2C
static bool writeRegister(uint8_t deviceAddress, uint8_t regAddress, uint8_t value) {
    Wire.beginTransmission(deviceAddress);
    Wire.write(regAddress);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

IMUDriver::IMUDriver(const char* name)
    : SensorComponent(name, true), // Activé par défaut
      _calibrationState(ComponentState::UNINITIALIZED) {
    memset(&_rawData, 0, sizeof(RawIMUData));
    memset(&_processedData, 0, sizeof(ProcessedIMUData));
    _processedData.isValid = false;
}

ErrorCode IMUDriver::initialize() {
    setState(ComponentState::INITIALIZING);
    LOG_INFO("IMUDriver", "Initializing %s...", getName());

    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock

    // Vérification de la présence du MPU6050
    Wire.beginTransmission(_config.i2cAddress);
    if (Wire.endTransmission() != 0) {
        LOG_ERROR("IMUDriver", "MPU6050 not found at address 0x%02X", _config.i2cAddress);
        // TODO: scanI2CBus(); // Optionnel: lancer un scan si non trouvé
        setState(ComponentState::ERROR);
        return ErrorCode::HARDWARE_FAILURE;
    }
    LOG_INFO("IMUDriver", "MPU6050 found at address 0x%02X", _config.i2cAddress);

    // Réveil du MPU6050 (registre PWR_MGMT_1 = 0)
    if (!writeRegister(_config.i2cAddress, 0x6B, 0x00)) {
        LOG_ERROR("IMUDriver", "Failed to wake up MPU6050.");
        setState(ComponentState::ERROR);
        return ErrorCode::HARDWARE_FAILURE;
    }
    delay(100); // Attendre que le capteur se stabilise

    ErrorCode configStatus = applyConfig();
    if (configStatus != ErrorCode::OK) {
        setState(ComponentState::ERROR);
        return configStatus;
    }

    // Initialiser l'état de calibration
    // Si des valeurs de calibration sont stockées, les charger ici.
    // Sinon, marquer comme non calibré.
    _calibrationState = ComponentState::IDLE; // Supposons non calibré initialement, mais prêt à calibrer
    LOG_INFO("IMUDriver", "IMU %s initialized. Calibration state: IDLE (needs calibration).", getName());

    setState(ComponentState::IDLE);
    return ErrorCode::OK;
}

ErrorCode IMUDriver::applyConfig() {
    LOG_INFO("IMUDriver", "Applying configuration...");
    // Configurer Gyroscope Range (registre GYRO_CONFIG)
    if (!writeRegister(_config.i2cAddress, 0x1B, _config.gyroRange << 3)) {
        LOG_ERROR("IMUDriver", "Failed to set gyro range.");
        return ErrorCode::HARDWARE_FAILURE;
    }

    // Configurer Accelerometer Range (registre ACCEL_CONFIG)
    if (!writeRegister(_config.i2cAddress, 0x1C, _config.accelRange << 3)) {
        LOG_ERROR("IMUDriver", "Failed to set accel range.");
        return ErrorCode::HARDWARE_FAILURE;
    }

    // Configurer DLPF (registre CONFIG)
    if (!writeRegister(_config.i2cAddress, 0x1A, _config.dlpfBandwidth)) {
        LOG_ERROR("IMUDriver", "Failed to set DLPF bandwidth.");
        return ErrorCode::HARDWARE_FAILURE;
    }

    // Configurer Sample Rate (registre SMPLRT_DIV)
    if (!writeRegister(_config.i2cAddress, 0x19, _config.sampleRateDiv)) {
        LOG_ERROR("IMUDriver", "Failed to set sample rate divider.");
        return ErrorCode::HARDWARE_FAILURE;
    }
    LOG_INFO("IMUDriver", "Configuration applied.");
    return ErrorCode::OK;
}

ErrorCode IMUDriver::configure(const IMUDriverConfig& config) {
    LOG_INFO("IMUDriver", "New configuration received for %s.", getName());
    _config = config;
    if (getState() == ComponentState::ACTIVE || getState() == ComponentState::IDLE) {
        // Appliquer la configuration si le composant est déjà initialisé
        ErrorCode status = applyConfig();
        if (status != ErrorCode::OK) {
            LOG_ERROR("IMUDriver", "Failed to apply new configuration for %s.", getName());
            setState(ComponentState::ERROR);
            return status;
        }
    }
    return ErrorCode::OK;
}

void IMUDriver::readSensor() {
    if (getState() != ComponentState::ACTIVE && getState() != ComponentState::IDLE) {
        LOG_WARNING("IMUDriver", "%s not active or idle, cannot read sensor.", getName());
        _processedData.isValid = false;
        return;
    }

    if (_calibrationState == ComponentState::UNINITIALIZED || _calibrationState == ComponentState::ERROR) {
         LOG_WARNING("IMUDriver", "%s not calibrated or calibration error, data might be inaccurate.", getName());
    }

    ErrorCode readStatus = readRawValues(_rawData);
    if (readStatus == ErrorCode::OK) {
        processRawValues(_rawData, _processedData);
        _processedData.isValid = true;
        // LOG_DEBUG("IMUDriver", "Sensor data read: Acc(%.2f, %.2f, %.2f) Gyro(%.2f, %.2f, %.2f)", 
        //    _processedData.accel[0], _processedData.accel[1], _processedData.accel[2],
        //    _processedData.gyro[0], _processedData.gyro[1], _processedData.gyro[2]);
    } else {
        LOG_ERROR("IMUDriver", "Failed to read raw sensor data.");
        _processedData.isValid = false;
        // Potentiellement passer à l'état ERROR ici ou incrémenter un compteur d'erreurs
    }
}

ErrorCode IMUDriver::readRawValues(RawIMUData& data) {
    Wire.beginTransmission(_config.i2cAddress);
    Wire.write(0x3B); // Adresse de départ pour lire AccelX_H -> GyroZ_L (14 octets)
    if (Wire.endTransmission(false) != 0) { // false pour envoyer un restart et garder la connexion active
        LOG_ERROR("IMUDriver", "I2C transmission failed before reading data.");
        return ErrorCode::COMMUNICATION_ERROR;
    }

    if (Wire.requestFrom((uint8_t)_config.i2cAddress, (uint8_t)14, (bool)true) != 14) {
        LOG_ERROR("IMUDriver", "Failed to read 14 bytes from MPU6050.");
        return ErrorCode::COMMUNICATION_ERROR;
    }

    data.ax = (Wire.read() << 8) | Wire.read();
    data.ay = (Wire.read() << 8) | Wire.read();
    data.az = (Wire.read() << 8) | Wire.read();
    int16_t tempRaw = (Wire.read() << 8) | Wire.read();
    data.gx = (Wire.read() << 8) | Wire.read();
    data.gy = (Wire.read() << 8) | Wire.read();
    data.gz = (Wire.read() << 8) | Wire.read();

    data.temperature = (tempRaw / 340.0f) + 36.53f; // Formule spécifique au MPU6050

    return ErrorCode::OK;
}

void IMUDriver::processRawValues(const RawIMUData& raw, ProcessedIMUData& processed) {
    // Appliquer les facteurs d'échelle et les biais de calibration
    processed.accel[0] = (raw.ax / IMU_ACCEL_SCALE_FACTOR) - _accelBias[0];
    processed.accel[1] = (raw.ay / IMU_ACCEL_SCALE_FACTOR) - _accelBias[1];
    processed.accel[2] = (raw.az / IMU_ACCEL_SCALE_FACTOR) - _accelBias[2];

    processed.gyro[0] = (raw.gx / IMU_GYRO_SCALE_FACTOR) - _gyroBias[0];
    processed.gyro[1] = (raw.gy / IMU_GYRO_SCALE_FACTOR) - _gyroBias[1];
    processed.gyro[2] = (raw.gz / IMU_GYRO_SCALE_FACTOR) - _gyroBias[2];

    // Calcul simple de l'orientation (pitch et roll à partir des accéléromètres)
    // Attention: sensible aux accélérations linéaires, Yaw non observable sans magnétomètre.
    // Un filtre de fusion (Kalman, Madgwick, Mahony) est nécessaire pour une orientation robuste.
    calculateOrientation(processed, processed.orientation[0], processed.orientation[1], processed.orientation[2]);

    // Pour l'instant, quaternion non calculé (nécessite un algo de fusion)
    processed.quaternion[0] = 1.0f; // w (identité)
    processed.quaternion[1] = 0.0f; // x
    processed.quaternion[2] = 0.0f; // y
    processed.quaternion[3] = 0.0f; // z

    processed.timestamp = millis();
}

void IMUDriver::calculateOrientation(const ProcessedIMUData& imu, float& pitch, float& roll, float& yaw) {
    // Pitch (autour de l'axe Y)
    pitch = atan2(imu.accel[0], sqrt(imu.accel[1] * imu.accel[1] + imu.accel[2] * imu.accel[2])) * 180.0f / M_PI;
    // Roll (autour de l'axe X)
    roll  = atan2(imu.accel[1], imu.accel[2]) * 180.0f / M_PI;
    // Yaw (autour de l'axe Z) - Non calculable précisément sans magnétomètre
    // Pour une estimation basique, on pourrait intégrer la vitesse gyroscopique Z, mais cela dérivera rapidement.
    yaw = 0.0f; // Placeholder
}

ErrorCode IMUDriver::calibrate() {
    if (getState() != ComponentState::IDLE && getState() != ComponentState::ACTIVE) {
        LOG_ERROR("IMUDriver", "Cannot calibrate %s, not in IDLE or ACTIVE state.", getName());
        return ErrorCode::INVALID_STATE;
    }
    
    LOG_INFO("IMUDriver", "Starting IMU calibration for %s... Do not move the sensor.", getName());
    _calibrationState = ComponentState::INITIALIZING; // Indique que la calibration est en cours

    long gyroSum[3] = {0, 0, 0};
    long accelSum[3] = {0, 0, 0};
    RawIMUData tempRawData;

    for (int i = 0; i < IMU_CALIBRATION_SAMPLES; ++i) {
        if (readRawValues(tempRawData) != ErrorCode::OK) {
            LOG_ERROR("IMUDriver", "Calibration: Failed to read raw data sample %d.", i);
            _calibrationState = ComponentState::ERROR;
            return ErrorCode::SENSOR_FAILURE;
        }
        accelSum[0] += tempRawData.ax;
        accelSum[1] += tempRawData.ay;
        accelSum[2] += tempRawData.az;
        gyroSum[0] += tempRawData.gx;
        gyroSum[1] += tempRawData.gy;
        gyroSum[2] += tempRawData.gz;
        delay(3); // Petit délai entre les lectures
    }

    _gyroBias[0] = static_cast<float>(gyroSum[0]) / IMU_CALIBRATION_SAMPLES / IMU_GYRO_SCALE_FACTOR;
    _gyroBias[1] = static_cast<float>(gyroSum[1]) / IMU_CALIBRATION_SAMPLES / IMU_GYRO_SCALE_FACTOR;
    _gyroBias[2] = static_cast<float>(gyroSum[2]) / IMU_CALIBRATION_SAMPLES / IMU_GYRO_SCALE_FACTOR;

    _accelBias[0] = static_cast<float>(accelSum[0]) / IMU_CALIBRATION_SAMPLES / IMU_ACCEL_SCALE_FACTOR;
    _accelBias[1] = static_cast<float>(accelSum[1]) / IMU_CALIBRATION_SAMPLES / IMU_ACCEL_SCALE_FACTOR;
    // Pour l'axe Z de l'accéléromètre, on s'attend à 1g (ou -1g selon l'orientation).
    // Le biais est la différence par rapport à cette attente.
    // Supposons que l'IMU est à plat, face vers le haut, donc az devrait être ~IMU_ACCEL_SCALE_FACTOR (1g)
    _accelBias[2] = (static_cast<float>(accelSum[2]) / IMU_CALIBRATION_SAMPLES / IMU_ACCEL_SCALE_FACTOR) - 1.0f;
    // Si l'orientation est différente, ajuster le "1.0f" ou "-1.0f" attendu.

    LOG_INFO("IMUDriver", "Calibration complete for %s.", getName());
    LOG_INFO("IMUDriver", "Gyro Bias: X=%.4f, Y=%.4f, Z=%.4f dps", _gyroBias[0], _gyroBias[1], _gyroBias[2]);
    LOG_INFO("IMUDriver", "Accel Bias: X=%.4f, Y=%.4f, Z=%.4f g", _accelBias[0], _accelBias[1], _accelBias[2]);

    _calibrationState = ComponentState::ACTIVE; // Indique calibré et prêt
    // TODO: Sauvegarder les valeurs de biais en mémoire non volatile (EEPROM, NVS)
    return ErrorCode::OK;
}

bool IMUDriver::read() {
    if (getState() != ComponentState::ACTIVE) {
        _processedData.isValid = false;
        return false;
    }

    setState(ComponentState::SAMPLING);
    // LOG_DEBUG("IMUDriver", "Reading sensor %s", getName()); // Peut être trop verbeux

    RawIMUData raw;
    ErrorCode status = readRawValues(raw);

    if (status == ErrorCode::OK) {
        _rawData = raw; // Sauvegarder les données brutes
        processRawValues(_rawData, _processedData);
        _processedData.timestamp = millis();
        _processedData.isValid = true;
        // LOG_DEBUG("IMUDriver", "IMU Data: Acc(%.2f, %.2f, %.2f) Gyro(%.2f, %.2f, %.2f) Ori(%.2f, %.2f, %.2f)",
        //           _processedData.accel[0], _processedData.accel[1], _processedData.accel[2],
        //           _processedData.gyro[0], _processedData.gyro[1], _processedData.gyro[2],
        //           _processedData.orientation[0], _processedData.orientation[1], _processedData.orientation[2]);
    } else {
        LOG_ERROR("IMUDriver", "Failed to read raw values from %s. Error: %d", getName(), static_cast<int>(status));
        _processedData.isValid = false;
        setState(ComponentState::ERROR); // Ou IDLE si l'erreur n'est pas critique
        return false;
    }

    setState(ComponentState::ACTIVE); // Retour à l'état actif après lecture
    return _processedData.isValid;
}

bool IMUDriver::dataAvailable() const {
    return _processedData.isValid && (getState() == ComponentState::ACTIVE || getState() == ComponentState::IDLE);
}

float IMUDriver::getPitch() const {
    return _processedData.isValid ? _processedData.orientation[0] : 0.0f;
}

float IMUDriver::getRoll() const {
    return _processedData.isValid ? _processedData.orientation[1] : 0.0f;
}

float IMUDriver::getYaw() const {
    return _processedData.isValid ? _processedData.orientation[2] : 0.0f;
}

float IMUDriver::getAccelX() const {
    return _processedData.isValid ? _processedData.accel[0] : 0.0f;
}

float IMUDriver::getAccelY() const {
    return _processedData.isValid ? _processedData.accel[1] : 0.0f;
}

float IMUDriver::getAccelZ() const {
    return _processedData.isValid ? _processedData.accel[2] : 0.0f;
}

float IMUDriver::getGyroX() const {
    return _processedData.isValid ? _processedData.gyro[0] : 0.0f;
}

float IMUDriver::getGyroY() const {
    return _processedData.isValid ? _processedData.gyro[1] : 0.0f;
}

float IMUDriver::getGyroZ() const {
    return _processedData.isValid ? _processedData.gyro[2] : 0.0f;
}

float IMUDriver::getTemperature() const {
    // La température est généralement stockée dans _rawData pour MPU6050
    // et convertie lors du traitement si nécessaire, ou directement utilisable.
    // Ici, nous supposons qu'elle est dans _rawData.temperature et déjà en °C.
    // Si _processedData avait un champ température, on l'utiliserait.
    return _rawData.temperature; // Ou une valeur par défaut si non valide/disponible
}

bool IMUDriver::selfTest() {
    // L'auto-test du MPU6050 active des actionneurs internes pour simuler des lectures.
    // Les détails de l'interprétation des valeurs de self-test sont dans la datasheet du MPU6050.
    // Cette fonction est une implémentation basique.
    LOG_INFO("IMUDriver", "Starting self-test for %s...", getName());

    // Sauvegarder la configuration actuelle
    uint8_t prevGyroConfig, prevAccelConfig;
    Wire.beginTransmission(_config.i2cAddress);
    Wire.write(0x1B); // GYRO_CONFIG
    Wire.endTransmission(false);
    Wire.requestFrom(_config.i2cAddress, (uint8_t)1, (bool)true);
    prevGyroConfig = Wire.read();

    Wire.beginTransmission(_config.i2cAddress);
    Wire.write(0x1C); // ACCEL_CONFIG
    Wire.endTransmission(false);
    Wire.requestFrom(_config.i2cAddress, (uint8_t)1, (bool)true);
    prevAccelConfig = Wire.read();

    // Activer le self-test: mettre les bits ST correspondants à 1
    // Gyro ST: bits 7 (XG_ST), 6 (YG_ST), 5 (ZG_ST)
    // Accel ST: bits 7 (XA_ST), 6 (YA_ST), 5 (ZA_ST)
    if (!writeRegister(_config.i2cAddress, 0x1B, prevGyroConfig | 0xE0)) return false; // Active tous les self-tests gyro
    if (!writeRegister(_config.i2cAddress, 0x1C, prevAccelConfig | 0xE0)) return false; // Active tous les self-tests accel

    delay(250); // Attendre que les mesures se stabilisent avec le self-test activé

    RawIMUData stEnabledData;
    if (readRawValues(stEnabledData) != ErrorCode::OK) {
        LOG_ERROR("IMUDriver", "Self-test: Failed to read data with ST enabled.");
        // Restaurer la config
        writeRegister(_config.i2cAddress, 0x1B, prevGyroConfig);
        writeRegister(_config.i2cAddress, 0x1C, prevAccelConfig);
        return false;
    }

    // Désactiver le self-test (restaurer la config précédente)
    if (!writeRegister(_config.i2cAddress, 0x1B, prevGyroConfig)) return false;
    if (!writeRegister(_config.i2cAddress, 0x1C, prevAccelConfig)) return false;

    delay(250); // Attendre que les mesures se stabilisent avec le self-test désactivé

    RawIMUData stDisabledData;
    if (readRawValues(stDisabledData) != ErrorCode::OK) {
        LOG_ERROR("IMUDriver", "Self-test: Failed to read data with ST disabled.");
        return false;
    }

    // Calculer la différence: ST_response = ST_enabled_data - ST_disabled_data
    // Ces valeurs doivent être comparées aux valeurs attendues de la datasheet (Factory Trim values)
    // Pour simplifier, on vérifie juste si la différence est non nulle (ce qui est une simplification excessive)
    bool success = false;
    if (std::abs(stEnabledData.ax - stDisabledData.ax) > 0) success = true;
    if (std::abs(stEnabledData.ay - stDisabledData.ay) > 0) success = true;
    if (std::abs(stEnabledData.az - stDisabledData.az) > 0) success = true;
    if (std::abs(stEnabledData.gx - stDisabledData.gx) > 0) success = true;
    if (std::abs(stEnabledData.gy - stDisabledData.gy) > 0) success = true;
    if (std::abs(stEnabledData.gz - stDisabledData.gz) > 0) success = true;

    if (success) {
        LOG_INFO("IMUDriver", "Self-test for %s PASSED (basic check).", getName());
    } else {
        LOG_ERROR("IMUDriver", "Self-test for %s FAILED (basic check).", getName());
    }
    // Une implémentation complète nécessiterait de comparer les STR (Self-Test Responses)
    // aux valeurs FT (Factory Trim) stockées dans les registres de self-test du MPU6050.
    return success;
}

void IMUDriver::sleep(bool enable) {
    if (writeRegister(_config.i2cAddress, 0x6B, enable ? 0x40 : 0x00)) { // PWR_MGMT_1, bit 6 SLEEP
        LOG_INFO("IMUDriver", "%s %s.", getName(), enable ? "put to sleep" : "woken up");
        setState(enable ? ComponentState::SUSPENDED : ComponentState::IDLE);
    } else {
        LOG_ERROR("IMUDriver", "Failed to change sleep state for %s.", getName());
    }
}

void IMUDriver::onEnable() {
    LOG_INFO("IMUDriver", "%s enabled. Initializing...", getName());
    initialize(); 
}

void IMUDriver::onDisable() {
    LOG_INFO("IMUDriver", "%s disabled. Putting to sleep...", getName());
    sleep(true);
    setState(ComponentState::COMPONENT_DISABLED);
}

// Note: La macro REGISTER_COMPONENT n'est pas utilisée ici car l'instance
// du pilote sera typiquement gérée par un SensorService ou directement
// par le code d'initialisation principal (main.cpp) qui connaît les pilotes HAL nécessaires.
// Si on voulait un IMUDriver global unique accessible via ComponentRegistry, on pourrait l'ajouter.
