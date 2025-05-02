/*
  -----------------------
  Kite PiloteV3 - Module de gestion des potentiomètres
  -----------------------
  
  Module de gestion des potentiomètres pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef POTENTIOMETER_MANAGER_H
#define POTENTIOMETER_MANAGER_H

#include <Arduino.h>
#include "config.h"  // Fichier de configuration centralisé

// Structure pour stocker les valeurs des potentiomètres
typedef struct {
  int rawValue;      // Valeur brute du potentiomètre (0-4095)
  int smoothValue;   // Valeur lissée
  int mappedValue;   // Valeur mise à l'échelle (-100 à +100 ou 0 à 100)
  int lastRawValue;  // Dernière valeur brute pour détecter les changements
  bool hasChanged;   // Indique si la valeur a changé depuis la dernière lecture
} PotentiometerState;

class PotentiometerManager {
  public:
    // Constructeur et destructeur
    PotentiometerManager();
    ~PotentiometerManager();
    
    // Initialisation
    void begin();
    
    // Fonctions de lecture des potentiomètres
    void updatePotentiometers();
    
    // Fonctions d'accès aux valeurs
    int getDirection();      // Obtient la valeur de direction (-100 à +100)
    int getTrim();           // Obtient la valeur de trim (-100 à +100)
    int getLineLength();     // Obtient la valeur de longueur de ligne (0 à 100)
    int getLastLineLength(); // Obtient la dernière valeur de longueur de ligne
    
    // Vérification de changement
    bool hasDirectionChanged();
    bool hasTrimChanged();
    bool hasLineLengthChanged();
    bool hasAnyPotChanged();
    
    // Calibration
    void calibrate();
    void setCalibrationValues(int dirMin, int dirMax, int trimMin, int trimMax, int lengthMin, int lengthMax);
    
    // Gestion du pilote automatique
    void setAutoPilotMode(bool enabled);
    bool isAutoPilotEnabled();
    void checkAutoPilotStatus(); // Vérifie si un potentiomètre a été ajusté et désactive le pilote automatique si nécessaire
    
  private:
    PotentiometerState direction;  // État du potentiomètre de direction
    PotentiometerState trim;       // État du potentiomètre de trim
    PotentiometerState lineLength; // État du potentiomètre de longueur de ligne
    
    // Valeurs de calibration
    int directionMin;      // Valeur minimale du potentiomètre de direction
    int directionMax;      // Valeur maximale du potentiomètre de direction
    int trimMin;           // Valeur minimale du potentiomètre de trim
    int trimMax;           // Valeur maximale du potentiomètre de trim
    int lineLengthMin;     // Valeur minimale du potentiomètre de longueur de ligne
    int lineLengthMax;     // Valeur maximale du potentiomètre de longueur de ligne
    
    unsigned long lastUpdateTime;  // Horodatage de la dernière mise à jour
    
    // Variables pour le pilote automatique
    bool autoPilotEnabled;        // État du pilote automatique
    int autoPilotDirection;       // Valeur de direction en mode pilote automatique
    int autoPilotTrim;            // Valeur de trim en mode pilote automatique
    int autoPilotLineLength;      // Valeur de longueur de ligne en mode pilote automatique
    
    // Fonctions utilitaires
    int readPotentiometer(uint8_t pin);  // Lecture d'un potentiomètre
    int applyDeadzone(int value, int center = ADC_RESOLUTION / 2);  // Applique une zone morte
    int smoothValue(int newValue, int oldValue);  // Lissage des valeurs
};

#endif // POTENTIOMETER_MANAGER_H