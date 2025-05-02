/*
  -----------------------
  Kite PiloteV3 - Module d'affichage
  -----------------------
  
  Module de gestion de l'affichage sur écran LCD 2004 I2C pour le système Kite PiloteV3.
  
  Version: 2.2.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"  // Fichier de configuration centralisé

// États d'affichage pour la rotation des écrans
enum DisplayState {
  DISPLAY_MAIN = 0,              // Écran principal
  DISPLAY_DIRECTION_TRIM = 1,    // Affichage direction et trim
  DISPLAY_LINE_LENGTH = 2,       // Affichage longueur des lignes
  DISPLAY_WIFI_INFO = 3,         // Informations WiFi
  DISPLAY_SYSTEM_STATS = 4,      // Statistiques système
  DISPLAY_STATE_COUNT = 5        // Nombre total d'états d'affichage
};

// États du menu principal
enum MenuState {
  MENU_MAIN = 0,
  MENU_SETTINGS = 1,
  MENU_CONTROL = 2,
  MENU_WIFI = 3,
  MENU_SYSTEM = 4
};

class ButtonUIManager; // Déclaration anticipée

class DisplayManager {
 friend class ButtonUIManager; // Déclaration d'amitié

 public:
    // Constructeur et destructeur
    DisplayManager();
    ~DisplayManager();
    
    // Fonctions d'initialisation
    void setupI2C();
    bool initLCD();
    void displayWelcomeScreen(bool simpleInit = false);
    void createCustomChars();
    
    // Fonctions d'affichage
    void clear();
    void displayMessage(const String& title, const String& message);
    void displayMessage(const char* title, const char* message); // Surcharge optimisée
    void updateMainDisplay();
    void updateDirectionTrimDisplay(int direction, int trim);
    void updateLineLengthDisplay(int length);
    void updateSystemDisplay();
    void displayOTAProgress(size_t current, size_t final);
    void displayOTAStatus(bool success);
    void displayWiFiInfo(const String& ssid, const IPAddress& ip);

    // Fonctions de navigation dans les menus
    void showMenu(MenuState menu);
    void menuUp();
    void menuDown();
    void menuSelect();
    void menuBack();

    // Gestion de l'affichage
    void updateDisplay();        // Mise à jour régulière de l'affichage
    void forceRefresh();         // Force le rafraîchissement de l'écran
    void nextScreen();           // Passe à l'écran suivant dans la rotation
     void checkDisplayStatus();   // Vérifie l'état de l'écran

     // Fonctions de contrôle et vérification
     bool isInitialized() const;
     DisplayState getCurrentDisplayState() const { return static_cast<DisplayState>(currentDisplayState); }

     // Barres de progression et indicateurs
    void drawProgressBar(uint8_t row, uint8_t percent);
    void drawDirection(uint8_t row, int value);    // Indicateur graphique de direction
    void drawLevel(uint8_t row, int value);        // Indicateur de niveau

  private:
    LiquidCrystal_I2C lcd;                // Objet de contrôle de l'écran LCD I2C
    bool lcdInitialized;                  // État d'initialisation de l'écran LCD
    unsigned long lastDisplayUpdate;      // Dernier rafraîchissement d'affichage
    unsigned long lastMenuUpdate;         // Dernière mise à jour du menu
    unsigned long lastDisplayCheck;       // Dernière vérification de l'écran
    int currentDisplayState;              // État actuel de rotation de l'affichage
    bool displayNeedsUpdate;              // Indicateur de besoin de rafraîchissement
    MenuState currentMenu;                // Menu actuel
    int currentMenuSelection;             // Sélection actuelle dans le menu
    
    // Fonctions utilitaires privées
    /**
     * @brief Centre un texte sur une ligne de l'écran LCD
     * @param row Numéro de la ligne (0-3)
     * @param text Texte à centrer
     */
    void centerText(uint8_t row, const char* text);

    /**
     * @brief Affiche un élément de menu sur l'écran LCD
     * @param row Ligne de l'écran (0-3)
     * @param text Texte du menu à afficher
     * @param selected Indique si l'élément est sélectionné (avec un marqueur)
     */
    void printMenuItem(uint8_t row, const char* text, bool selected);

    /**
     * @brief Fait défiler un texte long sur une ligne de l'écran LCD
     * @param row Ligne de l'écran (0-3)
     * @param text Texte à faire défiler
     * @param maxLength Longueur maximale visible
     */
    void scrollLongText(uint8_t row, const char* text, int maxLength);
};

#endif // DISPLAY_H
