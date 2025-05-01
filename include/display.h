/*
  -----------------------
  Kite PiloteV3 - Module d'affichage
  -----------------------
  
  Module de gestion de l'affichage sur écran ILI9341 pour le système Kite PiloteV3.
  
  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include "config.h"  // Fichier de configuration centralisé

// Ces constantes sont maintenant définies dans config.h
// On les utilise directement sans redéfinition locale

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
    void setupSPI();
    void setupI2C();
    bool initTFT();
    bool initTouch();
    void displayWelcomeScreen();
    
    // Fonctions d'affichage
    void displayMessage(const String& title, const String& message, bool clear = true);
    void updateDisplayRotation(const String& ssid, const IPAddress& ip);
    void displayOTAProgress(size_t current, size_t final);
    void displayOTAStatus(bool success);
    
    // Fonctions de contrôle et vérification
    void checkDisplayStatus();
    bool isInitialized() const;
    bool isTouchInitialized() const;
    
    // Accès à l'écran tactile
    TS_Point getTouch();
    bool touched();
    
    // Accès direct à l'objet TFT pour l'interface tactile
    Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

  private:
    Adafruit_FT6206 ctp;                  // Contrôleur tactile capacitif
    bool tftInitialized;                 // État d'initialisation de l'écran TFT
    bool touchInitialized;               // État d'initialisation de l'écran tactile
    unsigned long lastDisplayUpdate;     // Dernier rafraîchissement d'affichage
    unsigned long lastDisplayCheck;      // Dernière vérification de l'écran
    int currentDisplayState;             // État actuel de rotation de l'affichage
    
    // Fonctions utilitaires privées
    void drawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const String& label, uint16_t color);
};

#endif // DISPLAY_H
