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
    MENU_SYSTEM = 4
};

// Structure pour les éléments de menu
struct MenuItem {
    char text[20];
    void (*callback)();
};

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Initialisation
    bool begin();
    
    // Gestion de l'affichage
    void clear();
    void updateDisplay();
    void updateMainDisplay();
    void updateDirectionTrimDisplay(int direction, int trim);
    void updateLineLengthDisplay(int length);
    void displayMessage(const char* title, const char* message);
    void displayWiFiInfo(const String& ssid, const IPAddress& ip);
    void displaySystemStats();
    
    // Gestion des menus
    void showMenu(MenuState menu);
    void menuUp();
    void menuDown();
    void menuSelect();
    void menuBack();
    
    // Gestion des boutons
    void checkButtons();
    bool isButtonPressed(uint8_t buttonId);
    
    // Getters & Setters
    DisplayState getCurrentDisplayState() const { return currentDisplayState; }
    void setDisplayNeedsUpdate(bool update) { displayNeedsUpdate = update; }
    bool isInitialized() const { return lcdInitialized; }
    
private:
    // Composants matériels
    LiquidCrystal_I2C lcd;
    
    // État de l'interface
    bool lcdInitialized;
    bool displayNeedsUpdate;
    DisplayState currentDisplayState;
    MenuState currentMenu;
    uint8_t currentMenuSelection;
    
    // État des boutons
    bool buttonStates[4];
    bool lastButtonStates[4];
    unsigned long lastDebounceTime[4];
    unsigned long lastButtonCheck;
    
    // Temps de mise à jour
    unsigned long lastDisplayUpdate;
    unsigned long lastDisplayCheck;
    
    // Fonctions d'aide
    void createCustomChars();
    void centerText(uint8_t row, const char* text);
    void printMenuItem(uint8_t row, const char* text, bool selected);
    void drawProgressBar(uint8_t row, uint8_t percent);
    void drawDirection(uint8_t row, int value);
    void checkDisplayStatus();
};

class DisplayManager {
public:
    DisplayManager();
    bool isInitialized();
    void setupI2C();
    bool initLCD();
    void displayMessage(const char* title, const char* message);
    void displayWiFiInfo(const char* ssid, IPAddress ip);
    void displayWelcomeScreen(bool simpleMode);
    void createCustomChars();
    void updateMainDisplay();
    void displayOTAProgress(size_t current, size_t total);
    void displayOTAStatus(bool success);
};

#endif // UI_MANAGER_H