#pragma once

// Déclarations pour le contrôle des servomoteurs
#include <Arduino.h>

// Initialisation des servomoteurs
void servoInitAll();

// Contrôle individuel des servos
void setServoAngle(int servoId, float angle);
float getServoAngle(int servoId);