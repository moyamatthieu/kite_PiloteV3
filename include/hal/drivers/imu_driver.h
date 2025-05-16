#pragma once

#include "hal/hal_component.h"
#include "common/global_enums.h"
#include <Wire.h> // Nécessaire pour la communication I2C

// Constantes spécifiques à l'IMU (MPU6050)
#define IMU_DEFAULT_I2C_ADDR 0x68
#define IMU_CALIBRATION_SAMPLES 1000

// Structure pour les données brutes de l'IMU
struct RawIMUData {
    int16_t ax, ay, az; // Accélération
    int16_t gx, gy, gz; // Gyroscope
    float temperature;  // Température
};

// Structure pour les données traitées de l'IMU
struct ProcessedIMUData {
    float accel[3];      // Accélération [x, y, z] en g
    float gyro[3];       // Vitesse angulaire [x, y, z] en deg/s
    float orientation[3]; // Orientation [pitch, roll, yaw] en degrés (calculée)
    float quaternion[4];  // Quaternion d'orientation [w, x, y, z] (si disponible/calculé)
    uint32_t timestamp;  // Horodatage de la mesure
    bool isValid;        // Indicateur de validité des données
};

// Structure pour la configuration de l'IMU
struct IMUDriverConfig {
    uint8_t i2cAddress;
    uint8_t gyroRange;      // Ex: 0 pour ±250dps, 1 pour ±500dps, ...
    uint8_t accelRange;     // Ex: 0 pour ±2g, 1 pour ±4g, ...
    uint8_t dlpfBandwidth;  // Digital Low Pass Filter bandwidth
    uint8_t sampleRateDiv;  // Diviseur du taux d'échantillonnage

    IMUDriverConfig() :
        i2cAddress(IMU_DEFAULT_I2C_ADDR),
        gyroRange(0),
        accelRange(0),
        dlpfBandwidth(0),
        sampleRateDiv(0) {}
};

class IMUDriver : public SensorComponent {
public:
    IMUDriver(const char* name = "IMUDriver");
    ~IMUDriver() override = default;

    ErrorCode initialize() override;
    bool read() override; // Implémentation de la méthode virtuelle pure
    ErrorCode configure(const IMUDriverConfig& config); // Surcharge pour une config typée

    // Méthodes spécifiques à l'IMU
    ErrorCode calibrate();
    bool selfTest();
    void sleep(bool enable);
    void readSensor(); // Ajout pour correspondance avec l'implémentation

    const ProcessedIMUData& getProcessedData() const { return _processedData; }
    const RawIMUData& getRawData() const { return _rawData; }
    ComponentState getCalibrationState() const { return _calibrationState; }

    // Méthodes d'accès aux données spécifiques
    bool dataAvailable() const;
    float getPitch() const;
    float getRoll() const;
    float getYaw() const;
    float getAccelX() const;
    float getAccelY() const;
    float getAccelZ() const;
    float getGyroX() const;
    float getGyroY() const;
    float getGyroZ() const;
    float getTemperature() const;

protected:
    void onEnable() override;
    void onDisable() override;

private:
    ErrorCode applyConfig();
    ErrorCode readRawValues(RawIMUData& data);
    void processRawValues(const RawIMUData& raw, ProcessedIMUData& processed);
    void calculateOrientation(const ProcessedIMUData& imuData, float& pitch, float& roll, float& yaw); // Méthode d'aide
    float getGyroScaleFactor() const;  // Nouveau
    float getAccelScaleFactor() const; // Nouveau

    IMUDriverConfig _config;
    RawIMUData _rawData;
    ProcessedIMUData _processedData;
    ComponentState _calibrationState; // Utilise ComponentState pour la calibration aussi

    // Variables pour la calibration
    float _gyroBias[3] = {0.0f, 0.0f, 0.0f};
    float _accelBias[3] = {0.0f, 0.0f, 0.0f};
};
