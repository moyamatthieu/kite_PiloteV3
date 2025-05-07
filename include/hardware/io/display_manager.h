#pragma once

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi.h>
#include "../../core/config.h"

// Constantes pour l'écran LCD
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define I2C_SDA 21
#define I2C_SCL 22

// Constantes pour la gestion de l'écran
#define LCD_CHECK_INTERVAL 10000  // Intervalle de vérification de l'état de l'écran (ms)
#define LCD_AUTO_RECOVERY true    // Réinitialisation automatique en cas de problème

/**
 * Classe de gestion de l'affichage LCD
 * Gère l'écran LCD 20x4 avec interface I2C
 */
class DisplayManager {
private:
    LiquidCrystal_I2C lcd;     // Objet écran LCD I2C
    bool lcdInitialized;       // État d'initialisation de l'écran
    unsigned long lastInitTime; // Dernier temps d'initialisation
    unsigned long lastCheckTime; // Dernier temps de vérification
    unsigned long lastUpdateTime; // Dernier temps de mise à jour de l'affichage
    uint8_t recoveryAttempts;  // Nombre de tentatives de récupération
    uint32_t successfulUpdates; // Compteur de mises à jour réussies
    bool i2cInitialized;       // État d'initialisation du bus I2C
    uint8_t initAttemptCount;  // Nombre de tentatives d'initialisation

    // Ajout d'un buffer LCD pour optimiser les rafraîchissements
    char screenBuffer[LCD_ROWS][LCD_COLS+1]; // +1 pour le caractère nul de fin de chaîne
    char previousBuffer[LCD_ROWS][LCD_COLS+1];
    
    // Méthode pour mettre à jour uniquement les caractères modifiés
    void updateLCDDiff();

public:
    // Constructeur et destructeur
    DisplayManager();
    ~DisplayManager();
    
    // Méthodes d'initialisation
    bool setupI2C();
    bool initLCD();
    void createCustomChars();
    bool checkLCDConnection();
    
    // Méthodes d'affichage de base
    void clear();
    void centerText(uint8_t row, const char* text);
    void updateMainDisplay();
    
    // Méthodes d'affichage spéciales
    void displayMessage(const char* title, const char* message, unsigned long duration = 0);
    void displayWelcomeScreen(bool simpleMode = false);
    void displayWiFiInfo(const char* ssid, IPAddress ip);
    void displayOTAProgress(size_t current, size_t total);
    void displayOTAStatus(bool success);
    void displayLiveStatus(int direction, int trim, int lineLength, bool wifiConnected, unsigned long uptime);
    
    // Méthodes de vérification et récupération
    bool checkAndRecover();
    bool recoverLCD();
    
    // Getters et setters
    bool isInitialized() const { return lcdInitialized; }
};

#endif // DISPLAY_MANAGER_H