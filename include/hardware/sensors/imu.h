/*
  -----------------------
  Kite PiloteV3 - Module IMU (Interface)
  -----------------------
  
  Interface pour la gestion de l'unité de mesure inertielle (IMU) du kite.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h> // Include for TwoWire and Wire object
#include "../../core/config.h"

// === DÉFINITION DES TYPES ===

// Structure pour les données d'orientation
typedef struct {
  float roll;                   // Angle de roulis (degrés, -180 à 180)
  float pitch;                  // Angle de tangage (degrés, -90 à 90)
  float yaw;                    // Angle de lacet (degrés, 0 à 360)
  float rollRate;               // Vitesse de roulis (degrés/s)
  float pitchRate;              // Vitesse de tangage (degrés/s)
  float yawRate;                // Vitesse de lacet (degrés/s)
  int16_t accelX;               // Accélération X (brute)
  int16_t accelY;               // Accélération Y (brute)
  int16_t accelZ;               // Accélération Z (brute)
  uint32_t timestamp;           // Horodatage (ms)
  bool isCalibrated;            // État de calibration
  uint8_t accuracy;             // Précision (0-3)
} IMUData;

// États de calibration IMU
typedef enum {
  IMU_NOT_CALIBRATED = 0,       // Pas calibré
  IMU_PARTIALLY_CALIBRATED = 1, // Partiellement calibré
  IMU_CALIBRATED = 2,           // Complètement calibré
  IMU_CALIBRATION_ERROR = 3     // Erreur de calibration
} IMUCalibrationState;

// === DÉCLARATION DES FONCTIONS ===

/**
 * Initialise l'IMU
 * @param wirePort Port I2C à utiliser (Wire ou Wire1)
 * @return true si succès, false si échec
 */
bool imuInit(TwoWire &wirePort = Wire);

/**
 * Lit les données de l'IMU
 * @return Structure contenant les données d'orientation à jour
 */
IMUData imuReadData();

/**
 * Calibre l'IMU
 * @param autoMode Mode automatique si true, sinon guidage pas à pas
 * @return État de calibration après l'opération
 */
IMUCalibrationState imuCalibrate(bool autoMode = true);

/**
 * Obtient l'état de calibration actuel de l'IMU
 * @return État de calibration
 */
IMUCalibrationState imuGetCalibrationState();

/**
 * Remet à zéro l'orientation de référence
 * @param setYawToZero Remet le lacet à zéro si true
 * @return true si succès, false si échec
 */
bool imuResetReference(bool setYawToZero = true);

/**
 * Vérifie si l'IMU est fonctionnelle
 * @return true si fonctionnelle, false sinon
 */
bool imuIsHealthy();

/**
 * Fonction de filtrage pour améliorer la précision des données
 * @param rawData Données brutes de l'IMU
 * @return Données filtrées
 */
IMUData imuFilterData(const IMUData &rawData);

/**
 * Met en veille l'IMU pour économiser de l'énergie
 * @return true si succès, false si échec
 */
bool imuSleep();

/**
 * Sort l'IMU du mode veille
 * @return true si succès, false si échec
 */
bool imuWake();

#endif // IMU_H