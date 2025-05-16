/*
  -----------------------
  Kite PiloteV3 - Module Autopilote (Définitions)
  -----------------------
  
  Définitions du module d'autopilote pour le contrôle automatique du kite.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <Arduino.h>
#include "hal/drivers/imu_driver.h"
#include "core/config.h"

// === CONSTANTES ===

// Modes de l'autopilote
typedef enum {
  AUTOPILOT_OFF = 0,           // Autopilote désactivé (mode manuel)
  AUTOPILOT_FIGURE_8 = 1,      // Mode figure en 8 (génération d'énergie)
  AUTOPILOT_HOVER = 2,         // Mode stationnaire (maintien de position)
  AUTOPILOT_LANDING = 3,       // Mode atterrissage (descente contrôlée)
  AUTOPILOT_TAKEOFF = 4,       // Mode décollage (montée contrôlée)
  AUTOPILOT_EMERGENCY = 5,     // Mode urgence (procédure de sécurité)
  AUTOPILOT_CALIBRATION = 6    // Mode calibration (pour le réglage des paramètres)
} AutopilotMode;

// Paramètres de l'autopilote
typedef struct {
  uint8_t figure8Width;        // Largeur de la figure en 8 (en degrés) [10-90]
  uint8_t figure8Height;       // Hauteur de la figure en 8 (en degrés) [10-45]
  uint8_t turnSpeed;           // Vitesse de virage [1-10]
  uint8_t aggressiveness;      // Agressivité des manœuvres [1-10]
  uint8_t windAdaptation;      // Adaptation aux conditions de vent [1-10]
  bool safetyEnabled;          // Activation des fonctions de sécurité
  uint16_t maxAltitude;        // Altitude maximale autorisée (en mètres)
  uint16_t maxLineLength;      // Longueur maximale des lignes (en cm)
  uint8_t maxWindSpeed;        // Vitesse de vent maximale autorisée (en km/h)
} AutopilotParameters;

// === DÉFINITION DES CONSTANTES DE CONTRÔLE ===
#define MIN_ANGLE -45        // Angle minimum en degrés pour le contrôle de direction
#define MAX_ANGLE 45         // Angle maximum en degrés pour le contrôle de direction
#define DEFAULT_SPEED 1.0    // Vitesse par défaut pour les mouvements du kite

// Structure pour les paramètres PID du pilote automatique
typedef struct {
    // Paramètres du contrôleur PID
    float Kp;               // Gain proportionnel - Réactivité immédiate aux erreurs
    float Ki;               // Gain intégral - Correction des erreurs persistantes
    float Kd;               // Gain dérivé - Anticipation des changements d'erreur
    
    // Limites de sécurité
    float maxOutput;        // Sortie maximale du contrôleur
    float minOutput;        // Sortie minimale du contrôleur
    float maxIntegral;      // Limite d'accumulation de l'erreur intégrale
    
    // Variables d'état
    float lastError;        // Dernière erreur mesurée pour calcul dérivé
    float integral;         // Accumulation des erreurs pour terme intégral
    float setpoint;        // Point de consigne désiré
} PIDParams;

// État de l'autopilote
typedef struct {
  AutopilotMode currentMode;    // Mode actuel
  uint8_t confidence;           // Niveau de confiance de l'autopilote [0-100]
  float targetPosition[3];      // Position cible [x, y, z]
  float currentPosition[3];     // Position actuelle estimée [x, y, z]
  float trajectory[10][3];      // Points de trajectoire récents
  bool isStable;                // Indicateur de stabilité
  uint32_t flightTimeSeconds;   // Temps de vol en secondes
  char statusMessage[64];       // Message d'état
  bool enabled;                 // État d'activation du pilote automatique
  float targetAngle;            // Angle cible pour le contrôle de direction
  float currentAngle;           // Angle actuel mesuré
  float windSpeed;              // Vitesse du vent actuelle
  float lineLength;             // Longueur actuelle des lignes
  PIDParams pidParams;          // Paramètres du contrôleur PID
} AutopilotState;

// === FONCTIONS PUBLIQUES ===

// Initialiser l'autopilote
bool autopilotInit();

// Définir le mode de l'autopilote
bool setAutopilotMode(AutopilotMode mode);

// Obtenir le mode actuel de l'autopilote
AutopilotMode getAutopilotMode();

// Mettre à jour l'autopilote avec les dernières données IMU
void autopilotUpdate(const ProcessedIMUData& imuData);

// Régler les paramètres de l'autopilote
bool setAutopilotParameters(const AutopilotParameters& params);

// Obtenir les paramètres actuels de l'autopilote
AutopilotParameters getAutopilotParameters();

// Obtenir l'état actuel de l'autopilote
AutopilotState getAutopilotState();

// Calibrer l'autopilote
bool calibrateAutopilot();

// Arrêt d'urgence
void autopilotEmergencyStop();

// Vérifier si l'autopilote est actif
bool isAutopilotActive();

// Obtenir le niveau de confiance de l'autopilote
uint8_t getAutopilotConfidence();

// Activation/désactivation du pilote automatique
void autopilotEnable(bool enable);

// Mise à jour des paramètres du contrôleur PID
void updatePIDParams(const PIDParams* params);

// Calcul de la commande de contrôle
float computeControlCommand(float currentAngle, float targetAngle);

// Mise à jour de l'état du pilote automatique
void updateAutopilotState(float windSpeed, float lineLength);

// Fonction de sécurité pour vérifier les limites
bool checkSafetyLimits();

#endif // AUTOPILOT_H