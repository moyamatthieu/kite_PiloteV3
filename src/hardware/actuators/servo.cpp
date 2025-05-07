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
static unsigned long lastServoUpdateTime = 0;

/**
 * Initialise tous les servomoteurs avec une stratégie plus robuste
 * @return true si l'initialisation réussit, false sinon
 */
bool servoInitAll() {
  LOG_INFO("SERVO", "Initialisation des servomoteurs...");
  
  // Réinitialiser l'état
  servosInitialized = false;
  
  // Détacher d'abord tous les servos pour réinitialiser l'état interne
  servoDetachAll();
  
  // Délai important pour permettre au matériel de se stabiliser
  delay(200);
  
  // Configuration des timers - utilisation exclusive des timers pour ce module
  // Allocation avec délais plus longs pour stabiliser
  bool timersOk = true;
  
  // Essayer d'utiliser différentes combinaisons de timers si les premiers échouent
  int timerConfigs[3][4] = {
    {0, 1, 2, 3},  // Configuration standard
    {0, 2, 4, 6},  // Configuration alternative 1
    {1, 3, 5, 7}   // Configuration alternative 2
  };
  
  // Essayer les différentes configurations de timers
  bool timerConfigOk = false;
  for (int configIdx = 0; configIdx < 3 && !timerConfigOk; configIdx++) {
    LOG_INFO("SERVO", "Essai configuration timer #%d", configIdx + 1);
    
    timerConfigOk = true;
    for (int i = 0; i < 4 && timerConfigOk; i++) {
      delay(50); // Délai entre allocations
      
      // Vérifier si ce timer est déjà utilisé par une autre fonction
      if (ESP32PWM::hasPwm(timerConfigs[configIdx][i])) {
        LOG_WARNING("SERVO", "Timer %d déjà utilisé", timerConfigs[configIdx][i]);
        timerConfigOk = false;
      } else {
        ESP32PWM::allocateTimer(timerConfigs[configIdx][i]);
      }
    }
    
    if (timerConfigOk) {
      LOG_INFO("SERVO", "Configuration timer #%d utilisée avec succès", configIdx + 1);
      break;
    } else {
      // Libérer tous les timers si cette configuration a échoué
      for (int i = 0; i < 4; i++) {
        // ESP32PWM::deallocateTimer n'existe pas, on ne fait rien ici
        // La libération se fait automatiquement quand le servo est détaché
      }
      delay(100); // Délai de stabilisation
    }
  }
  
  if (!timerConfigOk) {
    LOG_ERROR("SERVO", "Impossible d'allouer les timers pour les servomoteurs");
    return false;
  }
  
  // Configuration de la fréquence des servomoteurs
  const int servoFreq = SERVO_FREQUENCY;
  
  // Initialisation du servo de direction
  bool dirOk = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    LOG_INFO("SERVO", "Initialisation du servo de direction (tentative %d/3)...", attempt);
    
    // Configuration du servo avec les paramètres corrects
    directionServo.setPeriodHertz(servoFreq);
    delay(50);
    
    // Tester différentes plages d'impulsions si l'initialisation échoue
    int minPulse = SERVO_MIN_PULSE_WIDTH;
    int maxPulse = SERVO_MAX_PULSE_WIDTH;
    
    if (attempt == 2) {
      // Deuxième tentative avec plage plus restreinte
      minPulse = 700;
      maxPulse = 2300;
    } else if (attempt == 3) {
      // Troisième tentative avec valeurs par défaut de la bibliothèque
      minPulse = 544;
      maxPulse = 2400;
    }
    
    dirOk = directionServo.attach(SERVO_DIRECTION_PIN, minPulse, maxPulse);
    
    if (dirOk) {
      directionServo.write(90); // Position neutre
      delay(100);
      LOG_INFO("SERVO", "Servo de direction initialisé avec succès");
      break;
    }
    
    // Détacher et réessayer après un délai
    if (directionServo.attached()) {
      directionServo.detach();
    }
    delay(100);
  }
  
  if (!dirOk) {
    LOG_ERROR("SERVO", "Échec d'initialisation du servo de direction après 3 tentatives");
    servoDetachAll();
    return false;
  }
  
  // Marquer comme initialisé
  servosInitialized = true;
  lastServoUpdateTime = millis();
  
  // Comme le servo de direction fonctionne, initialiser les autres servos
  // mais ne pas échouer le démarrage si ces servos secondaires ne fonctionnent pas
  
  // Initialisation du servo de trim (avec 2 tentatives)
  bool trimOk = false;
  for (int attempt = 1; attempt <= 2; attempt++) {
    LOG_INFO("SERVO", "Initialisation du servo de trim...");
    
    trimServo.setPeriodHertz(servoFreq);
    delay(50);
    
    trimOk = trimServo.attach(SERVO_TRIM_PIN, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
    
    if (trimOk) {
      trimServo.write(90); // Position neutre
      LOG_INFO("SERVO", "Servo de trim initialisé avec succès");
      break;
    }
    
    if (trimServo.attached()) {
      trimServo.detach();
    }
    delay(100);
  }
  
  if (!trimOk) {
    LOG_WARNING("SERVO", "Échec d'initialisation du servo de trim - continuera sans ce servo");
  }
  
  // Initialisation du servo de modulation de ligne (avec 2 tentatives)
  bool lineModOk = false;
  for (int attempt = 1; attempt <= 2; attempt++) {
    LOG_INFO("SERVO", "Initialisation du servo de modulation de ligne...");
    
    lineModServo.setPeriodHertz(servoFreq);
    delay(50);
    
    lineModOk = lineModServo.attach(SERVO_LINEMOD_PIN, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
    
    if (lineModOk) {
      lineModServo.write(0); // Position initiale
      LOG_INFO("SERVO", "Servo de modulation de ligne initialisé avec succès");
      break;
    }
    
    if (lineModServo.attached()) {
      lineModServo.detach();
    }
    delay(100);
  }
  
  if (!lineModOk) {
    LOG_WARNING("SERVO", "Échec d'initialisation du servo de modulation de ligne - continuera sans ce servo");
  }
  
  LOG_INFO("SERVO", "Initialisation des servomoteurs terminée");
  return true; // Succès si au moins le servo principal fonctionne
}

/**
 * Fonction de mise à jour périodique des servomoteurs
 * Contrôle la fréquence de mise à jour pour éviter les surcharges
 * 
 * @param direction Angle de direction (-90 à +90)
 * @param trim Angle de trim (-45 à +45)
 * @param lineModulation Position de modulation de ligne (0 à 100)
 * @return true si la mise à jour a réussi, false sinon
 */
bool servoUpdateAll(int direction, int trim, int lineModulation) {
  // Vérifier si les servos sont initialisés
  if (!servosInitialized) {
    // Si pas initialisé, tenter une réinitialisation mais pas trop fréquemment
    static unsigned long lastInitAttempt = 0;
    unsigned long now = millis();
    
    if (now - lastInitAttempt > 5000) { // Limiter les tentatives à une fois toutes les 5 secondes
      lastInitAttempt = now;
      LOG_WARNING("SERVO", "Servos non initialisés, tentative de réinitialisation...");
      return servoInitAll();
    }
    
    return false;
  }
  
  // Limiter la fréquence des mises à jour pour éviter de surcharger le bus
  unsigned long now = millis();
  if (now - lastServoUpdateTime < SERVO_UPDATE_INTERVAL) {
    return true; // Pas besoin de mise à jour, trop récente
  }
  
  lastServoUpdateTime = now;
  
  // Mettre à jour chaque servo avec gestion d'erreur
  bool success = true;
  
  // Direction
  if (!servoSetDirection(direction)) {
    success = false;
    // Ne pas interrompre, continuer avec les autres servos
  }
  
  // Trim
  if (!servoSetTrim(trim)) {
    success = false;
  }
  
  // Modulation de ligne
  if (!servoSetLineModulation(lineModulation)) {
    success = false;
  }
  
  // Détecter les problèmes persistants et planifier une réinitialisation si nécessaire
  static int failureCount = 0;
  if (!success) {
    failureCount++;
    if (failureCount > 5) { // Après 5 échecs consécutifs
      LOG_WARNING("SERVO", "Problèmes persistants avec les servos, réinitialisation planifiée");
      servosInitialized = false; // Forcer une réinitialisation lors de la prochaine tentative
      failureCount = 0;
    }
  } else {
    failureCount = 0; // Réinitialiser le compteur en cas de succès
  }
  
  return success;
}

/**
 * Tente de réinitialiser les servomoteurs en cas d'échec
 * Cette fonction peut être appelée périodiquement pour tenter de récupérer
 * les servomoteurs si l'initialisation initiale a échoué.
 * @return true si la réinitialisation réussit, false sinon
 */
bool servoReinitialize() {
  // Si les servos sont déjà initialisés et attachés, pas besoin de les réinitialiser
  if (servosInitialized && 
      directionServo.attached() && 
      trimServo.attached() && 
      lineModServo.attached()) {
    // Tout est OK, pas besoin de réinitialiser
    return true;
  }
  
  // Réinitialiser tous les servomoteurs
  LOG_INFO("SERVO", "Tentative de réinitialisation des servomoteurs...");
  
  // Détacher tous les servos avant de réinitialiser
  servoDetachAll();
  
  // Attendre un court instant pour que les ressources soient libérées
  delay(200);
  
  // Nouvelle tentative d'initialisation
  return servoInitAll();
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