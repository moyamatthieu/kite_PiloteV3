/*
  -----------------------
  Module IMU (Inertial Measurement Unit)
  -----------------------
  
  Gestion du capteur inertiel pour mesurer l'orientation et les mouvements du kite.
  Utilise un MPU6050 pour les mesures d'accélération et de rotation.
  
  Version: 2.0.0
  Date: 9 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>
#include "../../core/config.h"
#include "../../core/module.h"
#include "../../utils/state_machine.h"
#include "../../utils/error_manager.h"
#include <string>

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

/**
 * Classe IMUSensor - Gestion du capteur IMU
 * Hérite de SensorModule pour s'intégrer dans l'architecture modulaire
 */
class IMUSensor : public SensorModule {
public:
    /**
     * Constructeur
     */
    IMUSensor();
    
    /**
     * Destructeur
     */
    virtual ~IMUSensor();
    
    /**
     * Initialise le capteur IMU
     * @param config Configuration optionnelle
     * @return true si succès, false si échec
     */
    bool init(const IMUConfig* config = nullptr);
    
    /**
     * Lit les données du capteur
     * Implémentation de la méthode virtuelle de SensorModule
     */
    void readSensor() override;
    
    /**
     * Configure le capteur IMU
     * @param jsonConfig Configuration au format JSON
     */
    void configure(const std::string& jsonConfig) override;
    
    /**
     * Calibre le capteur IMU
     * @return État de calibration
     */
    IMUCalibrationState calibrate();
    
    /**
     * Obtient les dernières données lues
     * @return Structure contenant les données IMU
     */
    const IMUData& getData() const;
    
    /**
     * Effectue un auto-test du capteur
     * @return true si succès, false si échec
     */
    bool selfTest();
    
    /**
     * Met en veille ou réveille le capteur
     * @param enable true pour mettre en veille, false pour réveiller
     */
    void sleep(bool enable);
    
    /**
     * Retourne une description du module
     * @return Chaîne de caractères décrivant le module
     */
    const char* description() const override;
    
    /**
     * Lit les données brutes du capteur
     * @param data Structure pour stocker les données
     * @return true si succès, false si échec
     */
    bool readRawData(RawIMUData* data);

private:
    IMUData _lastData;              // Dernières données lues
    IMUConfig _config;              // Configuration actuelle
    IMUCalibrationState _calState;  // État de calibration
    // États de la machine à états de l'IMU
    enum IMUState {
        IMU_STATE_IDLE = 0,
        IMU_STATE_INIT,
        IMU_STATE_READING,
        IMU_STATE_CALIBRATING,
        IMU_STATE_ERROR
    };
    
    // Classe interne pour la machine à états de l'IMU
    class IMUStateMachine : public StateMachine {
    public:
        IMUStateMachine(IMUSensor* parent)
            : StateMachine("IMU_FSM", IMU_STATE_IDLE), _parent(parent) {}
        
    protected:
        int processState(int state) override {
            // Implémentation simple pour éviter l'erreur de classe abstraite
            return state;
        }
        
    private:
        IMUSensor* _parent;
    };
    
    IMUStateMachine _fsm;           // Machine à états pour les opérations asynchrones
    char _lastError[64];            // Dernier message d'erreur
    
    
    /**
     * Traite les données brutes en données calibrées
     * @param rawData Données brutes
     * @param processedData Données traitées
     */
    void processRawData(const RawIMUData& rawData, IMUData& processedData);
};

// Fonctions C pour compatibilité avec le code existant
bool imuInit(const IMUConfig* config = nullptr);
bool imuCalibrate();
bool imuReadRawData(RawIMUData* data);
bool imuReadProcessedData(IMUData* data);
bool imuConfigure(const IMUConfig* config);
bool imuSelfTest();
const char* imuGetLastError();
void imuSleep(bool enable);

#endif // IMU_H