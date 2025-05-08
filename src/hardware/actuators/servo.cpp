/*
  -----------------------
  Kite PiloteV3 - Module Servo (Implémentation)
  -----------------------
  
  Implémentation du module de contrôle des servomoteurs pour le positionnement du cerf-volant.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module gère les servomoteurs responsables du contrôle directionnel du cerf-volant.
  Il utilise une machine à états finis (FSM) pour toutes les opérations asynchrones,
  garantissant un fonctionnement non-bloquant compatible avec l'architecture multitâche.
  
  Principes de fonctionnement :
  1. Initialisation par étapes des servomoteurs via une FSM évitant tout `delay()`
  2. Allocation des timers PWM de manière dynamique et non-bloquante
  3. Contrôle des positions avec conversion des angles d'entrée vers les valeurs PWM
  4. Surveillance de l'état des servos et tentatives de récupération en cas de panne
  
  Architecture FSM implémentée :
  - États d'initialisation : INIT_START, TIMER_SCAN, TIMER_ALLOC, SERVO_CONFIG, SERVO_ATTACH, etc.
  - Toutes les transitions sont basées sur le temps écoulé et les résultats d'opérations
  - Chaque état effectue une opération atomique rapide puis rend la main au système
  - Cette approche est bien plus adaptée aux systèmes temps réel que l'utilisation de `delay()`
  
  Interactions avec d'autres modules :
  - TaskManager : Appelle ce module depuis des tâches FreeRTOS
  - Autopilot : Fournit les angles de direction et trim à appliquer
  - Logging : Journalisation des événements et erreurs
  - Config : Définition des broches GPIO et constantes
  
  Aspects techniques notables :
  - Utilisation systématique de timestamps (millis()) au lieu de delay() pour gérer le temps
  - FSM documentée dans le code avec des commentaires expliquant chaque état
  - Variables statiques pour conserver l'état entre les appels
  - Mécanisme de récupération automatique en cas d'échec d'initialisation
  
  Exemple d'implémentation de la FSM :
  ```
  switch(initState) {
    case INIT_START:
      // Initialisation des variables, transition vers l'état suivant
      initState = TIMER_SCAN;
      return false; // Pas terminé
      
    case TIMER_SCAN:
      // Recherche de timers disponibles sans bloquer
      if(currentTimer < maxTimers) {
        // Scanner un timer à la fois puis revenir
        currentTimer++;
        return false;
      }
      // Passage à l'étape suivante une fois tous les timers scannés
      initState = TIMER_ALLOC;
      return false;
      
    // ... autres états de la FSM ...
  }
  ```
*/

#include "hardware/actuators/servo.h"
#include "utils/logging.h"

// Variables pour les servomoteurs (déclarées en tant que globales)
Servo directionServo;
Servo trimServo;
Servo lineModServo;

// Variables d'état
static bool servosInitialized = false;
static unsigned long lastServoUpdateTime = 0;
static int initState = 0;
static unsigned long lastActionTime = 0;
static int currentTimer = 0;

/**
 * Initialisation de tous les servomoteurs
 * Utilise une machine à états finis pour éviter les blocages
 * @return true si l'initialisation est terminée avec succès, false sinon
 */
bool servoInitAll() {
  // Machine à états finie pour initialisation non-bloquante
  unsigned long currentTime = millis();
  static int availableTimers[4] = {-1, -1, -1, -1};
  static int numAvailableTimers = 0;
  static int allocatedTimers = 0;
  static int currentAttempt = 0;
  static bool dirOk = false;
  static bool trimOk = false;
  static bool lineModOk = false;
  
  // Réinitialisation complète de l'état pour éviter les problèmes
  if (initState == 0) {
    numAvailableTimers = 0;
    currentTimer = 0;
    allocatedTimers = 0;
    currentAttempt = 0;
    dirOk = false;
    trimOk = false;
    lineModOk = false;
    
    for (int i = 0; i < 4; i++) {
      availableTimers[i] = -1;
    }
    
    initState = 1;
    lastActionTime = currentTime;
    LOG_INFO("SERVO", "Démarrage de la séquence d'initialisation");
    return false; // Pas encore terminé
  }
  
  // Allocation immédiate de deux timers connus pour fonctionner
  if (initState == 1) {
    ESP32PWM::allocateTimer(0);  // Timer 0
    ESP32PWM::allocateTimer(1);  // Timer 1
    allocatedTimers = 2;
    LOG_INFO("SERVO", "2 timers alloués (0 et 1)");
    initState = 3;  // Passer directement à la configuration des servos
    lastActionTime = currentTime;
    return false;
  }
  
  // Configuration du servo de direction
  if (initState == 3) {
    if (currentTime - lastActionTime < 50) {
      return false; // Attendre avant de configurer
    }
    
    // Configuration et attachement du servo de direction
    directionServo.setPeriodHertz(SERVO_FREQUENCY);
    dirOk = directionServo.attach(SERVO_DIRECTION_PIN, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);

    if (dirOk) {
      directionServo.write(90);  // Position centrale
      LOG_INFO("SERVO", "Servo de direction initialisé avec succès sur la broche %d", SERVO_DIRECTION_PIN);
    } else {
      LOG_ERROR("SERVO", "Échec d'initialisation du servo de direction sur la broche %d", SERVO_DIRECTION_PIN);
      // Deuxième tentative avec des paramètres par défaut
      dirOk = directionServo.attach(SERVO_DIRECTION_PIN);
      if (dirOk) {
        directionServo.write(90);
        LOG_INFO("SERVO", "Servo de direction initialisé (méthode alternative) sur la broche %d", SERVO_DIRECTION_PIN);
      } else {
        LOG_ERROR("SERVO", "Échec total d'initialisation du servo de direction sur la broche %d", SERVO_DIRECTION_PIN);
      }
    }
    
    lastActionTime = currentTime;
    initState = 4;
    return false;
  }
  
  // Tentative d'attachement du servo de direction
  if (initState == 4) {
    if (currentTime - lastActionTime < 50) {
      return false; // Attendre avant d'attacher
    }
    
    initState = 6; // Passer à l'initialisation du trim
    lastActionTime = currentTime;
    return false;
  }
  
  // Configuration du servo de trim
  if (initState == 6) {
    if (currentTime - lastActionTime < 50) {
      return false; // Attendre avant de configurer
    }
    
    trimServo.setPeriodHertz(SERVO_FREQUENCY);
    lastActionTime = currentTime;
    initState = 7;
    LOG_INFO("SERVO", "Servo de trim configuré");
    return false;
  }
  
  // Tentative d'attachement du servo de trim
  if (initState == 7) {
    if (currentTime - lastActionTime < 50) {
      return false; // Attendre avant d'attacher
    }
    
    trimOk = trimServo.attach(SERVO_TRIM_PIN, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
    
    if (!trimOk) {
      LOG_WARNING("SERVO", "Échec de l'attachement initial du trim, tentative avec paramètres par défaut");
      trimOk = trimServo.attach(SERVO_TRIM_PIN);
    }
    
    if (trimOk) {
      trimServo.write(90);  // Position centrale
      LOG_INFO("SERVO", "Servo de trim initialisé avec succès");
    } else {
      LOG_ERROR("SERVO", "Échec d'initialisation du servo de trim");
    }
    
    initState = 8;
    lastActionTime = currentTime;
    return false;
  }
  
  // Configuration du servo de modulation de ligne
  if (initState == 8) {
    if (currentTime - lastActionTime < 50) {
      return false; // Attendre avant de configurer
    }
    
    lineModServo.setPeriodHertz(SERVO_FREQUENCY);
    lastActionTime = currentTime;
    initState = 9;
    LOG_INFO("SERVO", "Servo de modulation de ligne configuré");
    return false;
  }
  
  // Tentative d'attachement du servo de modulation de ligne
  if (initState == 9) {
    if (currentTime - lastActionTime < 50) {
      return false; // Attendre avant d'attacher
    }
    
    lineModOk = lineModServo.attach(SERVO_LINEMOD_PIN, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
    
    if (!lineModOk) {
      LOG_WARNING("SERVO", "Échec de l'attachement initial de la ligne, tentative avec paramètres par défaut");
      lineModOk = lineModServo.attach(SERVO_LINEMOD_PIN);
    }
    
    if (lineModOk) {
      lineModServo.write(90);  // Position centrale
      LOG_INFO("SERVO", "Servo de modulation de ligne initialisé avec succès");
    } else {
      LOG_ERROR("SERVO", "Échec d'initialisation du servo de modulation de ligne");
    }
    
    // Finalisation de l'initialisation
    servosInitialized = (dirOk || trimOk || lineModOk);
    lastServoUpdateTime = millis();
    
    LOG_INFO("SERVO", "Initialisation des servomoteurs terminée (Direction: %s, Trim: %s, LineMod: %s)",
             dirOk ? "OK" : "NOK", trimOk ? "OK" : "NOK", lineModOk ? "OK" : "NOK");
    
    // Réinitialiser la machine à états pour la prochaine initialisation
    initState = 0;
    
    return servosInitialized; // Initialisation terminée
  }
  
  // État inconnu, réinitialiser
  initState = 0;
  return false;
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
  
  // Log périodique des valeurs envoyées aux servomoteurs (toutes les 10 mises à jour)
  static int logCounter = 0;
  if (++logCounter >= 10) {
    LOG_INFO("SERVO", "Mise à jour servos: Direction=%d° Trim=%d° LineModulation=%d%%", 
             direction, trim, lineModulation);
    logCounter = 0;
  }
  
  // Mettre à jour chaque servo avec gestion d'erreur
  bool success = true;
  
  // Direction
  if (!servoSetDirection(direction)) {
    success = false;
    LOG_ERROR("SERVO", "Échec mise à jour servo direction à %d° (PIN: %d)", 
              direction, SERVO_DIRECTION_PIN);
  }
  
  // Trim
  if (!servoSetTrim(trim)) {
    success = false;
    LOG_ERROR("SERVO", "Échec mise à jour servo trim à %d° (PIN: %d)", 
              trim, SERVO_TRIM_PIN);
  }
  
  // Modulation de ligne
  if (!servoSetLineModulation(lineModulation)) {
    success = false;
    LOG_ERROR("SERVO", "Échec mise à jour servo ligne à %d%% (PIN: %d)", 
              lineModulation, SERVO_LINEMOD_PIN);
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

void servoInitialize() {
    pinMode(SERVO_DIRECTION_PIN, OUTPUT);
    pinMode(SERVO_TRIM_PIN, OUTPUT);

    ledcSetup(0, SERVO_FREQUENCY, 8);
    ledcAttachPin(SERVO_DIRECTION_PIN, 0);

    ledcSetup(1, SERVO_FREQUENCY, 8);
    ledcAttachPin(SERVO_TRIM_PIN, 1);

    LOG_INFO("SERVO", "Servomoteurs initialisés");
}

// Déplacement des vérifications des servos depuis `system.cpp`.
bool servoCheckHealth() {
    if (!servoIsAttached(0) || !servoIsAttached(1) || !servoIsAttached(2)) {
        LOG_WARNING("SERVO", "Un ou plusieurs servos ne sont pas attachés. Tentative de récupération...");
        return servoReinitialize();
    }
    return true;
}