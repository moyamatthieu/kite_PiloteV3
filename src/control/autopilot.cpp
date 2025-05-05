/*
  -----------------------
  Kite PiloteV3 - Module Autopilote (Implémentation)
  -----------------------
  
  Implémentation du module d'autopilote pour le contrôle automatique du kite.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../../include/control/autopilot.h"
#include "../../include/utils/logging.h"
#include "../../include/control/trajectory.h"
#include "../../include/control/safety.h"
#include "../../include/hardware/actuators/servos.h"
#include "../../include/hardware/sensors/wind.h"
#include <cmath> // Pour sqrt et autres fonctions mathématiques

// Variables statiques
static AutopilotParameters autopilotParams;
static AutopilotState autopilotState;
static bool isInitialized = false;
static unsigned long lastUpdateTime = 0;
static unsigned long flightStartTime = 0;

// Définition des valeurs par défaut
static const AutopilotParameters DEFAULT_PARAMS = {
  .figure8Width = 60,         // 60 degrés de largeur
  .figure8Height = 30,        // 30 degrés de hauteur
  .turnSpeed = 5,             // Vitesse moyenne (5/10)
  .aggressiveness = 4,        // Agressivité modérée (4/10)
  .windAdaptation = 7,        // Bonne adaptation au vent (7/10)
  .safetyEnabled = true,      // Sécurité activée par défaut
  .maxAltitude = 100,         // 100 mètres maximum
  .maxLineLength = 15000,     // 150 mètres maximum
  .maxWindSpeed = 40          // 40 km/h maximum
};

// === FONCTIONS PRIVÉES ===

// Calculer la trajectoire en fonction du mode actuel
static void calculateTrajectory();

// Vérifier les conditions de sécurité
static bool checkSafetyConditions();

// Mettre à jour le niveau de confiance
static void updateConfidence(const IMUData& imuData);

// Mettre à jour l'état de l'autopilote
static void updateAutopilotState();

// === IMPLÉMENTATION DES FONCTIONS PUBLIQUES ===

bool autopilotInit() {
  if (isInitialized) {
    return true;
  }
  
  // Initialiser les paramètres avec les valeurs par défaut
  autopilotParams = DEFAULT_PARAMS;
  
  // Initialiser l'état de l'autopilote
  memset(&autopilotState, 0, sizeof(AutopilotState));
  autopilotState.currentMode = AUTOPILOT_OFF;
  autopilotState.confidence = 100;
  autopilotState.isStable = true;
  strncpy(autopilotState.statusMessage, "Autopilote initialisé", sizeof(autopilotState.statusMessage) - 1);
  
  // Initialiser les valeurs de position
  for (int i = 0; i < 3; i++) {
    autopilotState.currentPosition[i] = 0;
    autopilotState.targetPosition[i] = 0;
    for (int j = 0; j < 10; j++) {
      autopilotState.trajectory[j][i] = 0;
    }
  }
  
  // Marquer comme initialisé
  isInitialized = true;
  
  LOG_INFO("APLT", "Autopilote initialisé avec succès");
  return true;
}

bool setAutopilotMode(AutopilotMode mode) {
  if (!isInitialized) {
    LOG_ERROR("APLT", "Tentative de changement de mode sans initialisation");
    return false;
  }
  
  // Vérifier si le mode est valide
  if (mode < AUTOPILOT_OFF || mode > AUTOPILOT_CALIBRATION) {
    LOG_ERROR("APLT", "Mode d'autopilote invalide: %d", mode);
    return false;
  }
  
  // Vérifier les conditions pour activer certains modes
  if (mode != AUTOPILOT_OFF && mode != AUTOPILOT_EMERGENCY) {
    if (!checkSafetyConditions()) {
      LOG_WARNING("APLT", "Conditions de sécurité non remplies pour le mode %d", mode);
      return false;
    }
  }
  
  // Si on active l'autopilote à partir du mode OFF, enregistrer le temps de début
  if (autopilotState.currentMode == AUTOPILOT_OFF && mode != AUTOPILOT_OFF) {
    flightStartTime = millis() / 1000;
  }
  
  // Enregistrer le mode précédent et définir le nouveau mode
  AutopilotMode previousMode = autopilotState.currentMode;
  autopilotState.currentMode = mode;
  
  // Mettre à jour le message de statut
  switch (mode) {
    case AUTOPILOT_OFF:
      strncpy(autopilotState.statusMessage, "Mode manuel activé", sizeof(autopilotState.statusMessage) - 1);
      break;
    case AUTOPILOT_FIGURE_8:
      strncpy(autopilotState.statusMessage, "Mode figure en 8 activé", sizeof(autopilotState.statusMessage) - 1);
      break;
    case AUTOPILOT_HOVER:
      strncpy(autopilotState.statusMessage, "Mode stationnaire activé", sizeof(autopilotState.statusMessage) - 1);
      break;
    case AUTOPILOT_LANDING:
      strncpy(autopilotState.statusMessage, "Procédure d'atterrissage initiée", sizeof(autopilotState.statusMessage) - 1);
      break;
    case AUTOPILOT_TAKEOFF:
      strncpy(autopilotState.statusMessage, "Procédure de décollage initiée", sizeof(autopilotState.statusMessage) - 1);
      break;
    case AUTOPILOT_EMERGENCY:
      strncpy(autopilotState.statusMessage, "MODE URGENCE ACTIVÉ", sizeof(autopilotState.statusMessage) - 1);
      break;
    case AUTOPILOT_CALIBRATION:
      strncpy(autopilotState.statusMessage, "Mode calibration activé", sizeof(autopilotState.statusMessage) - 1);
      break;
  }
  
  LOG_INFO("APLT", "Mode changé: %d -> %d", previousMode, mode);
  return true;
}

AutopilotMode getAutopilotMode() {
  return autopilotState.currentMode;
}

void autopilotUpdate(const IMUData& imuData) {
  if (!isInitialized) {
    return;
  }
  
  // Mettre à jour le temps entre les appels
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastUpdateTime;
  
  // Ne pas mettre à jour trop fréquemment
  if (deltaTime < 50) { // Maximum 20 Hz
    return;
  }
  
  // Mettre à jour le temps de vol
  if (autopilotState.currentMode != AUTOPILOT_OFF) {
    autopilotState.flightTimeSeconds = (currentTime / 1000) - flightStartTime;
  }
  
  // Mettre à jour le niveau de confiance
  updateConfidence(imuData);
  
  // Vérifier les conditions de sécurité en mode actif
  if (autopilotState.currentMode != AUTOPILOT_OFF && autopilotState.currentMode != AUTOPILOT_EMERGENCY) {
    if (!checkSafetyConditions()) {
      LOG_WARNING("APLT", "Conditions de sécurité non remplies, activation du mode urgence");
      setAutopilotMode(AUTOPILOT_EMERGENCY);
    }
  }
  
  // Calculer la trajectoire si l'autopilote est actif
  if (autopilotState.currentMode != AUTOPILOT_OFF) {
    calculateTrajectory();
  }
  
  // Mettre à jour l'état de l'autopilote
  updateAutopilotState();
  
  // Enregistrer le temps de mise à jour
  lastUpdateTime = currentTime;
}

bool setAutopilotParameters(const AutopilotParameters& params) {
  if (!isInitialized) {
    LOG_ERROR("APLT", "Tentative de définition des paramètres sans initialisation");
    return false;
  }
  
  // Valider les paramètres
  if (params.figure8Width < 10 || params.figure8Width > 90) {
    LOG_ERROR("APLT", "Largeur de figure en 8 invalide: %d", params.figure8Width);
    return false;
  }
  
  if (params.figure8Height < 10 || params.figure8Height > 45) {
    LOG_ERROR("APLT", "Hauteur de figure en 8 invalide: %d", params.figure8Height);
    return false;
  }
  
  if (params.turnSpeed < 1 || params.turnSpeed > 10) {
    LOG_ERROR("APLT", "Vitesse de virage invalide: %d", params.turnSpeed);
    return false;
  }
  
  if (params.aggressiveness < 1 || params.aggressiveness > 10) {
    LOG_ERROR("APLT", "Agressivité invalide: %d", params.aggressiveness);
    return false;
  }
  
  if (params.windAdaptation < 1 || params.windAdaptation > 10) {
    LOG_ERROR("APLT", "Adaptation au vent invalide: %d", params.windAdaptation);
    return false;
  }
  
  if (params.maxAltitude < 10 || params.maxAltitude > 500) {
    LOG_ERROR("APLT", "Altitude maximale invalide: %d", params.maxAltitude);
    return false;
  }
  
  if (params.maxLineLength < 1000 || params.maxLineLength > 30000) {
    LOG_ERROR("APLT", "Longueur maximale de ligne invalide: %d", params.maxLineLength);
    return false;
  }
  
  if (params.maxWindSpeed < 10 || params.maxWindSpeed > 60) {
    LOG_ERROR("APLT", "Vitesse de vent maximale invalide: %d", params.maxWindSpeed);
    return false;
  }
  
  // Mettre à jour les paramètres
  autopilotParams = params;
  
  LOG_INFO("APLT", "Paramètres d'autopilote mis à jour");
  return true;
}

AutopilotParameters getAutopilotParameters() {
  return autopilotParams;
}

AutopilotState getAutopilotState() {
  return autopilotState;
}

bool calibrateAutopilot() {
  if (!isInitialized) {
    LOG_ERROR("APLT", "Tentative de calibration sans initialisation");
    return false;
  }
  
  // Passer en mode calibration
  if (!setAutopilotMode(AUTOPILOT_CALIBRATION)) {
    LOG_ERROR("APLT", "Impossible de passer en mode calibration");
    return false;
  }
  
  // Code de calibration ici
  LOG_INFO("APLT", "Calibration de l'autopilote en cours...");
  delay(2000); // Simulation de la calibration
  
  // Réinitialiser les paramètres à leurs valeurs par défaut
  autopilotParams = DEFAULT_PARAMS;
  
  // Revenir au mode OFF après la calibration
  setAutopilotMode(AUTOPILOT_OFF);
  
  LOG_INFO("APLT", "Calibration de l'autopilote terminée");
  return true;
}

void autopilotEmergencyStop() {
  if (!isInitialized) {
    return;
  }
  
  // Passer en mode urgence
  setAutopilotMode(AUTOPILOT_EMERGENCY);
  
  // Actions d'urgence ici
  LOG_WARNING("APLT", "ARRÊT D'URGENCE ACTIVÉ");
  
  // Réinitialiser les servomoteurs à une position sûre
  // Code pour sécuriser le kite
}

bool isAutopilotActive() {
  return (autopilotState.currentMode != AUTOPILOT_OFF);
}

uint8_t getAutopilotConfidence() {
  return autopilotState.confidence;
}

// === IMPLÉMENTATION DES FONCTIONS PRIVÉES ===

static void calculateTrajectory() {
  // Cette fonction calculerait la trajectoire optimale en fonction du mode actuel
  // Pour l'instant, c'est une version simplifiée
  
  switch (autopilotState.currentMode) {
    case AUTOPILOT_FIGURE_8:
      // Calculer une trajectoire en 8
      // Code pour générer la trajectoire en 8 ici
      break;
      
    case AUTOPILOT_HOVER:
      // Maintenir une position stable
      // Définir la même position cible que la position actuelle
      for (int i = 0; i < 3; i++) {
        autopilotState.targetPosition[i] = autopilotState.currentPosition[i];
      }
      break;
      
    case AUTOPILOT_LANDING:
      // Calculer une trajectoire d'atterrissage
      // Réduire progressivement l'altitude
      autopilotState.targetPosition[2] = 0; // Z = 0 pour l'atterrissage
      break;
      
    case AUTOPILOT_TAKEOFF:
      // Calculer une trajectoire de décollage
      // Augmenter progressivement l'altitude
      autopilotState.targetPosition[2] = 30; // Z = 30m pour le décollage
      break;
      
    case AUTOPILOT_EMERGENCY:
      // Trajectoire d'urgence pour sécuriser le kite
      // Typiquement une descente contrôlée
      autopilotState.targetPosition[2] = 0; // Z = 0 pour descendre
      break;
      
    case AUTOPILOT_CALIBRATION:
      // Pas de trajectoire en mode calibration
      break;
      
    default:
      break;
  }
}

static bool checkSafetyConditions() {
  // Vérifier toutes les conditions de sécurité
  
  // Condition 1: La sécurité est activée
  if (!autopilotParams.safetyEnabled) {
    return true; // Si la sécurité est désactivée, toujours autoriser
  }
  
  // Condition 2: L'altitude est dans les limites
  float currentAltitude = autopilotState.currentPosition[2];
  if (currentAltitude > autopilotParams.maxAltitude) {
    LOG_WARNING("APLT", "Altitude trop élevée: %.1f m (max: %d m)", 
                currentAltitude, autopilotParams.maxAltitude);
    return false;
  }
  
  // Condition 3: La longueur des lignes est dans les limites
  // Nécessite des capteurs supplémentaires
  
  // Condition 4: La vitesse du vent est dans les limites
  // Nécessite des capteurs supplémentaires
  
  // Toutes les conditions sont remplies
  return true;
}

static void updateConfidence(const IMUData& imuData) {
  // Cette fonction calculerait le niveau de confiance basé sur différents facteurs
  // Pour l'instant, c'est une version simplifiée
  
  // Facteur 1: Stabilité des données IMU
  float gyroMagnitude = std::sqrt(std::pow(imuData.rollRate, 2) + std::pow(imuData.pitchRate, 2) + std::pow(imuData.yawRate, 2));
  
  // Réduire la confiance si les gyroscopes montrent trop de mouvement
  if (gyroMagnitude > 200) {
    autopilotState.confidence = std::max(30, autopilotState.confidence - 5);
  } else {
    // Augmenter lentement la confiance si tout est stable
    autopilotState.confidence = std::min(100, autopilotState.confidence + 1);
  }
  
  // Autres facteurs de confiance pourraient être ajoutés ici
}

static void updateAutopilotState() {
  // Mettre à jour l'état de l'autopilote
  
  // Vérifier la stabilité basée sur le niveau de confiance
  autopilotState.isStable = (autopilotState.confidence > 70);
  
  // Mettre à jour les points de trajectoire historiques
  // Décaler tous les points d'une position
  for (int i = 9; i > 0; i--) {
    for (int j = 0; j < 3; j++) {
      autopilotState.trajectory[i][j] = autopilotState.trajectory[i-1][j];
    }
  }
  
  // Ajouter la position actuelle au début
  for (int j = 0; j < 3; j++) {
    autopilotState.trajectory[0][j] = autopilotState.currentPosition[j];
  }
}