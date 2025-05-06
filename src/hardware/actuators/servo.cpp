/*
  -----------------------
  Kite PiloteV3 - Module Servo (Implémentation)
  -----------------------
  
  Implémentation du module de contrôle des servomoteurs.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/actuators/servo.h"
#include "utils/logging.h"

// Variables statiques
static Servo directionServo;
static Servo trimServo;
static Servo lineModServo;
static bool servosInitialized = false;

/**
 * Initialise tous les servomoteurs
 * @return true si l'initialisation réussit, false sinon
 */
bool servoInitAll() {
  LOG_INFO("SERVO", "Initialisation des servomoteurs...");
  
  // Configurer les paramètres des servomoteurs
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  
  // Configurer les servomoteurs avec les paramètres min/max
  directionServo.setPeriodHertz(50);
  trimServo.setPeriodHertz(50);
  lineModServo.setPeriodHertz(50);
  
  // Attacher les servomoteurs aux broches
  if (!directionServo.attach(SERVO_DIRECTION_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE)) {
    LOG_ERROR("SERVO", "Échec d'initialisation du servo de direction");
    return false;
  }
  
  if (!trimServo.attach(SERVO_TRIM_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE)) {
    LOG_ERROR("SERVO", "Échec d'initialisation du servo de trim");
    directionServo.detach();
    return false;
  }
  
  if (!lineModServo.attach(SERVO_LINEMOD_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE)) {
    LOG_ERROR("SERVO", "Échec d'initialisation du servo de modulation de ligne");
    directionServo.detach();
    trimServo.detach();
    return false;
  }
  
  // Positionner les servomoteurs en position neutre
  directionServo.write(90);  // 90° = position neutre (0°)
  trimServo.write(90);       // 90° = position neutre (0°)
  lineModServo.write(0);     // Ligne complètement relâchée
  
  servosInitialized = true;
  LOG_INFO("SERVO", "Servomoteurs initialisés avec succès");
  
  return true;
}

/**
 * Déplace le servomoteur de direction
 * @param angle Angle de direction (-90 à +90)
 * @return true si le mouvement réussit, false sinon
 */
bool servoSetDirection(int angle) {
  if (!servosInitialized || !directionServo.attached()) {
    LOG_ERROR("SERVO", "Servo de direction non initialisé");
    return false;
  }
  
  // Limiter l'angle aux plages acceptables
  angle = constrain(angle, DIRECTION_MIN_ANGLE, DIRECTION_MAX_ANGLE);
  
  // Convertir l'angle de -90..90 à 0..180 pour le servo
  int servoAngle = map(angle, DIRECTION_MIN_ANGLE, DIRECTION_MAX_ANGLE, 0, 180);
  
  // Déplacer le servo
  directionServo.write(servoAngle);
  
  return true;
}

/**
 * Déplace le servomoteur de trim
 * @param angle Angle de trim (-45 à +45)
 * @return true si le mouvement réussit, false sinon
 */
bool servoSetTrim(int angle) {
  if (!servosInitialized || !trimServo.attached()) {
    LOG_ERROR("SERVO", "Servo de trim non initialisé");
    return false;
  }
  
  // Limiter l'angle aux plages acceptables
  angle = constrain(angle, TRIM_MIN_ANGLE, TRIM_MAX_ANGLE);
  
  // Convertir l'angle de -45..45 à 0..180 pour le servo
  int servoAngle = map(angle, TRIM_MIN_ANGLE, TRIM_MAX_ANGLE, 0, 180);
  
  // Déplacer le servo
  trimServo.write(servoAngle);
  
  return true;
}

/**
 * Contrôle le servomoteur de modulation de ligne
 * @param position Position du servomoteur (0 à 100)
 * @return true si le mouvement réussit, false sinon
 */
bool servoSetLineModulation(int position) {
  if (!servosInitialized || !lineModServo.attached()) {
    LOG_ERROR("SERVO", "Servo de modulation de ligne non initialisé");
    return false;
  }
  
  // Limiter la position aux plages acceptables
  position = constrain(position, 0, 100);
  
  // Convertir la position de 0..100 à 0..180 pour le servo
  int servoPosition = map(position, 0, 100, 0, 180);
  
  // Déplacer le servo
  lineModServo.write(servoPosition);
  
  return true;
}

/**
 * Détache tous les servomoteurs pour économiser de l'énergie
 */
void servoDetachAll() {
  if (directionServo.attached()) {
    directionServo.detach();
  }
  
  if (trimServo.attached()) {
    trimServo.detach();
  }
  
  if (lineModServo.attached()) {
    lineModServo.detach();
  }
  
  servosInitialized = false;
  LOG_INFO("SERVO", "Tous les servomoteurs ont été détachés");
}

/**
 * Vérifie si un servomoteur est fixé
 * @param servoIndex Index du servomoteur (0=direction, 1=trim, 2=linemod)
 * @return true si le servomoteur est fixé, false sinon
 */
bool servoIsAttached(uint8_t servoIndex) {
  switch (servoIndex) {
    case 0:
      return directionServo.attached();
    case 1:
      return trimServo.attached();
    case 2:
      return lineModServo.attached();
    default:
      return false;
  }
}