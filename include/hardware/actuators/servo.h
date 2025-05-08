/*
  -----------------------
  Kite PiloteV3 - Module Servo (Interface)
  -----------------------
  
  Interface pour le contrôle des servomoteurs du système de positionnement du cerf-volant.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== INTERFACE PUBLIQUE =====
  Ce fichier définit l'interface publique pour le contrôle des trois servomoteurs
  principaux : direction, trim et modulation de ligne.
  
  Principales fonctionnalités exposées :
  - servoInitAll() : Initialisation et configuration des servomoteurs
  - servoUpdateAll() : Mise à jour des positions des trois servomoteurs
  - servoSetDirection() : Contrôle direct du servo de direction
  - servoSetTrim() : Contrôle direct du servo de trim
  - servoSetLineModulation() : Contrôle direct du servo de modulation de ligne
  
  Interactions avec d'autres modules :
  - TaskManager : Appelle ces fonctions depuis des tâches spécifiques
  - Autopilot : Utilise ces fonctions pour positionner le cerf-volant
  - Config : Fournit les définitions des broches et constantes
  - Logging : Utilisé pour la journalisation
  
  Contraintes techniques :
  - Les angles de direction sont limités à [-90°, +90°]
  - Les angles de trim sont limités à [-45°, +45°]
  - La modulation de ligne est exprimée en pourcentage [0-100]
*/

#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "../../core/config.h"

// Constantes pour les limites d'angle des servomoteurs
#define DIRECTION_MIN_ANGLE -90   // Angle minimum de direction (degrés)
#define DIRECTION_MAX_ANGLE 90    // Angle maximum de direction (degrés)
#define TRIM_MIN_ANGLE -45        // Angle minimum de trim (degrés)
#define TRIM_MAX_ANGLE 45         // Angle maximum de trim (degrés)

// Constantes pour l'optimisation des performances servomoteurs
#define SERVO_FREQUENCY 50        // Fréquence PWM en Hz
#define SERVO_UPDATE_INTERVAL 20  // Intervalle minimum entre les mises à jour (ms)

// Fonctions d'initialisation et de contrôle
bool servoInitAll();
bool servoReinitialize();
bool servoUpdateAll(int direction, int trim, int lineModulation);  // Nouvelle fonction de mise à jour optimisée
bool servoSetDirection(int angle);
bool servoSetTrim(int angle);
bool servoSetLineModulation(int position);
void servoDetachAll();
bool servoIsAttached(uint8_t servoIndex);
void servoInitialize();  // Ajout de la déclaration de la fonction publique pour initialiser les servos

#endif // SERVO_H