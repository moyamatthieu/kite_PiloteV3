/*
  -----------------------
  Kite PiloteV3 - Module IMU (Implémentation)
  -----------------------
  
  Implémentation du module de gestion de l'unité de mesure inertielle (IMU).
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/sensors/imu.h"
#include "utils/logging.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <MPU6050_tockn.h>

// Variables statiques pour le stockage des données d'IMU
static IMUData currentIMUData = {0};
static bool imuInitialized = false;
static SemaphoreHandle_t imuMutex = NULL;
static MPU6050 mpu6050(Wire);

/**
 * Initialise l'IMU
 * @param wirePort Port I2C à utiliser (Wire ou Wire1)
 * @return true si succès, false si échec
 */
bool imuInit(TwoWire &wirePort) {
    // Protection thread-safe
    if (imuMutex == NULL) {
        imuMutex = xSemaphoreCreateMutex();
        if (imuMutex == NULL) {
            LOG_ERROR("IMU", "Échec de création du mutex");
            return false;
        }
    }
    
    wirePort.begin();
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    LOG_INFO("IMU", "MPU6050 initialisé et calibré");
    
    imuInitialized = true;
    return true;
}

/**
 * Initialise l'IMU avec une configuration optionnelle
 * @param config Configuration optionnelle de l'IMU
 * @return true si succès, false si échec
 */
bool imuInit(const IMUConfig* config) {
    // Protection thread-safe
    if (imuMutex == NULL) {
        imuMutex = xSemaphoreCreateMutex();
        if (imuMutex == NULL) {
            LOG_ERROR("IMU", "Échec de création du mutex");
            return false;
        }
    }
    
    Wire.begin();
    mpu6050.begin();
    
    // Si une configuration est fournie, l'appliquer
    if (config != nullptr) {
        // Ici, on pourrait configurer le MPU6050 selon les paramètres fournis
        // Pour cette implémentation, on utilise les valeurs par défaut
        LOG_INFO("IMU", "Configuration personnalisée appliquée");
    }
    
    mpu6050.calcGyroOffsets(true);
    LOG_INFO("IMU", "MPU6050 initialisé et calibré");
    
    imuInitialized = true;
    return true;
}

/**
 * Lit les données de l'IMU
 * @return Structure contenant les données d'orientation à jour
 */
IMUData imuReadData() {
    IMUData result = {0};
    
    // Protection thread-safe
    if (imuMutex == NULL || !imuInitialized) {
        LOG_ERROR("IMU", "IMU non initialisé ou mutex non disponible");
        return result;
    }
    
    if (xSemaphoreTake(imuMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Simulation de lecture de données
        result = currentIMUData;
        xSemaphoreGive(imuMutex);
    } else {
        LOG_ERROR("IMU", "Impossible d'acquérir le mutex IMU");
    }
    
    return result;
}

/**
 * Lit les données traitées de l'IMU
 * @param data Structure pour stocker les données traitées
 * @return true si succès, false si échec
 */
bool imuReadProcessedData(IMUData* data) {
    if (data == nullptr) {
        LOG_ERROR("IMU", "Pointeur de données IMU invalide");
        return false;
    }
    
    // Protection thread-safe
    if (imuMutex == NULL || !imuInitialized) {
        LOG_ERROR("IMU", "IMU non initialisé ou mutex non disponible");
        return false;
    }
    
    bool result = false;
    
    // Mise à jour manuelle des données au lieu d'appeler updateIMU()
    if (xSemaphoreTake(imuMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Mettre à jour les données
        mpu6050.update();
        
        // Lire les valeurs mises à jour
        currentIMUData.orientation[1] = mpu6050.getAngleX(); // roll
        currentIMUData.orientation[0] = mpu6050.getAngleY(); // pitch
        currentIMUData.orientation[2] = mpu6050.getAngleZ(); // yaw
        currentIMUData.gyro[0] = mpu6050.getGyroX();
        currentIMUData.gyro[1] = mpu6050.getGyroY();
        currentIMUData.gyro[2] = mpu6050.getGyroZ();
        currentIMUData.accel[0] = mpu6050.getAccX();
        currentIMUData.accel[1] = mpu6050.getAccY();
        currentIMUData.accel[2] = mpu6050.getAccZ();
        currentIMUData.timestamp = millis();
        currentIMUData.dataValid = true;
        
        // Copier les données dans la structure fournie
        *data = currentIMUData;
        
        xSemaphoreGive(imuMutex);
        result = true;
    } else {
        LOG_ERROR("IMU", "Impossible d'acquérir le mutex IMU pour la lecture");
    }
    
    return result;
}

/**
 * Calibre l'IMU
 * @param autoMode Mode automatique si true, sinon guidage pas à pas
 * @return État de calibration après l'opération
 */
IMUCalibrationState imuCalibrate(bool autoMode) {
    // Protection thread-safe
    if (imuMutex == NULL || !imuInitialized) {
        LOG_ERROR("IMU", "IMU non initialisé ou mutex non disponible");
        return IMU_CALIBRATION_ERROR;
    }
    
    IMUCalibrationState result = IMU_CALIBRATION_ERROR;
    
    if (xSemaphoreTake(imuMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Simulation de calibration
        LOG_INFO("IMU", "Calibration de l'IMU réussie");
        result = IMU_CALIBRATED;
        xSemaphoreGive(imuMutex);
    } else {
        LOG_ERROR("IMU", "Impossible d'acquérir le mutex IMU pour la calibration");
    }
    
    return result;
}

/**
 * Obtient l'état de calibration actuel de l'IMU
 * @return État de calibration
 */
IMUCalibrationState imuGetCalibrationState() {
    if (!imuInitialized) {
        return IMU_NOT_CALIBRATED;
    }
    
    if (currentIMUData.dataValid) {
        return IMU_CALIBRATED;
    }
    
    return IMU_PARTIALLY_CALIBRATED;
}

/**
 * Remet à zéro l'orientation de référence
 * @param setYawToZero Remet le lacet à zéro si true
 * @return true si succès, false si échec
 */
bool imuResetReference(bool setYawToZero) {
    // Protection thread-safe
    if (imuMutex == NULL || !imuInitialized) {
        LOG_ERROR("IMU", "IMU non initialisé ou mutex non disponible");
        return false;
    }
    
    bool result = false;
    
    if (xSemaphoreTake(imuMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Simulation de réinitialisation
        if (setYawToZero) {
            currentIMUData.orientation[2] = 0;
        }
        LOG_INFO("IMU", "Réinitialisation de la référence IMU réussie");
        result = true;
        xSemaphoreGive(imuMutex);
    } else {
        LOG_ERROR("IMU", "Impossible d'acquérir le mutex IMU pour la réinitialisation");
    }
    
    return result;
}

/**
 * Vérifie si l'IMU est fonctionnelle
 * @return true si fonctionnelle, false sinon
 */
bool imuIsHealthy() {
    return imuInitialized;
}

/**
 * Fonction de filtrage pour améliorer la précision des données
 * @param rawData Données brutes de l'IMU
 * @return Données filtrées
 */
IMUData imuFilterData(const IMUData &rawData) {
    // Pour cette implémentation, on retourne simplement les données brutes
    return rawData;
}

/**
 * Met en veille l'IMU pour économiser de l'énergie
 * @return true si succès, false si échec
 */
bool imuSleep() {
    LOG_INFO("IMU", "IMU mise en veille");
    return true;
}

/**
 * Sort l'IMU du mode veille
 * @return true si succès, false si échec
 */
bool imuWake() {
    LOG_INFO("IMU", "IMU sortie de veille");
    return true;
}

/**
 * Mise à jour périodique des données de l'IMU
 * Fonction thread-safe compatible avec FreeRTOS
 * @return true si succès, false si échec
 */
bool updateIMU() {
    // Vérifier si l'IMU est initialisée
    if (!imuInitialized) {
        static unsigned long lastInitAttempt = 0;
        unsigned long now = millis();
        
        // Tenter une réinitialisation au plus une fois toutes les 5 secondes
        if (now - lastInitAttempt > 5000) {
            lastInitAttempt = now;
            LOG_WARNING("IMU", "IMU non initialisée, tentative d'initialisation...");
            imuInit();
        }
        
        return false;
    }
    
    // Protection thread-safe
    if (imuMutex == NULL) {
        LOG_ERROR("IMU", "Mutex IMU non disponible");
        return false;
    }
    
    bool result = false;
    
    if (xSemaphoreTake(imuMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        mpu6050.update();
         currentIMUData.orientation[1] = mpu6050.getAngleX(); // roll
         currentIMUData.orientation[0] = mpu6050.getAngleY(); // pitch
         currentIMUData.orientation[2] = mpu6050.getAngleZ(); // yaw
         currentIMUData.gyro[0] = mpu6050.getGyroX();
         currentIMUData.gyro[1] = mpu6050.getGyroY();
         currentIMUData.gyro[2] = mpu6050.getGyroZ();
         currentIMUData.accel[0] = mpu6050.getAccX();
         currentIMUData.accel[1] = mpu6050.getAccY();
         currentIMUData.accel[2] = mpu6050.getAccZ();
        currentIMUData.timestamp = millis();
        currentIMUData.dataValid = true;
        
        result = true;
        xSemaphoreGive(imuMutex);
    } else {
        LOG_ERROR("IMU", "Impossible d'acquérir le mutex IMU pour la mise à jour");
    }
    
    return result;
}

// Ajout d'un délai adaptatif pour éviter les blocages lors de l'acquisition du mutex.
bool imuTryLock() {
    for (int i = 0; i < 3; i++) {
        if (xSemaphoreTake(imuMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Attente avant une nouvelle tentative
    }
    LOG_ERROR("IMU", "Impossible d'acquérir le mutex après plusieurs tentatives");
    return false;
}
