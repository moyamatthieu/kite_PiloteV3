/*
  -----------------------
  Kite PiloteV3 - Module d'affichage
  -----------------------
  
  Module de gestion de l'affichage sur écran OLED SSD1306 pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 30 avril 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Paramètres de l'écran OLED
#define SCREEN_WIDTH      128      // Largeur de l'écran en pixels
#define SCREEN_HEIGHT     64       // Hauteur de l'écran en pixels
#define OLED_RESET        -1       // Pin de reset (-1 = pas utilisé)
#define SCREEN_ADDRESS    0x3C     // Adresse I2C de l'écran (typiquement 0x3C ou 0x3D)
#define I2C_SDA_PIN       21       // Pin SDA pour I2C
#define I2C_SCL_PIN       22       // Pin SCL pour I2C
#define I2C_CLOCK_SPEED   100000   // Fréquence de l'horloge I2C (100kHz pour stabilité)

// Paramètres d'affichage
const uint8_t TEXT_SIZE_TITLE = 1;          // Taille du texte pour les titres
const uint8_t TEXT_SIZE_CONTENT = 1;        // Taille du texte pour le contenu
const uint16_t DISPLAY_ROTATION_INTERVAL = 5000;  // Intervalle de rotation des écrans (ms)
const uint16_t DISPLAY_CHECK_INTERVAL = 30000;    // Intervalle de vérification de l'écran (ms)

// Nombre maximum de tentatives d'initialisation
const uint8_t MAX_INIT_ATTEMPTS = 3;         // Nombre maximal de tentatives d'initialisation
const uint16_t INIT_RETRY_DELAY = 500;       // Délai entre les tentatives d'initialisation (ms)

// États d'affichage pour la rotation
enum DisplayState {
  DISPLAY_WIFI_INFO = 0,
  DISPLAY_IP_ADDRESS = 1,
  DISPLAY_OTA_INSTRUCTIONS = 2,
  DISPLAY_SYSTEM_STATS = 3,
  DISPLAY_STATE_COUNT = 4 // Nombre total d'états d'affichage
};

class DisplayManager {
  public:
    // Constructeur et destructeur
    DisplayManager();
    ~DisplayManager();
    
    // Fonctions d'initialisation
    void setupI2C();
    bool initOLED();
    void displayWelcomeScreen();
    
    // Fonctions d'affichage
    void displayMessage(const String& title, const String& message, bool clear = true);
    void updateDisplayRotation(const String& ssid, const IPAddress& ip);
    void displayOTAProgress(size_t current, size_t final);
    void displayOTAStatus(bool success);
    
    // Fonctions de contrôle et vérification
    void checkDisplayStatus();
    bool isInitialized() const;
    
  private:
    Adafruit_SSD1306 display;           // Objet pour contrôler l'écran OLED
    bool oledInitialized;               // État d'initialisation de l'écran OLED
    unsigned long lastDisplayUpdate;    // Dernier rafraîchissement d'affichage
    unsigned long lastDisplayCheck;     // Dernière vérification de l'écran
    int currentDisplayState;            // État actuel de rotation de l'affichage
};

#endif // DISPLAY_H