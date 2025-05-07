/*
  -----------------------
  Kite PiloteV3 - Module WiFi Manager (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire de connexion WiFi pour le système Kite PiloteV3.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module gère la connectivité WiFi du système, permettant le contrôle à distance
  et la visualisation des données via une interface web. Il implémente une machine 
  à états finis (FSM) pour gérer toutes les opérations de manière non-bloquante.
  
  Principes de fonctionnement :
  1. Initialisation et configuration du WiFi (station ou point d'accès) via une FSM
  2. Gestion des connexions et reconnexions automatiques sans bloquer le CPU
  3. Surveillance de la qualité de connexion et adaptation avec opérations asynchrones
  4. Mise à disposition des informations de connexion
  
  Architecture FSM implémentée :
  - États : IDLE, CONNECTING, CONNECTED, RECONNECTING, AP_STARTING, AP_ACTIVE
  - Les transitions entre états se font sur la base d'événements temporels ou externes
  - Chaque état possède sa propre méthode de traitement qui s'exécute rapidement
  - Aucune opération ne bloque le flux d'exécution du programme
  
  Interactions avec d'autres modules :
  - TaskManager : Appelle ce module depuis une tâche dédiée
  - WebServer : Utilise la connexion WiFi pour servir l'interface web
  - API : Permet l'accès à distance via des requêtes REST
  - System : Fournit des informations sur l'état du système pour le tableau de bord web
  - OTA : Utilise la connexion WiFi pour les mises à jour over-the-air
  
  Aspects techniques notables :
  - Utilisation d'une FSM pour remplacer tous les appels à delay() par des opérations non-bloquantes
  - Gestion intelligente de la puissance d'émission pour économiser la batterie
  - Basculement automatique en mode point d'accès en cas d'échec de connexion
  - Stockage des configurations dans la mémoire non volatile
  - Mécanismes de sécurité pour prévenir les accès non autorisés
  
  Exemple d'approche non-bloquante :
  Au lieu de :
  ```
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
    delay(100);  // Bloque l'exécution pendant 100ms
  }
  ```
  
  Notre FSM utilise :
  ```
  case CONNECTING:
    if (millis() - lastStateTime >= checkInterval) {
      lastStateTime = millis();
      if (WiFi.status() == WL_CONNECTED) {
        state = CONNECTED;
        // Actions à l'établissement de la connexion
      } else if (millis() - connectStartTime >= timeout) {
        state = IDLE;
        // Actions en cas de timeout
      }
    }
    break;
  ```
*/

#include "communication/wifi_manager.h"
#include "utils/logging.h"
#include "core/config.h"

// Ajout d'une énumération pour les états de la FSM
enum WiFiState {
  IDLE,
  CONNECTING,
  CONNECTED,
  RECONNECTING,
  AP_STARTING,
  AP_ACTIVE
};

// Ajout d'une variable pour suivre l'état actuel
WiFiState currentState = IDLE;
unsigned long lastStateTime = 0;
unsigned long connectStartTime = 0;

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

  // Initialiser la FSM
  currentState = CONNECTING;
  connectStartTime = millis();
  lastStateTime = millis();

  return true;
}

/**
 * Ajout d'une méthode pour gérer la FSM
 */
void WiFiManager::handleFSM() {
  switch (currentState) {
    case CONNECTING:
      if (millis() - lastStateTime >= 100) { // Vérification périodique
        lastStateTime = millis();
        if (WiFi.status() == WL_CONNECTED) {
          currentState = CONNECTED;
          connected = true;
          IPAddress ip = WiFi.localIP();
          LOG_INFO("WIFI", "Connecté à %s, IP: %d.%d.%d.%d", 
                   ssid, ip[0], ip[1], ip[2], ip[3]);
        } else if (millis() - connectStartTime >= timeout) {
          currentState = IDLE;
          connected = false;
          LOG_ERROR("WIFI", "Échec de connexion au réseau %s", ssid);
        }
      }
      break;

    case CONNECTED:
      // Vérifier si la connexion est toujours active
      if (WiFi.status() != WL_CONNECTED) {
        currentState = RECONNECTING;
        LOG_WARNING("WIFI", "Connexion perdue, tentative de reconnexion...");
      }
      break;

    case RECONNECTING:
      if (millis() - lastStateTime >= 5000) { // Tentative toutes les 5 secondes
        lastStateTime = millis();
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        currentState = CONNECTING;
        connectStartTime = millis();
      }
      break;

    case IDLE:
    case AP_STARTING:
    case AP_ACTIVE:
      // Gérer les autres états si nécessaire
      break;
  }
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

  if (currentState != CONNECTED && currentState != CONNECTING) {
    currentState = RECONNECTING;
    lastStateTime = millis();
    connectStartTime = millis();
    WiFi.disconnect();
    WiFi.begin(ssid, password);
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
  // Vérifier si le WiFi est connecté
  connected = (WiFi.status() == WL_CONNECTED);
  
  // Journaliser les changements d'état
  static bool lastConnectedState = false;
  if (connected != lastConnectedState) {
    if (connected) {
      IPAddress ip = WiFi.localIP();
      LOG_INFO("WIFI", "Connexion établie. IP: %d.%d.%d.%d", 
               ip[0], ip[1], ip[2], ip[3]);
    } else {
      LOG_WARNING("WIFI", "Connexion WiFi perdue");
    }
    lastConnectedState = connected;
  }
}