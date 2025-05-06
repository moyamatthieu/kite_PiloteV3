/*
  -----------------------
  Kite PiloteV3 - Module WiFiManager (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire de connexions WiFi.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "communication/wifi_manager.h"
#include "utils/logging.h"
#include "core/config.h"

/**
 * Constructeur - initialise les variables membres
 */
WiFiManager::WiFiManager() : connected(false), apActive(false), lastConnectAttempt(0) {
  // Initialiser les chaînes avec des valeurs vides
  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));
}

/**
 * Destructeur - arrête le WiFi
 */
WiFiManager::~WiFiManager() {
  stop();
}

/**
 * Initialise la connexion WiFi
 * @param ssid SSID du réseau WiFi
 * @param password Mot de passe du réseau WiFi
 * @param timeout Délai d'attente en millisecondes
 * @return true si la connexion réussit, false sinon
 */
bool WiFiManager::begin(const char* ssid, const char* password, uint32_t timeout) {
  if (strlen(ssid) == 0) {
    LOG_ERROR("WIFI", "SSID vide");
    return false;
  }
  
  // Enregistrer les credentials
  strncpy(this->ssid, ssid, sizeof(this->ssid) - 1);
  strncpy(this->password, password, sizeof(this->password) - 1);
  
  LOG_INFO("WIFI", "Connexion au réseau %s...", ssid);
  
  // Configuration du mode WiFi en mode station
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Attendre la connexion avec timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
    delay(100);
  }
  
  // Vérifier si la connexion a réussi
  connected = (WiFi.status() == WL_CONNECTED);
  lastConnectAttempt = millis();
  
  if (connected) {
    IPAddress ip = WiFi.localIP();
    LOG_INFO("WIFI", "Connecté à %s, IP: %d.%d.%d.%d", 
             ssid, ip[0], ip[1], ip[2], ip[3]);
  } else {
    LOG_ERROR("WIFI", "Échec de connexion au réseau %s", ssid);
  }
  
  return connected;
}

/**
 * Arrête la connexion WiFi
 */
void WiFiManager::stop() {
  if (apActive) {
    stopAP();
  }
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  connected = false;
  
  LOG_INFO("WIFI", "WiFi arrêté");
}

/**
 * Vérifie si le WiFi est connecté
 * @return true si connecté, false sinon
 */
bool WiFiManager::isConnected() {
  updateConnectionStatus();
  return connected;
}

/**
 * Tente une reconnexion au réseau WiFi
 * @param timeout Délai d'attente en millisecondes
 * @return true si la reconnexion réussit, false sinon
 */
bool WiFiManager::reconnect(uint32_t timeout) {
  if (strlen(ssid) == 0) {
    LOG_ERROR("WIFI", "Tentative de reconnexion sans SSID défini");
    return false;
  }
  
  // Éviter des tentatives trop fréquentes (au moins 5 secondes entre les tentatives)
  if (millis() - lastConnectAttempt < 5000) {
    return connected;
  }
  
  LOG_INFO("WIFI", "Reconnexion au réseau %s...", ssid);
  
  // Configuration du mode WiFi en mode station
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Attendre la connexion avec timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
    delay(100);
  }
  
  // Vérifier si la connexion a réussi
  connected = (WiFi.status() == WL_CONNECTED);
  lastConnectAttempt = millis();
  
  if (connected) {
    IPAddress ip = WiFi.localIP();
    LOG_INFO("WIFI", "Reconnecté à %s, IP: %d.%d.%d.%d", 
             ssid, ip[0], ip[1], ip[2], ip[3]);
  } else {
    LOG_ERROR("WIFI", "Échec de reconnexion au réseau %s", ssid);
  }
  
  return connected;
}

/**
 * Définit les credentials WiFi sans se connecter
 * @param ssid SSID du réseau WiFi
 * @param password Mot de passe du réseau WiFi
 */
void WiFiManager::setCredentials(const char* ssid, const char* password) {
  strncpy(this->ssid, ssid, sizeof(this->ssid) - 1);
  strncpy(this->password, password, sizeof(this->password) - 1);
  LOG_INFO("WIFI", "Credentials WiFi mis à jour pour %s", ssid);
}

/**
 * Récupère le SSID du réseau WiFi
 * @return SSID du réseau
 */
String WiFiManager::getSSID() {
  return String(ssid);
}

/**
 * Récupère l'adresse IP locale
 * @return Adresse IP locale
 */
IPAddress WiFiManager::getLocalIP() {
  return WiFi.localIP();
}

/**
 * Récupère la force du signal WiFi
 * @return Force du signal en dBm
 */
int32_t WiFiManager::getRSSI() {
  if (!connected) {
    return -100; // Valeur faible par défaut
  }
  return WiFi.RSSI();
}

/**
 * Démarre le mode point d'accès
 * @param ssid SSID du point d'accès
 * @param password Mot de passe du point d'accès (NULL pour un réseau ouvert)
 * @return true si le démarrage réussit, false sinon
 */
bool WiFiManager::startAP(const char* ssid, const char* password) {
  // Arrêter le mode client s'il est actif
  if (connected) {
    WiFi.disconnect();
    connected = false;
  }
  
  LOG_INFO("WIFI", "Démarrage du point d'accès %s...", ssid);
  
  // Configurer le mode AP
  WiFi.mode(WIFI_AP);
  
  // Démarrer le point d'accès
  bool success;
  if (password == NULL || strlen(password) == 0) {
    success = WiFi.softAP(ssid);
  } else {
    success = WiFi.softAP(ssid, password);
  }
  
  if (success) {
    apActive = true;
    IPAddress apIP = WiFi.softAPIP();
    LOG_INFO("WIFI", "Point d'accès démarré. IP: %d.%d.%d.%d", 
             apIP[0], apIP[1], apIP[2], apIP[3]);
  } else {
    LOG_ERROR("WIFI", "Échec de démarrage du point d'accès");
  }
  
  return success;
}

/**
 * Arrête le mode point d'accès
 */
void WiFiManager::stopAP() {
  if (!apActive) {
    return;
  }
  
  WiFi.softAPdisconnect(true);
  apActive = false;
  
  LOG_INFO("WIFI", "Point d'accès arrêté");
}

/**
 * Vérifie si le mode point d'accès est actif
 * @return true si le mode AP est actif, false sinon
 */
bool WiFiManager::isAPActive() {
  return apActive;
}

/**
 * Met à jour l'état de connexion WiFi
 */
void WiFiManager::updateConnectionStatus() {
  // Mettre à jour l'état de connexion
  connected = (WiFi.status() == WL_CONNECTED);
}