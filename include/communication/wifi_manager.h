#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "core/component.h"  // Ajout de l'inclusion pour ManagedComponent

/**
 * Classe de gestion des connexions WiFi
 * Gère les connexions, reconnexions et l'état du WiFi
 */
class WiFiManager : public ManagedComponent {
public:
    // Constructeur et destructeur
    WiFiManager() : ManagedComponent("WiFiManager", true) {}
    ~WiFiManager() override;
    
    // Singleton
    static WiFiManager* getInstance() {
        static WiFiManager instance;
        return &instance;
    }
    
    // Initialisation
    bool begin(const char* ssid, const char* password, uint32_t timeout = 10000);
    void stop();
    
    // Implémentation de l'interface ManagedComponent
    ErrorCode initialize() override {
        setState(ComponentState::INITIALIZING);
        // Utiliser les identifiants stockés ou valeurs par défaut
        const char* defaultSsid = "KitePiloteV3_AP";
        const char* defaultPassword = "kite12345";
        bool result = begin(ssid[0] ? ssid : defaultSsid, 
                           password[0] ? password : defaultPassword);
        setState(result ? ComponentState::IDLE : ComponentState::ERROR);
        return result ? ErrorCode::OK : ErrorCode::COMPONENT_INITIALIZATION_ERROR;
    }
    
    void update() override {
        handleFSM(); // Gère le FSM du wifi
    }
    
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
    friend class WiFiFSM; // Permet à la FSM d'accéder aux membres privés
    
private:
    char ssid[33] = {0};           // SSID du réseau (max 32 caractères + null)
    char password[65] = {0};       // Mot de passe (max 64 caractères + null)
    bool connected = false;          // État de connexion
    bool apActive = false;           // Mode AP actif
    unsigned long lastConnectAttempt = 0; // Timestamp de la dernière tentative
    uint32_t timeout = 30000;        // Timeout for operations
    
    // Méthodes privées
    void updateConnectionStatus();
};

// Déclaration explicite de la fonction wifiManagerInit
void wifiManagerInit();