#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "../../core/config.h" // Inclure config.h pour les définitions de broches

// Limites d'angle pour les servomoteurs
#define DIRECTION_MIN_ANGLE -90
#define DIRECTION_MAX_ANGLE 90
#define TRIM_MIN_ANGLE -45
#define TRIM_MAX_ANGLE 45

/**
 * Initialise tous les servomoteurs
 * @return true si l'initialisation réussit, false sinon
 */
bool servoInitAll();

/**
 * Déplace le servomoteur de direction
 * @param angle Angle de direction (-90 à +90)
 * @return true si le mouvement réussit, false sinon
 */
bool servoSetDirection(int angle);

/**
 * Déplace le servomoteur de trim
 * @param angle Angle de trim (-45 à +45)
 * @return true si le mouvement réussit, false sinon
 */
bool servoSetTrim(int angle);

/**
 * Contrôle le servomoteur de modulation de ligne
 * @param position Position du servomoteur (0 à 100)
 * @return true si le mouvement réussit, false sinon
 */
bool servoSetLineModulation(int position);

/**
 * Détache tous les servomoteurs pour économiser de l'énergie
 */
void servoDetachAll();

/**
 * Vérifie si un servomoteur est fixé
 * @param servoIndex Index du servomoteur (0=direction, 1=trim, 2=linemod)
 * @return true si le servomoteur est fixé, false sinon
 */
bool servoIsAttached(uint8_t servoIndex);