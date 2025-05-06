#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi.h>

// Constantes pour l'écran LCD
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define I2C_SDA 21
#define I2C_SCL 22

/**
 * Classe de gestion de l'affichage LCD
 * Gère l'écran LCD 20x4 avec interface I2C
 */
class DisplayManager {
public:
    // Constructeur & destructeur
    DisplayManager();
    ~DisplayManager();
    
    // Initialisation
    void setupI2C();
    bool initLCD();
    void createCustomChars();
    
    // Vérification
    bool isInitialized() { return lcdInitialized; }
    
    // Affichage de base
    void clear();
    void displayMessage(const char* title, const char* message);
    void updateMainDisplay();
    
    // Affichages spécialisés
    void displayWiFiInfo(const char* ssid, IPAddress ip);
    void displayWelcomeScreen(bool simpleMode);
    void displaySystemStats();
    void displayOTAProgress(size_t current, size_t total);
    void displayOTAStatus(bool success);
    
private:
    LiquidCrystal_I2C lcd;
    bool lcdInitialized;
    
    // Méthodes utilitaires
    void centerText(uint8_t row, const char* text);
    void drawProgressBar(uint8_t row, uint8_t percent);
};