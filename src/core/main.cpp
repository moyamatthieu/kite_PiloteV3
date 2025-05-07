/*
  -----------------------
  Kite PiloteV3 - Template de Fichier
  -----------------------

  Objectif : Décrire les objectifs et les choix d'architecture pour ce fichier.
  
  Instructions :
  - Ajouter des commentaires pour expliquer les sections importantes.
  - Respecter les conventions de codage définies dans le projet.
  - Documenter les fonctions et les classes pour faciliter la maintenance.

  Date : 6 mai 2025
  Auteur : Équipe Kite PiloteV3
*/

/*
  -----------------------
  Kite PiloteV3 - Programme principal
  -----------------------
  
  Point d'entrée principal pour le système de contrôle de kite générateur d'électricité.
  Architecture modulaire optimisée pour ESP32.
  
  Version: 3.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

// === INCLUSIONS DES BIBLIOTHÈQUES EXTERNES ===
#include <WiFi.h>                // Gestion de la connexion WiFi
#include <AsyncTCP.h>            // Communication TCP asynchrone non bloquante
#include <ESPAsyncWebServer.h>   // Serveur HTTP asynchrone pour interface web
#include <ElegantOTA.h>          // Module de mise à jour OTA (Over The Air)
#include <ESP32Servo.h>          // Bibliothèque pour les servomoteurs
#include <FastAccelStepper.h>    // Bibliothèque pour le moteur pas à pas

// === INCLUSIONS DES MODULES CENTRAUX DU PROJET ===
#include "core/config.h"         // Constantes de configuration globale (pins, timings)
#include "core/system.h"         // Gestion de l'état système et watchdog
#include "core/logging.h"        // Fonctions et macros de journalisation
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
#include "communication/api.h"
#include "communication/wifi_manager.h"

// === INCLUSIONS MODULES CONTROL ===
#include "control/autopilot.h"

// === INCLUSIONS MODULES UI ===
#include "ui/dashboard.h"
#include "ui/presenter.h"
#include "ui/webserver.h"                 // Serveur web pour l'interface utilisateur

// === INCLUSIONS MODULES UTILS ===
#include "utils/data_storage.h"           // Gestion du stockage des données
#include "utils/diagnostics.h"            // Diagnostics système
#include "utils/terminal.h"               // Terminal distant

// === DÉCLARATION DES OBJETS GLOBAUX ===
DisplayManager display;                       // Gestionnaire d'affichage LCD
ButtonUIManager buttonUI(&display);           // Gestionnaire de l'interface utilisateur à boutons
PotentiometerManager potManager;              // Gestionnaire des potentiomètres
AsyncWebServer server(SERVER_PORT);           // Serveur web asynchrone
TaskManager taskManager;                      // Gestionnaire de tâches multiples
UIManager uiManager;                          // Gestionnaire de l'interface utilisateur
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

// Ajout pour affichage dynamique
static unsigned long lastDisplayUpdate = 0;
static int lastDir = 0, lastTrim = 0, lastLen = 0;
static bool lastWifi = false;
static unsigned long lastUptime = 0;

// === DÉCLARATION DES FONCTIONS ===

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
  if (!display.isInitialized()) {
    return;
  }
  
  LOG_INFO("WIFI", "Connexion au réseau %s...", WIFI_SSID);
  display.displayMessage("Système", "Connexion WiFi...");
  
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
    
    // Afficher les informations de connexion
    display.displayWiFiInfo(WIFI_SSID, WiFi.localIP());
    delay(1000); // Afficher pendant 1 seconde
  } else {
    LOG_ERROR("WIFI", "Échec de connexion au réseau %s", WIFI_SSID);
    display.displayMessage("Erreur", "Échec WiFi");
    delay(1000);
  }
}

/**
 * Configure et démarre le serveur web avec support OTA (optimisé)
 */
void setupServer() {
  if (!wifiConnected) {
    LOG_WARNING("SERVER", "WiFi non connecté, serveur web non démarré");
    return;
  }
  
  display.displayMessage("Système", "Démarrage serveur...");
  
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
  
  // Affichage final
  display.displayMessage("Système", "Prêt!");
  delay(1000);
  
  char otaUrl[40];
  snprintf(otaUrl, sizeof(otaUrl), "http://%d.%d.%d.%d/update", 
           WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  display.displayMessage("OTA", otaUrl);
  delay(1000);
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
  
  // Initialiser l'écran LCD
  display.setupI2C();
  if (display.initLCD()) {
    // Afficher "INIT" pendant 2 secondes au démarrage
    display.displayWelcomeScreen(true); // Utiliser le mode simple
    delay(2000); // Attendre 2 secondes
    
    // Créer les caractères personnalisés
    display.createCustomChars();
    // Initialiser le gestionnaire d'interface (LCD et boutons)
    uiManager.begin();
  } else {
    LOG_ERROR("INIT", "Échec d'initialisation de l'écran LCD");
  }
  
  // Initialiser les potentiomètres
  LOG_INFO("POT", "Initialisation des potentiomètres");
  potManager.begin();
  
  // Valider l'initialisation en lisant les valeurs initiales
  if (potManager.isInitialized()) {
    LOG_INFO("POT", "Valeurs initiales - Dir: %d, Trim: %d, Longueur: %d", 
             potManager.getDirection(), potManager.getTrim(), potManager.getLineLength());
  } else {
    LOG_ERROR("POT", "Échec d'initialisation des potentiomètres");
  }
  
  // Initialiser les servomoteurs
  servoInitAll();
  
  // Initialiser l'interface utilisateur à boutons
  buttonUI.begin();
  
  // Initialiser l'interface utilisateur à boutons
  buttonUI.begin();
  
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
  display.displayMessage("OTA", "Mise à jour démarrée");
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
    display.displayOTAProgress(current, final);
    
    // Faire clignoter la LED pour indiquer l'activité
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

/**
 * Callback appelé à la fin d'une mise à jour OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void onOTAEnd(bool success) {
  // LED éteinte après la mise à jour
  digitalWrite(LED_PIN, LOW);
  
  display.displayOTAStatus(success);
  
  if (success) {
    LOG_INFO("OTA", "Mise à jour terminée avec succès");
    
    // Double clignotement pour indiquer le succès
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(50);
      digitalWrite(LED_PIN, LOW);
      delay(50);
    }
    
    // Redémarrer après une courte pause
    systemRestart(3000);
  } else {
    LOG_ERROR("OTA", "Erreur durant la mise à jour OTA");
    
    // Clignotement rapide pour indiquer l'erreur
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(25);
      digitalWrite(LED_PIN, LOW);
      delay(25);
    }
  }
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
 * Initialise le système et démarre les tâches principales
 */
void setup() {
  // Initialisation du port série et du système de logs
  #if defined(LOG_LEVEL)
    logInit((LogLevel)LOG_LEVEL, 115200);
  #else
    logInit(LOG_INFO, 115200);  // Niveau INFO par défaut si non spécifié
  #endif

  LOG_INFO("INIT", "Démarrage Kite PiloteV3, version : %s", SYSTEM_VERSION);

  // Initialisation du système et du watchdog
  SystemErrorCode result = systemInit();
  if (result != SYS_OK) {
    LOG_ERROR("SYS", "Échec init système : %s", systemErrorToString(result));
  }

  // Configuration du matériel : GPIO, écran, capteurs, actionneurs
  setupHardware();

  // Connexion au réseau WiFi configuré dans config.h
  setupWiFi();

  // Démarrage du serveur web et configuration OTA
  setupServer();

  // Lancement des tâches FreeRTOS (UI, capteurs, contrôle, etc.)
  setupTasks();

  // Vérification de l'état global après initialisation
  if (isSystemHealthy()) {
    LOG_INFO("INIT", "Système prêt et en bon état");
  } else {
    LOG_WARNING("INIT", "Système prêt avec avertissements");
  }

  // Affichage initial sur l'écran LCD : statut système et réseau
  display.updateMainDisplay();
}

/**
 * BOUCLE PRINCIPALE
 * Gère les mises à jour OTA et la coordination des différentes fonctionnalités
 */
void loop() {
  // Nourrit les watchdogs pour éviter les resets intempestifs
  feedWatchdogs();

  // Exécution de la boucle de mise à jour OTA (vérifications et upload)
  ElegantOTA.loop();

  // Pause légère pour céder le cœur CPU (non bloquant)
  delay(10);
}
