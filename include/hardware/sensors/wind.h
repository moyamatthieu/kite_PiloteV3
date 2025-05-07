/*
  -----------------------
  Kite PiloteV3 - Module Wind Sensor (Interface)
  -----------------------
  
  Interface pour la gestion du capteur de vent.
  
  Version: 1.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef WIND_H
#define WIND_H

#include <Arduino.h>

/**
 * Structure pour les données de vent
 */
typedef struct {
    float speed;         // Vitesse du vent en m/s
    float direction;     // Direction du vent en degrés (0-359)
    float gust;          // Rafale maximale en m/s
    uint32_t timestamp;  // Horodatage de la mesure
    bool isValid;        // Indique si les données sont valides
} WindData;

/**
 * Initialise le capteur de vent
 * @return true si succès, false si échec
 */
bool windInit();

/**
 * Lit les données actuelles du vent
 * @return Structure WindData avec les mesures actuelles
 */
WindData windRead();

/**
 * Calibre le capteur de vent
 * @return true si succès, false si échec
 */
bool windCalibrate();

/**
 * Vérifie si le capteur de vent est fonctionnel
 * @return true si fonctionnel, false sinon
 */
bool windIsHealthy();

/**
 * Mise à jour périodique du capteur de vent
 * Fonction thread-safe compatible avec FreeRTOS
 * @return true si succès, false si échec
 */
bool updateWindSensor();

#endif // WIND_H