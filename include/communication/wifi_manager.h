#pragma once

#include <Arduino.h>
#include <WiFi.h>

/**
 * Classe de gestion des connexions WiFi
 * Gère les connexions, reconnexions et l'état du WiFi
 */
class WiFiManager {
public:
    // Constructeur et destructeur
    WiFiManager();
    ~WiFiManager();
    
    // Initialisation
    bool begin(const char* ssid, const char* password, uint32_t timeout = 10000);
    void stop();
    
    // Gestion de la connexion
    bool isConnected();
    bool reconnect(uint32_t timeout = 10000);
    void setCredentials(const char* ssid, const char* password);
    
    // Informations sur la connexion
    String getSSID();
    IPAddress getLocalIP();
    int32_t getRSSI();
    
    // Mode AP (Point d'accès)
    bool startAP(const char* ssid, const char* password = NULL);
    void stopAP();
    bool isAPActive();

    // FSM handling
    void handleFSM();
    
private:
    char ssid[33];           // SSID du réseau (max 32 caractères + null)
    char password[65];       // Mot de passe (max 64 caractères + null)
    bool connected;          // État de connexion
    bool apActive;           // Mode AP actif
    unsigned long lastConnectAttempt; // Timestamp de la dernière tentative
    uint32_t timeout;        // Timeout for operations
    
    // Méthodes privées
    void updateConnectionStatus();
};

// Déclaration explicite de la fonction wifiManagerInit
void wifiManagerInit();