/*
  -----------------------
  Kite PiloteV3 - Module Button UI Manager (Implémentation)
  -----------------------
  
  Implémentation de la gestion des boutons de l'interface utilisateur.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/io/button_ui.h"
#include "utils/logging.h"

// Constantes locales
#define DEBOUNCE_DELAY 50    // Délai d'antirebond en ms
#define LONG_PRESS_TIME 1000 // Temps pour considérer une pression longue en ms

/**
 * Constructeur
 * @param display Pointeur vers le gestionnaire d'affichage
 */
ButtonUIManager::ButtonUIManager(DisplayManager* display) : displayManager(display) {
    // Initialisation des états des boutons
    for (int i = 0; i < 4; i++) {
        currentButtonState[i] = false;
        previousButtonState[i] = false;
        buttonPressTime[i] = 0;
    }
}

/**
 * Destructeur
 */
ButtonUIManager::~ButtonUIManager() {
    // Rien à libérer
}

/**
 * Initialise les boutons
 * @return true si l'initialisation est réussie
 */
bool ButtonUIManager::begin() {
    LOG_INFO("BTN", "Initialisation des boutons");
    
    // Configuration des broches
    setupPins();
    
    // Mettre à jour l'état initial des boutons
    update();
    
    LOG_INFO("BTN", "Boutons initialisés avec succès");
    return true;
}

/**
 * Configure les broches des boutons
 */
void ButtonUIManager::setupPins() {
    // Configuration des broches en entrée avec pull-up
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_BACK_PIN, INPUT_PULLUP);
}

/**
 * Met à jour l'état des boutons
 */
void ButtonUIManager::update() {
    // Enregistrer les états précédents
    for (int i = 0; i < 4; i++) {
        previousButtonState[i] = currentButtonState[i];
    }
    
    // Mettre à jour les états actuels
    currentButtonState[0] = readButton(BUTTON_UP_PIN);
    currentButtonState[1] = readButton(BUTTON_DOWN_PIN);
    currentButtonState[2] = readButton(BUTTON_SELECT_PIN);
    currentButtonState[3] = readButton(BUTTON_BACK_PIN);
    
    // Enregistrer le temps de pression pour les boutons nouvellement pressés
    unsigned long currentTime = millis();
    for (int i = 0; i < 4; i++) {
        if (currentButtonState[i] && !previousButtonState[i]) {
            buttonPressTime[i] = currentTime;
        }
    }
}

/**
 * Lit l'état d'un bouton avec antirebond
 * @param pin Broche du bouton à lire
 * @return true si le bouton est pressé
 */
bool ButtonUIManager::readButton(uint8_t pin) {
    // Les boutons sont configurés avec pull-up, donc logique inversée
    // LOW (0) signifie bouton pressé, HIGH (1) signifie relâché
    return digitalRead(pin) == LOW;
}

/**
 * Vérifie si le bouton Up est actuellement pressé
 * @return true si pressé
 */
bool ButtonUIManager::isUpPressed() {
    return currentButtonState[0];
}

/**
 * Vérifie si le bouton Down est actuellement pressé
 * @return true si pressé
 */
bool ButtonUIManager::isDownPressed() {
    return currentButtonState[1];
}

/**
 * Vérifie si le bouton Select est actuellement pressé
 * @return true si pressé
 */
bool ButtonUIManager::isSelectPressed() {
    return currentButtonState[2];
}

/**
 * Vérifie si le bouton Back est actuellement pressé
 * @return true si pressé
 */
bool ButtonUIManager::isBackPressed() {
    return currentButtonState[3];
}

/**
 * Vérifie si le bouton Up vient d'être pressé
 * @return true si pressé durant cette mise à jour
 */
bool ButtonUIManager::wasUpPressed() {
    return currentButtonState[0] && !previousButtonState[0];
}

/**
 * Vérifie si le bouton Down vient d'être pressé
 * @return true si pressé durant cette mise à jour
 */
bool ButtonUIManager::wasDownPressed() {
    return currentButtonState[1] && !previousButtonState[1];
}

/**
 * Vérifie si le bouton Select vient d'être pressé
 * @return true si pressé durant cette mise à jour
 */
bool ButtonUIManager::wasSelectPressed() {
    return currentButtonState[2] && !previousButtonState[2];
}

/**
 * Vérifie si le bouton Back vient d'être pressé
 * @return true si pressé durant cette mise à jour
 */
bool ButtonUIManager::wasBackPressed() {
    return currentButtonState[3] && !previousButtonState[3];
}

/**
 * Vérifie si un bouton est pressé depuis longtemps
 * @param buttonPin Index du bouton (0-3)
 * @return true si la pression est longue
 */
bool ButtonUIManager::isLongPress(uint8_t buttonPin) {
    if (buttonPin >= 4) return false;
    
    // Vérifier si le bouton est actuellement pressé
    if (!currentButtonState[buttonPin]) return false;
    
    // Vérifier la durée de la pression
    unsigned long pressTime = millis() - buttonPressTime[buttonPin];
    return pressTime >= LONG_PRESS_TIME;
}