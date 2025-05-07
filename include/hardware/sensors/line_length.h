/*
  -----------------------
  Kite PiloteV3 - Module Line Length Sensor (Interface)
  -----------------------
  
  Interface pour la gestion du capteur de longueur de ligne.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef LINE_LENGTH_H
#define LINE_LENGTH_H

#include <Arduino.h>

/**
 * Initialise le capteur de longueur de ligne
 * @return true si succès, false si échec
 */
bool lineLengthInit();

/**
 * Lit la longueur de ligne actuelle
 * @return Longueur de ligne en centimètres (ou -1 si erreur)
 */
int lineLengthRead();

/**
 * Calibre le capteur de longueur de ligne
 * @param minLength Longueur minimale en cm
 * @param maxLength Longueur maximale en cm
 * @return true si succès, false si échec
 */
bool lineLengthCalibrate(int minLength, int maxLength);

/**
 * Vérifie si le capteur de longueur de ligne est fonctionnel
 * @return true si fonctionnel, false sinon
 */
bool lineLengthIsHealthy();

/**
 * Mise à jour périodique du capteur de longueur de ligne
 * Fonction thread-safe compatible avec FreeRTOS
 * @return true si succès, false si échec
 */
bool updateLineLengthSensor();

#endif // LINE_LENGTH_H