/*
  -----------------------
  Kite PiloteV3 - Module UI Manager
  -----------------------
  
  Gestionnaire unifié de l'interface utilisateur matérielle.
  Combine l'affichage LCD, les boutons et les entrées.
  
  Version: 2.0.0
  Date: 5 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "../../core/config.h"
#include "display_manager.h"

#ifndef MAX_BUTTONS
#define MAX_BUTTONS 3  // Nombre maximum de boutons
#endif

// Constantes pour la gestion de l'interface utilisateur
// Utilisons les constantes existantes définies dans config.h
// #define DISPLAY_UPDATE_INTERVAL 250   // Remplacé par la constante de config.h (150ms)
// #define DISPLAY_CHECK_INTERVAL 5000   // Remplacé par la constante de config.h (45000ms)

// États d'affichage
enum DisplayState {
    DISPLAY_MAIN = 0,
    DISPLAY_DIRECTION_TRIM = 1,
    DISPLAY_LINE_LENGTH = 2,
    DISPLAY_WIFI_INFO = 3,
    DISPLAY_SYSTEM_STATS = 4,
    DISPLAY_STATE_COUNT = 5
};

// États du menu
enum MenuState {
    MENU_MAIN = 0,
    MENU_SETTINGS = 1,
    MENU_CONTROL = 2,
    MENU_WIFI = 3,
    MENU_SYSTEM = 4,
    MENU_MODULES = 5 // Ajouté pour la gestion dynamique des modules
};

// Gestionnaire unifié de l'interface utilisateur
class UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Initialisation
    bool begin();
    
    // Gestion de l'affichage
    void clear();
    void centerText(uint8_t row, const char* text);
    void updateDisplay();
    void updateMainDisplay();
    void updateDirectionTrimDisplay(int direction, int trim);
    void updateLineLengthDisplay(int length);
    void displayMessage(const char* title, const char* message);
    void displayWiFiInfo(const String& ssid, const IPAddress& ip);
    void displaySystemStats();
    void createCustomChars();
    
    // Méthodes utilitaires pour l'affichage
    void drawProgressBar(uint8_t row, uint8_t percent);
    void drawDirection(uint8_t row, int value);
    
    // Gestion des menus
    void showMenu(MenuState menu);
    void printMenuItem(uint8_t row, const char* text, bool selected);
    void menuUp();
    void menuDown();
    void menuSelect();
    void menuBack();
    
    // Gestion des boutons
    void checkButtons();
    bool isButtonPressed(uint8_t buttonId);
    
    // Surveillance et récupération de l'écran LCD
    void checkDisplayStatus();
    
    // Getters & Setters
    DisplayState getCurrentDisplayState() const { return currentDisplayState; }
    void setDisplayState(DisplayState state) { 
        currentDisplayState = state;
        displayNeedsUpdate = true;
    }
    void setDisplayNeedsUpdate(bool update) { displayNeedsUpdate = update; }
    bool isInitialized() const { return lcdInitialized; }
    
private:
    // Composants matériels
    LiquidCrystal_I2C lcd;
    DisplayManager display; // Gestionnaire de l'écran LCD
    
    // État de l'interface
    bool lcdInitialized;
    bool displayNeedsUpdate;
    DisplayState currentDisplayState;
    MenuState currentMenu;
    uint8_t currentMenuSelection;
    
    // États des boutons
    bool buttonStates[MAX_BUTTONS];
    bool lastButtonStates[MAX_BUTTONS];
    unsigned long lastDebounceTime[MAX_BUTTONS];
    
    // Horodatages pour les vérifications périodiques
    unsigned long lastDisplayUpdate;
    unsigned long lastButtonCheck;
    unsigned long lastButtonCheckTime; // Pour l'anti-rebond des boutons
    unsigned long lastDisplayCheck;    // Pour la vérification de l'état de l'écran
};

// Instance globale
extern UIManager uiManager;

#endif // UI_MANAGER_H
