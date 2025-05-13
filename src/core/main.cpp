/*
  -----------------------
  Kite PiloteV3 - Programme principal
  -----------------------
  
  Point d'entrée principal pour le système de contrôle de kite générateur d'électricité.
  Architecture modulaire optimisée pour ESP32.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce fichier implémente le programme principal du système Kite PiloteV3 qui contrôle
  un cerf-volant générateur d'électricité. Son architecture est basée sur FreeRTOS
  pour une gestion efficace des tâches parallèles.
  
  Le fonctionnement se décompose en plusieurs étapes :
  1. Initialisation du système (GPIO, écran LCD, capteurs, servomoteurs)
  2. Configuration du réseau WiFi et du serveur web
  3. Démarrage des tâches FreeRTOS pour contrôler les différents sous-systèmes
  4. Maintien du système en exécution via la boucle principale
  
  Lors de l'exécution :
  - Le gestionnaire de tâches orchestre l'exécution des différentes fonctionnalités
  - L'interface web permet le contrôle à distance du système
  - Les capteurs fournissent des données en temps réel sur l'état du cerf-volant
  - Les actionneurs ajustent la position du cerf-volant selon les commandes
  - L'écran LCD affiche les informations essentielles
  
  Le système supporte la mise à jour Over-The-Air (OTA) pour faciliter les mises à jour
  sans avoir à connecter physiquement l'appareil.
*/

// === INCLUSIONS DES BIBLIOTHÈQUES EXTERNES ===
#include <WiFi.h>                // Gestion de la connexion WiFi
#include <AsyncTCP.h>            // Communication TCP asynchrone non bloquante
#include <ESPAsyncWebServer.h>   // Serveur HTTP asynchrone pour interface web
#include <ElegantOTA.h>          // Module de mise à jour OTA (Over The Air)
#include <ESP32Servo.h>          // Bibliothèque pour les servomoteurs
#include <FastAccelStepper.h>    // Bibliothèque pour le moteur pas à pas
#include <Wire.h>                // Gestion du bus I2C

// === INCLUSIONS DES MODULES CENTRAUX DU PROJET ===
#include "utils/logging.h"       // Fonctions et macros de journalisation
#include "core/config.h"         // Constantes de configuration globale (pins, timings)
#include "core/system.h"         // Gestion de l'état système et watchdog
#include "core/task_manager.h"   // Orchestration des tâches FreeRTOS

// === INCLUSIONS MODULES HARDWARE ===
// Capteurs
#include "hardware/sensors/imu.h"
#include "hardware/sensors/line_length.h" // Capteur de longueur de ligne
#include "hardware/sensors/tension.h"     // Capteur de tension
#include "hardware/sensors/wind.h"        // Capteur de vent
// Actionneurs
#include "hardware/actuators/servo.h"     // Servomoteurs
#include "hardware/actuators/generator.h" // Actionneur générateur
#include "hardware/actuators/winch.h"     // Treuil
// E/S
#include "hardware/io/ui_manager.h"
#include "hardware/io/button_ui.h"
#include "hardware/io/potentiometer_manager.h"

// === INCLUSIONS MODULES COMMUNICATION ===
#include "communication/kite_webserver.h"
#include "communication/api_manager.h"
#include "communication/wifi_manager.h" // Ajout de l'en-tête wifi_manager.h

// === INCLUSIONS MODULES CONTROL ===
#include "control/autopilot.h"

// === INCLUSIONS MODULES UI ===
#include "ui/dashboard.h"
#include "ui/presenter.h"
#include "ui/webserver.h"                 // Serveur web pour l'interface utilisateur

// === INCLUSIONS MODULES UTILS ===
#include "utils/data_storage.h"           // Gestion du stockage des données
#include "utils/diagnostics.h"            // Diagnostics système et scanI2CBus
#include "utils/terminal.h"               // Terminal distant

// === DÉCLARATION DES OBJETS GLOBAUX ===
DisplayManager display_global;                       // Gestionnaire d'affichage LCD GLOBAL
ButtonUIManager buttonUI(&display_global);           // Gestionnaire de l'interface utilisateur à boutons - Initialisé avec display_global
PotentiometerManager potManager;              // Gestionnaire des potentiomètres - Sera initialisé dans InitTask
AsyncWebServer server(SERVER_PORT);           // Serveur web asynchrone
TaskManager taskManager;                      // Gestionnaire de tâches multiples
UIManager uiManager(&display_global);         // Gestionnaire de l'interface utilisateur - PREND MAINTENANT LE DISPLAY GLOBAL
WiFiManager wifiManager;                      // Gestionnaire WiFi

// Variables d'état système
bool wifiConnected = false;                  // État de la connexion WiFi
uint8_t systemState = 0;                     // État général du système
unsigned long ota_progress_millis = 0;       // Timestamp pour la progression OTA
unsigned long lastSystemCheck = 0;           // Dernière vérification système
// Intervalles de log pour limiter la verbosité
static unsigned long lastSystemStateLogTime = 0;
static unsigned long lastMemoryLogTime = 0;
static const unsigned long SYSTEM_STATE_LOG_INTERVAL_MS = 1000; // 1s
static const unsigned long MEMORY_LOG_INTERVAL_MS = 2000;       // 2s

// === DÉCLARATION DES FONCTIONS ===

// Tâche d'initialisation principale (démarre après le setup)
void initTask(void* parameter);

// Fonctions d'initialisation
void setupWiFi();
void setupServer();
void setupHardware();
void setupTasks();

// Fonctions OTA
void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

// Vérification de l'état du système
void checkSystemState();

// Déclaration de la fonction feedWatchdogs
void feedWatchdogs();

// === IMPLÉMENTATION DES FONCTIONS ===

/**
 * Initialise et connecte le WiFi avec timeout et gestion optimisée
 */
void setupWiFi() {
  if (!display_global.isSuccessfullyInitialized()) {
    LOG_WARNING("WIFI", "Écran LCD non initialisé, affichage WiFi sauté pour certains messages.");
  }
  
  LOG_INFO("WIFI", "Connexion au réseau %s...", WIFI_SSID);
  if (display_global.isSuccessfullyInitialized()) {
    display_global.displayMessage("Système", "Connexion WiFi...");
  }
  
  // Configuration du mode WiFi en mode station
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Attente optimisée de la connexion WiFi avec timeout
  unsigned long startAttemptTime = millis();
  uint8_t dots = 0;
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
    // Clignote la LED pendant l'attente pour feedback visuel
    digitalWrite(LED_PIN, (dots % 2) ? HIGH : LOW);
    
    // Afficher progression mais moins fréquemment
    if (dots % 3 == 0) {
      Serial.print(".");
    }
    
    delay(200); // Réduit à 200ms au lieu de 500ms
    dots++;
  }
  Serial.println("");
  
  // Vérifier si la connexion a réussi
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  
  if (wifiConnected) {
    IPAddress ip = WiFi.localIP();
    LOG_INFO("WIFI", "Connecté à %s, IP: %d.%d.%d.%d", 
             WIFI_SSID, ip[0], ip[1], ip[2], ip[3]);
    
    if (display_global.isSuccessfullyInitialized()) {
      display_global.displayWiFiInfo(WIFI_SSID, WiFi.localIP());
      delay(1000); // Afficher pendant 1 seconde
    }
  } else {
    LOG_ERROR("WIFI", "Échec de connexion au réseau %s", WIFI_SSID);
    if (display_global.isSuccessfullyInitialized()) {
      display_global.displayMessage("Erreur", "Échec WiFi");
      delay(1000);
    }
  }
}

/**
 * Configure et démarre le serveur web avec support OTA (optimisé)
 */
void setupServer() {
#if MODULE_WEBSERVER_ENABLED
  if (!wifiConnected) {
    LOG_WARNING("SERVER", "WiFi non connecté, serveur web non démarré");
    return;
  }
  
  if (display_global.isSuccessfullyInitialized()) {
    display_global.displayMessage("Système", "Démarrage serveur...");
  }
  
  // Définition du mode de fonctionnement du serveur web
  setWebServerMode(false);
  
  // Configuration du serveur web asynchrone
  setupServerRoutes(&server);
  
  // Initialiser l'API REST
  apiInit(server);
  apiEnable(true);
  
  // Démarrer ElegantOTA et configurer les callbacks
  ElegantOTA.begin(&server);
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  
  // Démarrer le serveur web asynchrone
  server.begin();
  LOG_INFO("SERVER", "Serveur HTTP démarré sur le port %d", SERVER_PORT);
  // Afficher l'adresse IP pour accéder à l'interface web
  LOG_INFO("SERVER", "IP: %s", WiFi.localIP().toString().c_str());
  
  if (display_global.isSuccessfullyInitialized()) {
    display_global.displayMessage("Système", "Prêt!");
    delay(1000);
  
    char otaUrl[40];
    snprintf(otaUrl, sizeof(otaUrl), "http://%d.%d.%d.%d/update", 
             WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    display_global.displayMessage("OTA", otaUrl);
    delay(1000);
  }
#else
  LOG_INFO("SERVER", "Serveur web désactivé (MODULE_WEBSERVER_ENABLED=0)");
#endif
}

/**
 * Initialise tous les composants matériels
 */
void setupHardware() {
  // Configurer les GPIO
  pinMode(LED_PIN, OUTPUT);
  // Broches des boutons (config centralisée)
  pinMode(BUTTON_BACK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);

  // LED clignote une fois pour indiquer l'initialisation réussie
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  
  LOG_INFO("GPIO", "GPIOs configurés");
  
  // Initialiser l'écran LCD (objet global display_global)

  LOG_INFO("INIT", "Initialisation de l'écran LCD (global)...");
  unsigned long lcdInitStartTime = millis();
  const unsigned long lcdInitTimeoutMs = 7000; // Augmenté pour tests, était 5000
  bool globalLcdInitialized = false;

  while (millis() - lcdInitStartTime < lcdInitTimeoutMs) {
      display_global.update(); // Pilote la FSM pour l'écran global
      if (display_global.isSuccessfullyInitialized()) {
          globalLcdInitialized = true;
          break;
      }
      // Donner du temps à la FSM pour progresser. setupHardware est appelé depuis initTask.
      if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) { // Vérifier si le scheduler est démarré
        vTaskDelay(pdMS_TO_TICKS(20)); // Réduit pour appels plus fréquents
      } else {
        delay(20); // Fallback si le scheduler n'est pas encore démarré
      }
  }

  if (globalLcdInitialized) {
    LOG_INFO("INIT", "Écran LCD (global) initialisé avec succès.");
    display_global.displayWelcomeScreen(true); // Utiliser le mode simple
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) { vTaskDelay(pdMS_TO_TICKS(1000)); } else { delay(1000); } // Réduit, était 2000

    // Créer les caractères personnalisés sur l'écran global
    display_global.createCustomChars();
    
    // Initialiser le gestionnaire d'interface (LCD et boutons)
    if (!uiManager.begin()) { 
        LOG_ERROR("INIT", "Échec de l'initialisation de UIManager.");
    } else {
        LOG_INFO("INIT", "UIManager initialisé.");
    }
  } else {
    LOG_ERROR("INIT", "Échec d'initialisation de l'écran LCD (global) après timeout.");
    scanI2CBus(); // Appel du scanner I2C si l'écran LCD échoue
  }
  
  // Initialiser les servomoteurs
  servoInitAll();
  
  // Initialiser l'interface tableau de bord
  dashboardInit();
  
  // Initialiser l'autopilote (mais désactivé par défaut)
  autopilotInit();
}

/**
 * Configure et démarre les tâches FreeRTOS
 */
void setupTasks() {
    // Initialiser le gestionnaire de tâches avec les bons arguments
    taskManager.begin(&uiManager, &wifiManager);
    taskManager.startTasks();
  
    LOG_INFO("TASKS", "Tâches FreeRTOS démarrées");
}

/**
 * Callback appelé au démarrage d'une mise à jour OTA
 */
void onOTAStart() {
  LOG_INFO("OTA", "Mise à jour OTA démarrée");
  digitalWrite(LED_PIN, HIGH);  // LED allumée pendant la mise à jour
  if (display_global.isSuccessfullyInitialized()) {
    display_global.displayMessage("OTA", "Mise à jour démarrée");
  }
}

/**
 * Callback appelé régulièrement pendant la progression d'une mise à jour OTA
 * @param current Nombre d'octets actuellement transférés
 * @param final Taille totale du fichier à transférer
 */
void onOTAProgress(size_t current, size_t final) {
  // Mise à jour moins fréquente pour les performances
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    LOG_INFO("OTA", "Progression: %u / %u octets", current, final);
    
    // Afficher la progression sur l'écran LCD
    display_global.displayOTAProgress(current, final);
    
    // Faire clignoter la LED pour indiquer l'activité
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

/**
 * Callback appelé à la fin d'une mise à jour OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void onOTAEnd(bool success) {
    OTAManager::handleOTAEnd(success, LED_PIN, display_global);
}

/**
 * Vérifie l'état du système
 */
void checkSystemState() {
  // Exemple de vérification d'état système
  if (wifiConnected) {
    LOG_INFO("SYSTEM", "WiFi connecté, état système normal");
  } else {
    LOG_WARNING("SYSTEM", "WiFi non connecté, vérifier la configuration");
  }
}

/**
 * FONCTION PRINCIPALE D'INITIALISATION
 * Initialise uniquement le moniteur série et démarre FreeRTOS
 */
void setup() {
    // Initialisation du moniteur série
    Serial.begin(115200);
    delay(100); // Court délai pour que le moniteur série s'initialise
    
    // Initialisation des logs (premier composant à initialiser)
    logInit(LOG_LEVEL_INFO, 115200);
    LOG_INFO("INIT", "Démarrage Kite PiloteV3, version : %s", SYSTEM_VERSION);
    
    // Configuration des broches de diagnostic (LED uniquement)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED allumée pendant l'initialisation
    
    // Initialisation du bus I2C
    LOG_INFO("I2C", "Initialisation du bus I2C (SDA: %d, SCL: %d, Freq: 400kHz)", I2C_SDA, I2C_SCL);
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000); // 400kHz
    Wire.setTimeOut(250); // Définir un timeout de 250ms pour les opérations I2C
    LOG_INFO("I2C", "Timeout I2C configuré à 250ms.");
    
    // Création de la tâche principale d'initialisation (avec priorité élevée)
    LOG_INFO("INIT", "Démarrage de la tâche d'initialisation");
    xTaskCreate(
        initTask,           // Fonction de la tâche
        "InitTask",         // Nom de la tâche 
        8192,               // Taille de la pile (mots)
        NULL,               // Paramètres de la tâche
        3,                  // Priorité (plus élevée pour garantir son exécution rapide)
        NULL                // Handle de la tâche
    );
    
    // FreeRTOS prend maintenant en charge l'exécution
    LOG_INFO("INIT", "Configuration terminée, FreeRTOS démarré");
}

/**
 * Tâche d'initialisation principale
 * @param parameter Paramètre passé à la tâche (non utilisé ici)
 */
void initTask(void* parameter) {
    // Initialisation du système
    if (systemInit() != SYS_OK) {
        LOG_ERROR("SYS", "Échec de l'initialisation du système");
        vTaskDelete(NULL);
        return;
    }

    // Initialisation du matériel (y compris display_global)
    setupHardware();

    // Initialisation des managers qui dépendent de matériel initialisé
    // ou qui ont leur propre logique d'initialisation/tâches.
    
    // PotentiometerManager - initialisation unique
    LOG_INFO("INIT_TASK", "Initialisation PotentiometerManager...");
    potManager.begin();
    if (potManager.isInitialized()) {
        LOG_INFO("INIT_TASK", "PotManager OK. Dir: %d, Trim: %d, Len: %d", potManager.getDirection(), potManager.getTrim(), potManager.getLineLength());
    } else {
        LOG_ERROR("INIT_TASK", "Échec PotManager.");
    }

    // ButtonUIManager - dépend de display_global qui doit être initialisé par setupHardware
    LOG_INFO("INIT_TASK", "Initialisation ButtonUIManager...");
    if (buttonUI.begin()) { 
        LOG_INFO("INIT_TASK", "ButtonUIManager OK.");
    } else {
        LOG_ERROR("INIT_TASK", "Échec ButtonUIManager.");
    }

    // S'assurer que l'interface utilisateur (UIManager) est correctement initialisée
    if (!uiManager.isInitialized()) {
        LOG_WARNING("INIT_TASK", "UIManager non initialisé après setupHardware. Nouvelle tentative...");
    } else {
        LOG_INFO("INIT_TASK", "UIManager est initialisé.");
    }
    
    // Délai pour s'assurer que tout est correctement initialisé
    vTaskDelay(pdMS_TO_TICKS(100));

    // Démarrage des tâches FreeRTOS
    LOG_INFO("INIT_TASK", "Démarrage des tâches via TaskManager...");
    taskManager.begin(&uiManager, &wifiManager);
    taskManager.startTasks();

    LOG_INFO("INIT_TASK", "Système prêt.");

    // Supprimer la tâche une fois l'initialisation terminée
    vTaskDelete(NULL);
}

/**
 * BOUCLE PRINCIPALE
 * Gère les mises à jour OTA et la coordination des différentes fonctionnalités
 */
void loop() {
  // Gestion des tâches en arrière-plan
  feedWatchdogs();
  ElegantOTA.loop();
  delay(10);
}
