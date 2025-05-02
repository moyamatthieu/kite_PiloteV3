/*
  -----------------------
  Kite PiloteV3 - Module de gestion des potentiomètres (Implémentation)
  -----------------------
  
  Implémentation des fonctions du module de gestion des potentiomètres.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../include/potentiometer_manager.h"
#include "../include/logging.h"

/**
 * Constructeur de la classe PotentiometerManager
 * Initialise les variables membres
 */
PotentiometerManager::PotentiometerManager() {
  // Initialiser les valeurs par défaut des potentiomètres
  direction.rawValue = 0;
  direction.smoothValue = 0;
  direction.mappedValue = 0;
  direction.lastRawValue = 0;
  direction.hasChanged = false;
  
  trim.rawValue = 0;
  trim.smoothValue = 0;
  trim.mappedValue = 0;
  trim.lastRawValue = 0;
  trim.hasChanged = false;
  
  lineLength.rawValue = 0;
  lineLength.smoothValue = 0;
  lineLength.mappedValue = 0;
  lineLength.lastRawValue = 0;
  lineLength.hasChanged = false;
  
  // Valeurs de calibration par défaut (plage complète)
  directionMin = 0;
  directionMax = ADC_RESOLUTION;
  trimMin = 0;
  trimMax = ADC_RESOLUTION;
  lineLengthMin = 0;
  lineLengthMax = ADC_RESOLUTION;
  
  lastUpdateTime = 0;

  // État du pilote automatique
  autoPilotEnabled = false;
  autoPilotDirection = 0;
  autoPilotTrim = 0;
  autoPilotLineLength = 0;
}

/**
 * Destructeur de la classe PotentiometerManager
 */
PotentiometerManager::~PotentiometerManager() {
  // Rien à libérer
}

/**
 * Initialise le gestionnaire de potentiomètres
 */
void PotentiometerManager::begin() {
  LOG_INFO("POT", "Initialisation des potentiomètres");
  
  // Configurer les pins ADC
  pinMode(POT_DIRECTION, INPUT);
  pinMode(POT_TRIM, INPUT);
  pinMode(POT_LENGTH, INPUT);
  
  // ESP32 ADC est déjà configuré à 12 bits par défaut
  // Pas besoin de configuration supplémentaire
  
  // Lecture initiale des potentiomètres
  updatePotentiometers();
  
  LOG_INFO("POT", "Initialisation terminée. Direction: %d, Trim: %d, Longueur: %d", 
          getDirection(), getTrim(), getLineLength());
}

/**
 * Lecture d'un potentiomètre avec moyenne de plusieurs échantillons
 * @param pin Pin du potentiomètre à lire
 * @return Valeur lue (0-4095)
 */
int PotentiometerManager::readPotentiometer(uint8_t pin) {
  // Prendre plusieurs échantillons pour réduire le bruit
  const int numSamples = 3;
  int total = 0;
  
  for (int i = 0; i < numSamples; i++) {
    total += analogRead(pin);
    delayMicroseconds(100); // Court délai entre les échantillons
  }
  
  return total / numSamples;
}

/**
 * Applique une zone morte au potentiomètre pour éviter les oscillations
 * @param value Valeur à filtrer
 * @param center Valeur centrale (pour les potentiomètres bidirectionnels)
 * @return Valeur filtrée
 */
int PotentiometerManager::applyDeadzone(int value, int center) {
  int offset = value - center;
  
  // Appliquer la zone morte
  if (abs(offset) < POT_DEADZONE) {
    return center;
  }
  
  return value;
}

/**
 * Lissage des valeurs pour éviter les sauts brusques
 * @param newValue Nouvelle valeur brute
 * @param oldValue Ancienne valeur lissée
 * @return Nouvelle valeur lissée
 */
int PotentiometerManager::smoothValue(int newValue, int oldValue) {
  return oldValue + (ADC_SMOOTHING_FACTOR * (newValue - oldValue));
}

/**
 * Met à jour les valeurs des potentiomètres
 */
void PotentiometerManager::updatePotentiometers() {
  unsigned long currentTime = millis();
  
  // Limiter la fréquence de mise à jour
  if (currentTime - lastUpdateTime < POT_READ_INTERVAL) {
    return;
  }
  
  lastUpdateTime = currentTime;
  
  // Lecture du potentiomètre de direction
  direction.rawValue = readPotentiometer(POT_DIRECTION);
  direction.hasChanged = abs(direction.rawValue - direction.lastRawValue) > POT_DEADZONE;
  direction.lastRawValue = direction.rawValue;
  
  if (direction.hasChanged) {
    // Appliquer la zone morte pour la position centrale
    int centerValue = (directionMax + directionMin) / 2;
    int valueWithDeadzone = applyDeadzone(direction.rawValue, centerValue);
    
    // Lisser la valeur
    direction.smoothValue = smoothValue(valueWithDeadzone, direction.smoothValue);
    
    // Mapper la valeur dans la plage -100 à +100
    if (direction.smoothValue < centerValue) {
      // Négatif (gauche)
      direction.mappedValue = map(direction.smoothValue, directionMin, centerValue, -100, 0);
    } else {
      // Positif (droite)
      direction.mappedValue = map(direction.smoothValue, centerValue, directionMax, 0, 100);
    }
    
    // Limiter à la plage -100 à +100
    direction.mappedValue = constrain(direction.mappedValue, -100, 100);
  }
  
  // Lecture du potentiomètre de trim
  trim.rawValue = readPotentiometer(POT_TRIM);
  trim.hasChanged = abs(trim.rawValue - trim.lastRawValue) > POT_DEADZONE;
  trim.lastRawValue = trim.rawValue;
  
  if (trim.hasChanged) {
    // Appliquer la zone morte pour la position centrale
    int centerValue = (trimMax + trimMin) / 2;
    int valueWithDeadzone = applyDeadzone(trim.rawValue, centerValue);
    
    // Lisser la valeur
    trim.smoothValue = smoothValue(valueWithDeadzone, trim.smoothValue);
    
    // Mapper la valeur dans la plage -100 à +100
    if (trim.smoothValue < centerValue) {
      // Négatif
      trim.mappedValue = map(trim.smoothValue, trimMin, centerValue, -100, 0);
    } else {
      // Positif
      trim.mappedValue = map(trim.smoothValue, centerValue, trimMax, 0, 100);
    }
    
    // Limiter à la plage -100 à +100
    trim.mappedValue = constrain(trim.mappedValue, -100, 100);
  }
  
  // Lecture du potentiomètre de longueur de ligne
  lineLength.rawValue = readPotentiometer(POT_LENGTH);
  lineLength.hasChanged = abs(lineLength.rawValue - lineLength.lastRawValue) > POT_DEADZONE;
  lineLength.lastRawValue = lineLength.rawValue;
  
  if (lineLength.hasChanged) {
    // Lisser la valeur
    lineLength.smoothValue = smoothValue(lineLength.rawValue, lineLength.smoothValue);
    
    // Mapper la valeur dans la plage 0 à 100
    lineLength.mappedValue = map(lineLength.smoothValue, lineLengthMin, lineLengthMax, 0, 100);
    
    // Limiter à la plage 0 à 100
    lineLength.mappedValue = constrain(lineLength.mappedValue, 0, 100);
  }
  
  // Vérifier si un potentiomètre a été ajusté et désactiver le pilote automatique si nécessaire
  checkAutoPilotStatus();
}

/**
 * Obtient la valeur de direction (-100 à +100)
 * @return Valeur mappée de direction
 */
int PotentiometerManager::getDirection() {
  return direction.mappedValue;
}

/**
 * Obtient la valeur de trim (-100 à +100)
 * @return Valeur mappée de trim
 */
int PotentiometerManager::getTrim() {
  return trim.mappedValue;
}

/**
 * Obtient la valeur de longueur de ligne (0 à 100)
 * @return Valeur mappée de longueur de ligne
 */
int PotentiometerManager::getLineLength() {
  return lineLength.mappedValue;
}

/**
 * Vérifie si la valeur de direction a changé
 * @return true si la valeur a changé, false sinon
 */
bool PotentiometerManager::hasDirectionChanged() {
  bool changed = direction.hasChanged;
  direction.hasChanged = false;
  return changed;
}

/**
 * Vérifie si la valeur de trim a changé
 * @return true si la valeur a changé, false sinon
 */
bool PotentiometerManager::hasTrimChanged() {
  bool changed = trim.hasChanged;
  trim.hasChanged = false;
  return changed;
}

/**
 * Vérifie si la valeur de longueur de ligne a changé
 * @return true si la valeur a changé, false sinon
 */
bool PotentiometerManager::hasLineLengthChanged() {
  bool changed = lineLength.hasChanged;
  lineLength.hasChanged = false;
  return changed;
}

/**
 * Vérifie si l'un des potentiomètres a changé
 * @return true si l'un des potentiomètres a changé, false sinon
 */
bool PotentiometerManager::hasAnyPotChanged() {
  return hasDirectionChanged() || hasTrimChanged() || hasLineLengthChanged();
}

/**
 * Calibre les potentiomètres
 * Détecte les valeurs min et max en demandant à l'utilisateur de bouger les potentiomètres
 */
void PotentiometerManager::calibrate() {
  LOG_INFO("POT", "Démarrage de la calibration des potentiomètres");
  
  // Réinitialiser les valeurs de calibration
  directionMin = ADC_RESOLUTION;
  directionMax = 0;
  trimMin = ADC_RESOLUTION;
  trimMax = 0;
  lineLengthMin = ADC_RESOLUTION;
  lineLengthMax = 0;
  
  // Période de calibration (5 secondes)
  unsigned long startTime = millis();
  unsigned long calibrationTime = 5000;
  
  LOG_INFO("POT", "Bougez les potentiomètres dans leur plage complète...");
  
  while (millis() - startTime < calibrationTime) {
    // Direction
    int dirValue = analogRead(POT_DIRECTION);
    if (dirValue < directionMin) directionMin = dirValue;
    if (dirValue > directionMax) directionMax = dirValue;
    
    // Trim
    int trimValue = analogRead(POT_TRIM);
    if (trimValue < trimMin) trimMin = trimValue;
    if (trimValue > trimMax) trimMax = trimValue;
    
    // Longueur
    int lengthValue = analogRead(POT_LENGTH);
    if (lengthValue < lineLengthMin) lineLengthMin = lengthValue;
    if (lengthValue > lineLengthMax) lineLengthMax = lengthValue;
    
    delay(10);
  }
  
  // Ajouter une marge de sécurité
  directionMin = max(0, directionMin - 50);
  directionMax = min(ADC_RESOLUTION, directionMax + 50);
  trimMin = max(0, trimMin - 50);
  trimMax = min(ADC_RESOLUTION, trimMax + 50);
  lineLengthMin = max(0, lineLengthMin - 50);
  lineLengthMax = min(ADC_RESOLUTION, lineLengthMax + 50);
  
  LOG_INFO("POT", "Calibration terminée");
  LOG_INFO("POT", "Direction: Min=%d, Max=%d", directionMin, directionMax);
  LOG_INFO("POT", "Trim: Min=%d, Max=%d", trimMin, trimMax);
  LOG_INFO("POT", "Longueur: Min=%d, Max=%d", lineLengthMin, lineLengthMax);
}

/**
 * Définit manuellement les valeurs de calibration
 */
void PotentiometerManager::setCalibrationValues(int dirMin, int dirMax, int trmMin, int trmMax, int lengthMin, int lengthMax) {
  directionMin = dirMin;
  directionMax = dirMax;
  trimMin = trmMin;
  trimMax = trmMax;
  lineLengthMin = lengthMin;
  lineLengthMax = lengthMax;
  
  LOG_INFO("POT", "Valeurs de calibration définies manuellement");
}

/**
 * Active ou désactive le mode pilote automatique
 * @param enabled true pour activer, false pour désactiver
 */
void PotentiometerManager::setAutoPilotMode(bool enabled) {
  // Si on active le pilote automatique, enregistrer les valeurs actuelles
  if (enabled && !autoPilotEnabled) {
    autoPilotDirection = direction.mappedValue;
    autoPilotTrim = trim.mappedValue;
    autoPilotLineLength = lineLength.mappedValue;
    LOG_INFO("AUTOPILOT", "Pilote automatique activé. Direction: %d, Trim: %d, Longueur: %d",
             autoPilotDirection, autoPilotTrim, autoPilotLineLength);
  } else if (!enabled && autoPilotEnabled) {
    LOG_INFO("AUTOPILOT", "Pilote automatique désactivé");
  }
  
  autoPilotEnabled = enabled;
}

/**
 * Vérifie si le pilote automatique est activé
 * @return true si le pilote automatique est activé, false sinon
 */
bool PotentiometerManager::isAutoPilotEnabled() {
  return autoPilotEnabled;
}

/**
 * Vérifie si un potentiomètre a été ajusté et désactive le pilote automatique si nécessaire
 * Cette fonction doit être appelée après updatePotentiometers()
 */
void PotentiometerManager::checkAutoPilotStatus() {
  // Vérifier si le pilote automatique est activé
  if (!autoPilotEnabled) {
    return;
  }
  
  // Seuil de différence pour détecter un ajustement manuel (plus sensible que le deadzone normal)
  const int MANUAL_ADJUST_THRESHOLD = POT_DEADZONE / 2;
  
  // Vérifier si un des potentiomètres a été ajusté manuellement
  bool directionAdjusted = abs(direction.mappedValue - autoPilotDirection) > MANUAL_ADJUST_THRESHOLD;
  bool trimAdjusted = abs(trim.mappedValue - autoPilotTrim) > MANUAL_ADJUST_THRESHOLD;
  bool lineLengthAdjusted = abs(lineLength.mappedValue - autoPilotLineLength) > MANUAL_ADJUST_THRESHOLD;
  
  // Si un potentiomètre a été ajusté, désactiver le pilote automatique
  if (directionAdjusted || trimAdjusted || lineLengthAdjusted) {
    LOG_INFO("AUTOPILOT", "Potentiomètre ajusté manuellement, désactivation du pilote automatique");
    if (directionAdjusted) {
      LOG_DEBUG("AUTOPILOT", "Direction ajustée: %d → %d", autoPilotDirection, direction.mappedValue);
    }
    if (trimAdjusted) {
      LOG_DEBUG("AUTOPILOT", "Trim ajusté: %d → %d", autoPilotTrim, trim.mappedValue);
    }
    if (lineLengthAdjusted) {
      LOG_DEBUG("AUTOPILOT", "Longueur ajustée: %d → %d", autoPilotLineLength, lineLength.mappedValue);
    }
    
    setAutoPilotMode(false);
  }
}

/**
 * Obtient la dernière valeur de longueur de ligne
 * @return Dernière valeur de longueur de ligne
 */
int PotentiometerManager::getLastLineLength() {
  return lineLength.lastRawValue;
}