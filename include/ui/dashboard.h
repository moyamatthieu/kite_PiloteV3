/*
  -----------------------
  Kite PiloteV3 - Module Dashboard (Interface)
  -----------------------
  
  Interface pour le tableau de bord du système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <Arduino.h>
#include "core/config.h"
#include "control/autopilot.h"
#include "hal/drivers/imu_driver.h"

// === DÉFINITION DES TYPES ===

// Structure pour les données du tableau de bord
typedef struct {
  // Données système
  uint32_t uptime;                // Temps depuis le démarrage (secondes)
  uint32_t freeHeap;              // Mémoire libre (octets)
  uint8_t cpuUsage;               // Utilisation CPU (%)
  float cpuTemperature;           // Température CPU (°C)
  
  // Données kite
  float kiteAltitude;             // Altitude estimée du kite (mètres)
  float kiteSpeed;                // Vitesse estimée du kite (m/s)
  float kitePower;                // Puissance estimée du kite (watts)
  
  // Données vent
  float windSpeed;                // Vitesse du vent (m/s)
  float windDirection;            // Direction du vent (degrés)
  float windVariation;            // Variation du vent (%)
  
  // Données de contrôle
  int16_t directionAngle;         // Angle de direction (-90 à 90)
  int16_t trimAngle;              // Angle de trim (-45 à 45)
  uint16_t lineLength;            // Longueur des lignes (cm)
  float lineTension;              // Tension des lignes (kg)
  
  // Données de performance
  uint32_t totalEnergy;           // Énergie totale générée (watt-heures)
  float currentPower;             // Puissance instantanée (watts)
  float peakPower;                // Puissance maximale atteinte (watts)
  float efficiency;               // Efficacité (0.0-1.0)
  
  // État autopilote
  AutopilotMode autopilotMode;    // Mode de l'autopilote
  uint8_t autopilotConfidence;    // Confiance de l'autopilote (0-100)
  
  // État général
  uint8_t systemStatus;           // État général (0-100)
  char statusMessage[64];         // Message d'état
  bool hasWarning;                // Présence d'avertissement
  bool hasError;                  // Présence d'erreur
} DashboardData;

// Type de mise à jour du tableau de bord
typedef enum {
  DASH_UPDATE_FULL = 0,           // Mise à jour complète
  DASH_UPDATE_SYSTEM = 1,         // Mise à jour système uniquement
  DASH_UPDATE_KITE = 2,           // Mise à jour kite uniquement
  DASH_UPDATE_CONTROL = 3,        // Mise à jour contrôle uniquement
  DASH_UPDATE_PERFORMANCE = 4,    // Mise à jour performance uniquement
  DASH_UPDATE_STATUS = 5          // Mise à jour statut uniquement
} DashboardUpdateType;

// === DÉCLARATION DES FONCTIONS ===

/**
 * Initialise le module de tableau de bord
 * @return true si succès, false si échec
 */
bool dashboardInit();

/**
 * Met à jour les données du tableau de bord
 * @param updateType Type de mise à jour à effectuer
 * @return true si la mise à jour a réussi, false sinon
 */
bool dashboardUpdate(DashboardUpdateType updateType = DASH_UPDATE_FULL);

/**
 * Obtient les données actuelles du tableau de bord
 * @return Structure contenant les données du tableau de bord
 */
DashboardData dashboardGetData();

/**
 * Met à jour les données système dans le tableau de bord
 * @param uptime Temps depuis le démarrage (secondes)
 * @param freeHeap Mémoire libre (octets)
 * @param cpuUsage Utilisation CPU (%)
 * @param cpuTemp Température CPU (°C)
 * @return true si succès, false si échec
 */
bool dashboardUpdateSystem(uint32_t uptime, uint32_t freeHeap, uint8_t cpuUsage, float cpuTemp);

/**
 * Met à jour les données du kite dans le tableau de bord
 * @param imuData Données de l'IMU
 * @return true si succès, false si échec
 */
bool dashboardUpdateKite(const ProcessedIMUData& imuData);

/**
 * Met à jour les données de contrôle dans le tableau de bord
 * @param directionAngle Angle de direction (-90 à 90)
 * @param trimAngle Angle de trim (-45 à 45)
 * @param lineLength Longueur des lignes (cm)
 * @param lineTension Tension des lignes (kg)
 * @return true si succès, false si échec
 */
bool dashboardUpdateControl(int16_t directionAngle, int16_t trimAngle, 
                           uint16_t lineLength, float lineTension);

/**
 * Met à jour les données de performance dans le tableau de bord
 * @param currentPower Puissance instantanée (watts)
 * @param totalEnergy Énergie totale (watt-heures)
 * @param efficiency Efficacité (0.0-1.0)
 * @return true si succès, false si échec
 */
bool dashboardUpdatePerformance(float currentPower, uint32_t totalEnergy, float efficiency);

/**
 * Met à jour l'état de l'autopilote dans le tableau de bord
 * @param mode Mode de l'autopilote
 * @param confidence Confiance de l'autopilote (0-100)
 * @return true si succès, false si échec
 */
bool dashboardUpdateAutopilot(AutopilotMode mode, uint8_t confidence);

/**
 * Met à jour l'état général dans le tableau de bord
 * @param status État général (0-100)
 * @param message Message d'état
 * @param hasWarning Présence d'avertissement
 * @param hasError Présence d'erreur
 * @return true si succès, false si échec
 */
bool dashboardUpdateStatus(uint8_t status, const char* message, 
                         bool hasWarning = false, bool hasError = false);

/**
 * Génère une représentation JSON des données du tableau de bord
 * @param updateType Type de mise à jour à inclure
 * @return Chaîne JSON des données
 */
String dashboardToJson(DashboardUpdateType updateType = DASH_UPDATE_FULL);

#endif // DASHBOARD_H