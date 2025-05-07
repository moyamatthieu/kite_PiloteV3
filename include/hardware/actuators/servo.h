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

#endif // SERVO_H