#pragma once

#include <Arduino.h>
#include "hardware/io/display_manager.h"
#include "../../core/config.h" // Inclure config.h pour les définitions de broches

// Define button constants to resolve undefined identifier errors
#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_SELECT 2
#define BUTTON_BACK 3

/**
 * Classe de gestion des boutons de l'interface utilisateur
 * Gère les interactions avec les boutons physiques et les événements associés
 */
class ButtonUIManager {
public:
    // Constructeur et destructeur
    ButtonUIManager(DisplayManager* display);
    ~ButtonUIManager();
    
    // Initialisation
    bool begin();
    
    // Mise à jour de l'état des boutons
    void update();
    
    // État actuel des boutons
    bool isUpPressed();
    bool isDownPressed();
    bool isSelectPressed();
    bool isBackPressed();
    
    // Événements de boutons
    bool wasUpPressed();
    bool wasDownPressed();
    bool wasSelectPressed();
    bool wasBackPressed();
    
    // Gestion des événements longs
    bool isLongPress(uint8_t buttonPin);

    // Lecture de l'état d'un bouton avec debouncing
    bool readButton(uint8_t pin);
    
private:
    DisplayManager* displayManager;
    
    // États des boutons
    bool currentButtonState[4];
    bool previousButtonState[4];
    unsigned long buttonPressTime[4];
    
    // Configuration des broches
    void setupPins();
};