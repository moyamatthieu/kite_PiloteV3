/*
  -----------------------
  Module IMU (Inertial Measurement Unit)
  -----------------------
  
  Gestion du capteur inertiel pour mesurer l'orientation et les mouvements du kite.
  Utilise un MPU6050 pour les mesures d'accélération et de rotation.
*/

#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h> // Include for TwoWire and Wire object
#include "../../core/config.h"

// === CONSTANTES IMU ===
#define IMU_I2C_ADDR 0x68        // Adresse I2C par défaut du MPU6050
#define GYRO_SCALE 131.0         // Échelle du gyroscope (LSB/deg/s)
#define ACCEL_SCALE 16384.0      // Échelle de l'accéléromètre (LSB/g)
#define CALIBRATION_SAMPLES 1000  // Nombre d'échantillons pour la calibration

// === DÉFINITION DES TYPES ===

// Structure pour les données brutes de l'IMU
typedef struct {
    int16_t ax;                  // Accélération brute sur X
    int16_t ay;                  // Accélération brute sur Y
    int16_t az;                  // Accélération brute sur Z
    int16_t gx;                  // Vitesse angulaire brute autour de X
    int16_t gy;                  // Vitesse angulaire brute autour de Y
    int16_t gz;                  // Vitesse angulaire brute autour de Z
    float temperature;           // Température du capteur en °C
} RawIMUData;

// Structure pour les données calibrées de l'IMU
typedef struct {
    float accel[3];             // Accélérations calibrées [x, y, z] en g
    float gyro[3];              // Vitesses angulaires calibrées [x, y, z] en deg/s
    float orientation[3];        // Orientation [pitch, roll, yaw] en degrés
    float quaternion[4];        // Quaternion d'orientation [w, x, y, z]
    uint32_t timestamp;         // Horodatage de la mesure
    bool dataValid;             // Indicateur de validité des données
} IMUData;

// Structure pour la configuration de l'IMU
typedef struct {
    uint8_t gyroRange;          // Plage du gyroscope (±250/500/1000/2000 deg/s)
    uint8_t accelRange;         // Plage de l'accéléromètre (±2/4/8/16 g)
    uint8_t dlpfBandwidth;      // Bande passante du filtre passe-bas
    uint8_t sampleRate;         // Taux d'échantillonnage en Hz
    bool fifoEnabled;           // Activation du FIFO
    bool lowPowerMode;          // Mode basse consommation
} IMUConfig;

// États de calibration IMU
typedef enum {
  IMU_NOT_CALIBRATED = 0,       // Pas calibré
  IMU_PARTIALLY_CALIBRATED = 1, // Partiellement calibré
  IMU_CALIBRATED = 2,           // Complètement calibré
  IMU_CALIBRATION_ERROR = 3     // Erreur de calibration
} IMUCalibrationState;

// === PROTOTYPES DES FONCTIONS ===

/**
 * Initialise l'IMU
 * @param config Configuration optionnelle de l'IMU
 * @return true si succès, false si échec
 */
bool imuInit(const IMUConfig* config = nullptr);

/**
 * Calibre l'IMU
 * @return true si succès, false si échec
 */
bool imuCalibrate();

/**
 * Lit les données brutes de l'IMU
 * @param data Structure pour stocker les données brutes
 * @return true si succès, false si échec
 */
bool imuReadRawData(RawIMUData* data);

/**
 * Lit les données traitées de l'IMU
 * @param data Structure pour stocker les données traitées
 * @return true si succès, false si échec
 */
bool imuReadProcessedData(IMUData* data);

/**
 * Configure l'IMU
 * @param config Configuration à appliquer
 * @return true si succès, false si échec
 */
bool imuConfigure(const IMUConfig* config);

/**
 * Effectue un auto-test de l'IMU
 * @return true si succès, false si échec
 */
bool imuSelfTest();

/**
 * Obtient le dernier message d'erreur de l'IMU
 * @return Chaîne de caractères contenant l'erreur
 */
const char* imuGetLastError();

/**
 * Met en veille ou réveille l'IMU
 * @param enable true pour mettre en veille, false pour réveiller
 */
void imuSleep(bool enable);

#endif // IMU_H