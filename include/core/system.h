/*
  -----------------------
  Kite PiloteV3 - Module système (Définitions)
  -----------------------
  
  Définitions des fonctions système de base pour Kite PiloteV3.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include "config.h"

// === CONSTANTES ===

// États système
#define SYS_STATE_INIT            0
#define SYS_STATE_READY           1
#define SYS_STATE_RUNNING         2
#define SYS_STATE_ERROR           3
#define SYS_STATE_POWER_SAVE      4
#define SYS_STATE_UPDATE          5
#define SYS_STATE_CALIBRATION     6

#define SYS_OK 0

// === TYPES DE DONNÉES ===

// Structure pour les informations système
typedef struct {
  uint8_t systemState;            // État actuel du système
  uint32_t uptimeSeconds;         // Temps écoulé depuis le démarrage (secondes)
  uint32_t freeHeapBytes;         // Mémoire heap disponible en octets
  uint8_t cpuUsagePercent;        // Utilisation CPU en pourcentage
  float cpuTemperature;           // Température CPU en degrés Celsius
  uint16_t batteryVoltage;        // Tension de la batterie en mV
  uint8_t batteryPercent;         // Niveau de batterie en pourcentage
  bool hasError;                  // Indicateur d'erreur
  bool hasWarning;                // Indicateur d'avertissement
  char lastErrorMessage[64];      // Dernier message d'erreur
} SystemInfo;

typedef int SystemErrorCode;

// === FONCTIONS PUBLIQUES ===

// Initialiser le système
bool systemInit();

// Obtenir les informations système actuelles
SystemInfo getSystemInfo();

// Mettre à jour les informations système
void updateSystemInfo();

// Redémarrer le système avec un délai facultatif
void systemRestart(unsigned long delayMs = 0);

// Vérifier la santé du système
bool systemHealthCheck();

// Entrer en mode économie d'énergie
void enterPowerSaveMode();

// Quitter le mode économie d'énergie
void exitPowerSaveMode();

// Obtenir la version du système
const char* getSystemVersion();

// Vérifier si le système est en état d'erreur
bool hasSystemError();

// Effacer les erreurs système
void clearSystemErrors();

// Gérer une erreur système
void handleSystemError(const char* errorSource, const char* errorMessage);

// Vérifier si le système est en bonne santé
bool isSystemHealthy();

// Alimenter les watchdogs
void feedWatchdogs();

const char* systemErrorToString(SystemErrorCode errorCode);

#endif // SYSTEM_H