/*
  -----------------------
  Kite PiloteV3 - Module IMU (Implémentation)
  -----------------------
  
  Implémentation du module de gestion de l'unité de mesure inertielle (IMU).
  
  Version: 2.0.0
  Date: 9 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/imu.h"
#include <ArduinoJson.h>
#include "utils/diagnostics.h" // Ajout pour scanI2CBus
#include "utils/logging.h"     // Ajout pour LOG_INFO et LOG_ERROR

// Instance globale unique de IMUSensor
static IMUSensor* _imuSensorInstance = nullptr;

// Constructeur
IMUSensor::IMUSensor()
    : SensorModule("IMU"),
      _fsm(this),
      _calState(IMU_NOT_CALIBRATED)
{
    // Initialisation des valeurs par défaut
    memset(&_lastData, 0, sizeof(IMUData));
    memset(&_config, 0, sizeof(IMUConfig));
    memset(_lastError, 0, sizeof(_lastError));
    
    // Configuration par défaut
    _config.gyroRange = 0;      // ±250 deg/s
    _config.accelRange = 0;     // ±2g
    _config.dlpfBandwidth = 0;  // 260Hz
    _config.sampleRate = 0;     // 8kHz
    _config.fifoEnabled = false;
    _config.lowPowerMode = false;
    
    // Enregistrement de l'instance pour les fonctions C
    _imuSensorInstance = this;
}

// Destructeur
IMUSensor::~IMUSensor() {
    if (_imuSensorInstance == this) {
        _imuSensorInstance = nullptr;
    }
}

// Initialisation du capteur
bool IMUSensor::init(const IMUConfig* config) {
    // Initialisation de l'IMU avec les paramètres par défaut ou spécifiés
    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock

    // Vérification de la présence du MPU6050
    Wire.beginTransmission(IMU_I2C_ADDR);
    if (Wire.endTransmission() != 0) {
        LOG_ERROR("IMU", "MPU6050 not found at address 0x%02X", IMU_I2C_ADDR);
        scanI2CBus(); // Lancer le scan si le MPU6050 n'est pas trouvé
        return false; // Échec de l'initialisation
    }
    LOG_INFO("IMU", "MPU6050 found at address 0x%02X", IMU_I2C_ADDR);
    
    // Réveil du MPU6050
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);    // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    
    // Configuration si des paramètres sont fournis
    if (config != nullptr) {
        memcpy(&_config, config, sizeof(IMUConfig));
        
        Wire.beginTransmission(IMU_I2C_ADDR);
        Wire.write(0x1B); // GYRO_CONFIG register
        Wire.write(_config.gyroRange << 3); // Set gyro range
        Wire.endTransmission(true);
        
        Wire.beginTransmission(IMU_I2C_ADDR);
        Wire.write(0x1C); // ACCEL_CONFIG register
        Wire.write(_config.accelRange << 3); // Set accel range
        Wire.endTransmission(true);
        
        Wire.beginTransmission(IMU_I2C_ADDR);
        Wire.write(0x1A); // CONFIG register
        Wire.write(_config.dlpfBandwidth); // Set DLPF bandwidth
        Wire.endTransmission(true);
        
        Wire.beginTransmission(IMU_I2C_ADDR);
        Wire.write(0x19); // SMPLRT_DIV register
        Wire.write(_config.sampleRate); // Set sample rate
        Wire.endTransmission(true);
    }
    
    return true;
}

// Lecture des données du capteur
void IMUSensor::readSensor() {
    // Vérifier si l'initialisation est nécessaire
    static bool initialized = false;
    if (!initialized) {
        _fsm.transitionTo(IMU_STATE_INIT);
        if (!init(nullptr)) {
            ErrorManager::getInstance()->reportError(
                ErrorCode::SENSOR_ERROR,
                "IMU",
                "IMU initialization failed"
            );
            setState(State::MODULE_ERROR);
            _fsm.transitionTo(IMU_STATE_ERROR);
            return;
        }
        initialized = true;
    }
    
    // Lecture des données
    _fsm.transitionTo(IMU_STATE_READING);
    RawIMUData rawData;
    if (!readRawData(&rawData)) {
        ErrorManager::getInstance()->reportError(
            ErrorCode::SENSOR_ERROR,
            "IMU",
            "IMU raw data read failed"
        );
        setState(State::MODULE_ERROR);
        _fsm.transitionTo(IMU_STATE_ERROR);
        return;
    }
    
    // Traitement des données
    processRawData(rawData, _lastData);
    
    // Mise à jour de l'état du module
    setState(State::MODULE_ENABLED);
    _fsm.transitionTo(IMU_STATE_IDLE);
}

// Configuration du capteur
void IMUSensor::configure(const std::string& jsonConfig) {
    // Utilisation d'ArduinoJson pour parser la configuration
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, jsonConfig);
    
    if (error) {
        snprintf(_lastError, sizeof(_lastError), "JSON parsing error: %s", error.c_str());
        return;
    }
    
    // Application de la configuration
    IMUConfig newConfig = _config;
    
    if (doc.containsKey("gyroRange")) {
        newConfig.gyroRange = doc["gyroRange"];
    }
    
    if (doc.containsKey("accelRange")) {
        newConfig.accelRange = doc["accelRange"];
    }
    
    if (doc.containsKey("dlpfBandwidth")) {
        newConfig.dlpfBandwidth = doc["dlpfBandwidth"];
    }
    
    if (doc.containsKey("sampleRate")) {
        newConfig.sampleRate = doc["sampleRate"];
    }
    
    if (doc.containsKey("fifoEnabled")) {
        newConfig.fifoEnabled = doc["fifoEnabled"];
    }
    
    if (doc.containsKey("lowPowerMode")) {
        newConfig.lowPowerMode = doc["lowPowerMode"];
    }
    
    // Application de la nouvelle configuration
    init(&newConfig);
}

// Calibration du capteur
IMUCalibrationState IMUSensor::calibrate() {
    // Implémentation de la calibration
    RawIMUData sumData = {0};
    int validSamples = 0;
    
    // Collecte des échantillons
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        RawIMUData data;
        if (readRawData(&data)) {
            sumData.ax += data.ax;
            sumData.ay += data.ay;
            sumData.az += data.az;
            sumData.gx += data.gx;
            sumData.gy += data.gy;
            sumData.gz += data.gz;
            validSamples++;
        }
        delay(2); // Petit délai entre les échantillons
    }
    
    // Vérification du nombre d'échantillons valides
    if (validSamples < CALIBRATION_SAMPLES * 0.9) {
        _calState = IMU_CALIBRATION_ERROR;
        return _calState;
    }
    
    // Calcul des offsets
    // Dans une implémentation réelle, ces offsets seraient stockés et appliqués
    
    _calState = IMU_CALIBRATED;
    return _calState;
}

// Obtention des dernières données
const IMUData& IMUSensor::getData() const {
    return _lastData;
}

// Auto-test du capteur
bool IMUSensor::selfTest() {
    // Implémentation de l'auto-test
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x1B); // GYRO_CONFIG register
    Wire.write(0xE0); // Set self-test bits
    Wire.endTransmission(true);
    
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x1C); // ACCEL_CONFIG register
    Wire.write(0xE0); // Set self-test bits
    Wire.endTransmission(true);
    
    delay(100); // Attendre que le test s'exécute
    
    // Lecture des résultats du test
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x0D); // SELF_TEST_X register
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)IMU_I2C_ADDR, (uint8_t)4, (bool)true);
    
    uint8_t testX = Wire.read();
    uint8_t testY = Wire.read();
    uint8_t testZ = Wire.read();
    uint8_t testA = Wire.read();
    
    // Restaurer la configuration normale
    init(&_config);
    
    // Vérification des résultats (simplifié)
    return (testX != 0 || testY != 0 || testZ != 0 || testA != 0);
}

// Mise en veille du capteur
void IMUSensor::sleep(bool enable) {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(enable ? 0x40 : 0x00); // Set/clear sleep bit
    Wire.endTransmission(true);
}

// Description du module
const char* IMUSensor::description() const {
    return "Capteur IMU (MPU6050)";
}

// Lecture des données brutes
bool IMUSensor::readRawData(RawIMUData* data) {
    if (data == nullptr) return false;
    
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)IMU_I2C_ADDR, (uint8_t)14, (bool)true); // request 14 registers
    
    // Lecture des données d'accélération et de gyroscope
    data->ax = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    data->ay = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    data->az = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    
    // Lecture de la température
    int16_t tempRaw = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    data->temperature = tempRaw / 340.0 + 36.53; // Formule de conversion pour MPU6050
    
    // Lecture des données du gyroscope
    data->gx = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    data->gy = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    data->gz = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    
    return true;
}

// Traitement des données brutes
void IMUSensor::processRawData(const RawIMUData& rawData, IMUData& processedData) {
    // Conversion des données brutes en données calibrées
    processedData.accel[0] = rawData.ax / ACCEL_SCALE;
    processedData.accel[1] = rawData.ay / ACCEL_SCALE;
    processedData.accel[2] = rawData.az / ACCEL_SCALE;
    
    processedData.gyro[0] = rawData.gx / GYRO_SCALE;
    processedData.gyro[1] = rawData.gy / GYRO_SCALE;
    processedData.gyro[2] = rawData.gz / GYRO_SCALE;
    
    // Calcul de l'orientation (simplifié)
    // Dans une implémentation réelle, on utiliserait un filtre de fusion de capteurs
    // comme un filtre de Kalman ou un filtre complémentaire
    processedData.orientation[0] = atan2(processedData.accel[1], processedData.accel[2]) * 180.0 / PI; // pitch
    processedData.orientation[1] = atan2(-processedData.accel[0], sqrt(processedData.accel[1] * processedData.accel[1] + processedData.accel[2] * processedData.accel[2])) * 180.0 / PI; // roll
    processedData.orientation[2] = 0; // yaw (nécessite un magnétomètre)
    
    // Quaternion simplifié (identité)
    processedData.quaternion[0] = 1.0f; // w
    processedData.quaternion[1] = 0.0f; // x
    processedData.quaternion[2] = 0.0f; // y
    processedData.quaternion[3] = 0.0f; // z
    
    processedData.timestamp = millis();
    processedData.dataValid = true;
}

// Instanciation globale et enregistrement
static IMUSensor imuSensor;
REGISTER_MODULE(imuSensor, &imuSensor);

//=============================================================================
// Fonctions C pour compatibilité avec le code existant
//=============================================================================

bool imuInit(const IMUConfig* config) {
    if (_imuSensorInstance == nullptr) {
        return false;
    }
    return _imuSensorInstance->init(config);
}

bool imuCalibrate() {
    if (_imuSensorInstance == nullptr) {
        return false;
    }
    return _imuSensorInstance->calibrate() == IMU_CALIBRATED;
}

bool imuReadRawData(RawIMUData* data) {
    if (_imuSensorInstance == nullptr) {
        return false;
    }
    return _imuSensorInstance->readRawData(data);
}

bool imuReadProcessedData(IMUData* data) {
    if (_imuSensorInstance == nullptr || data == nullptr) {
        return false;
    }
    *data = _imuSensorInstance->getData();
    return data->dataValid;
}

bool imuConfigure(const IMUConfig* config) {
    if (_imuSensorInstance == nullptr || config == nullptr) {
        return false;
    }
    return _imuSensorInstance->init(config);
}

bool imuSelfTest() {
    if (_imuSensorInstance == nullptr) {
        return false;
    }
    return _imuSensorInstance->selfTest();
}

const char* imuGetLastError() {
    if (_imuSensorInstance == nullptr) {
        return "IMU instance not available";
    }
    return _imuSensorInstance->description(); // Temporaire, devrait retourner le dernier message d'erreur
}

void imuSleep(bool enable) {
    if (_imuSensorInstance != nullptr) {
        _imuSensorInstance->sleep(enable);
    }
}
