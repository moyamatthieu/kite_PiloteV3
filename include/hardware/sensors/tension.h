/*
  -----------------------
  Kite PiloteV3 - Module Tension Sensor (Interface)
  -----------------------
  
  Interface pour la gestion du capteur de tension de ligne.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef TENSION_H
#define TENSION_H

#include <Arduino.h>

/**
 * Initialise le capteur de tension
 * @return true si succès, false si échec
 */
bool tensionInit();

/**
 * Lit la tension actuelle
 * @return Tension en Newtons (ou -1 si erreur)
 */
float tensionRead();

/**
 * Calibre le capteur de tension
 * @param referenceWeight Poids de référence en Newtons
 * @return true si succès, false si échec
 */
bool tensionCalibrate(float referenceWeight);

/**
 * Vérifie si le capteur de tension est fonctionnel
 * @return true si fonctionnel, false sinon
 */
bool tensionIsHealthy();

/**
 * Mise à jour périodique du capteur de tension
 * Fonction thread-safe compatible avec FreeRTOS
 * @return true si succès, false si échec
 */
bool updateTensionSensor();

#endif // TENSION_H