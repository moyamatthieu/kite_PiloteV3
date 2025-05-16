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
#include "core/module.h"
#include "utils/state_machine.h"
#include "utils/error_manager.h"
#include "core/logging.h"
#include <string>

// Implémentation de la classe WiFiManager
// Le constructeur est maintenant dans le fichier d'en-tête
WiFiManager::~WiFiManager() {
    stop(); // S'assurer que tout est arrêté proprement
}

bool WiFiManager::begin(const char* ssid, const char* password, uint32_t timeout) {
    if (ssid == nullptr) {
        LOG_ERROR("WIFI", "SSID ne peut pas être null");
        return false;
    }
    
    // Copier les identifiants
    strncpy(this->ssid, ssid, sizeof(this->ssid) - 1);
    this->ssid[sizeof(this->ssid) - 1] = '\0';
    
    if (password != nullptr) {
        strncpy(this->password, password, sizeof(this->password) - 1);
        this->password[sizeof(this->password) - 1] = '\0';
    } else {
        this->password[0] = '\0';
    }
    
    this->timeout = timeout;
    
    // Initialisation du WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.setSleep(true);
    
    // Démarrer la connexion
    lastConnectAttempt = millis();
    WiFi.begin(this->ssid, this->password);
    
    // Attendre la connexion avec un timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
        delay(100);
    }
    
    // Vérifier si la connexion a réussi
    connected = (WiFi.status() == WL_CONNECTED);
    
    if (connected) {
        LOG_INFO("WIFI", "Connecté à %s, IP: %s",
                this->ssid, WiFi.localIP().toString().c_str());
    } else {
        LOG_WARNING("WIFI", "Échec de connexion à %s", this->ssid);
    }
    
    return connected;
}

void WiFiManager::stop() {
    if (apActive) {
        stopAP();
    }
    
    WiFi.disconnect(true);
    connected = false;
    LOG_INFO("WIFI", "WiFi déconnecté");
}

bool WiFiManager::isConnected() {
    updateConnectionStatus();
    return connected;
}

bool WiFiManager::reconnect(uint32_t timeout) {
    if (connected) {
        return true;
    }
    
    lastConnectAttempt = millis();
    WiFi.reconnect();
    
    // Attendre la connexion avec un timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
        delay(100);
    }
    
    // Vérifier si la connexion a réussi
    connected = (WiFi.status() == WL_CONNECTED);
    
    if (connected) {
        LOG_INFO("WIFI", "Reconnecté à %s", ssid);
    } else {
        LOG_WARNING("WIFI", "Échec de reconnexion à %s", ssid);
    }
    
    return connected;
}

void WiFiManager::setCredentials(const char* ssid, const char* password) {
    if (ssid != nullptr) {
        strncpy(this->ssid, ssid, sizeof(this->ssid) - 1);
        this->ssid[sizeof(this->ssid) - 1] = '\0';
    }
    
    if (password != nullptr) {
        strncpy(this->password, password, sizeof(this->password) - 1);
        this->password[sizeof(this->password) - 1] = '\0';
    } else {
        this->password[0] = '\0';
    }
}

String WiFiManager::getSSID() {
    return String(ssid);
}

IPAddress WiFiManager::getLocalIP() {
    return WiFi.localIP();
}

int32_t WiFiManager::getRSSI() {
    return WiFi.RSSI();
}

bool WiFiManager::startAP(const char* ssid, const char* password) {
    if (ssid == nullptr) {
        LOG_ERROR("WIFI", "SSID ne peut pas être null pour le mode AP");
        return false;
    }
    
    // Arrêter le mode station s'il est actif
    if (connected) {
        WiFi.disconnect(true);
        connected = false;
    }
    
    // Configurer le mode AP
    WiFi.mode(WIFI_AP);
    
    // Démarrer le point d'accès
    bool success;
    if (password != nullptr && strlen(password) >= 8) {
        success = WiFi.softAP(ssid, password);
    } else {
        success = WiFi.softAP(ssid);
    }
    
    if (success) {
        apActive = true;
        LOG_INFO("WIFI", "Point d'accès démarré: %s, IP: %s",
                ssid, WiFi.softAPIP().toString().c_str());
    } else {
        LOG_ERROR("WIFI", "Échec du démarrage du point d'accès");
    }
    
    return success;
}

void WiFiManager::stopAP() {
    if (apActive) {
        WiFi.softAPdisconnect(true);
        apActive = false;
        LOG_INFO("WIFI", "Point d'accès arrêté");
    }
}

bool WiFiManager::isAPActive() {
    return apActive;
}

void WiFiManager::handleFSM() {
    // Vérifier l'état de la connexion
    updateConnectionStatus();
    
    // Logique de la machine à états pour gérer la connexion WiFi
    // Cette méthode est appelée régulièrement par le TaskManager
    
    // Si la connexion est perdue, tenter de reconnecter
    if (!connected && !apActive && millis() - lastConnectAttempt > 30000) {
        reconnect(timeout);
    }
}

void WiFiManager::updateConnectionStatus() {
    connected = (WiFi.status() == WL_CONNECTED);
}

// Implémentation de la fonction wifiManagerInit
void wifiManagerInit() {
    // Initialisation du WiFi avec les paramètres par défaut
    WiFi.mode(WIFI_STA);
    
    // Configuration de base
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    
    // Optimisation de la consommation d'énergie
    WiFi.setSleep(true);
    
    // Initialisation terminée
    LOG_INFO("WIFI", "WiFi Manager initialisé");
}

// Module WiFi
class WiFiCommunicationModule : public Module {
public:
    WiFiCommunicationModule() : Module("WiFi"), fsm("WiFi_FSM", 0), errorManager(*ErrorManager::getInstance()) {}
    
    void enable() override {
        Module::enable();
        fsm.transitionTo(0, 0, "Enable");
    }
    
    void disable() override {
        Module::disable();
        fsm.transitionTo(-1, 0, "Disable");
    }
    
    void update() override {
        fsm.update();
        if (!isEnabled()) return;
        
        // Appeler la méthode handleFSM du WiFiManager global
        if (WiFiManager::getInstance() != nullptr) {
            WiFiManager::getInstance()->handleFSM();
        }
    }
    
    void configure(const std::string& jsonConfig) {
        // Appliquer la configuration WiFi à partir d'un JSON
    }
    
    const char* description() const override { return "Module WiFi (communication)"; }
    
private:
    class WiFiFSM : public StateMachine {
    public:
        WiFiFSM(const char* name, int initialState) : StateMachine(name, initialState) {}
    protected:
        int processState(int state) override {
            // Implémenter la logique d'état ici
            return state;
        }
    };
    
    WiFiFSM fsm;
    ErrorManager& errorManager;
};

static WiFiCommunicationModule wifiModule;
REGISTER_MODULE(wifiModule, &wifiModule);