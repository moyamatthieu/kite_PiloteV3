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

// === INCLUSIONS DES BIBLIOTHÈQUES ===
#include <WiFi.h>                // Gestion de la connexion WiFi
#include <AsyncTCP.h>            // Gestion des communications TCP asynchrones
#include <ESPAsyncWebServer.h>   // Serveur web asynchrone
#include <ElegantOTA.h>          // Mise à jour OTA (Over The Air)
#include <ESP32Servo.h>          // Bibliothèque pour les servomoteurs
#include <FastAccelStepper.h>    // Bibliothèque pour le moteur pas à pas

// === INCLUSIONS MODULES CORE ===
#include "core/config.h"         // Configuration centralisée
#include "core/system.h"         // Système de base
#include "core/logging.h"        // Système de journalisation
#include "core/task_manager.h"   // Gestionnaire de tâches

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
#include "../../include/hardware/io/ui_manager.h"
#include "../../include/hardware/io/button_ui.h"
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

// === DÉFINITION DES VARIABLES GLOBALES ===

// Objets principaux
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
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
  
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
  } else {
    LOG_ERROR("INIT", "Échec d'initialisation de l'écran LCD");
  }
  
  // Initialiser les potentiomètres
  LOG_INFO("POT", "Initialisation des potentiomètres");
  potManager.begin();
  
  // Valider l'initialisation en lisant les valeurs initiales
  potManager.updatePotentiometers();
  LOG_INFO("POT", "Valeurs initiales - Dir: %d, Trim: %d, Longueur: %d", 
           potManager.getDirection(), potManager.getTrim(), potManager.getLineLength());
  
  // Initialiser les servomoteurs
  servoInitAll();
  
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
 * Fonction setup appelée une fois au démarrage du système
 */
void setup() {
  // Initialiser le système de journalisation
  #if defined(LOG_LEVEL)
    logInit((LogLevel)LOG_LEVEL, 115200);
  #else
    logInit(LOG_INFO, 115200);  // Par défaut si non défini
  #endif
  
  // Message de démarrage
  LOG_INFO("INIT", "Démarrage Kite PiloteV3 v%s", SYSTEM_VERSION);
  
  // Initialiser le système principal
  SystemErrorCode sysInitResult = systemInit();
  if (sysInitResult != SYS_OK) {
    LOG_ERROR("INIT", "Erreur d'initialisation système: %s", systemErrorToString(sysInitResult));
  }
  
  // Initialiser les composants matériels
  setupHardware();
  
  // Configurer la connexion WiFi
  setupWiFi();
  
  // Configurer le serveur web
  setupServer();
  
  // Configurer et démarrer les tâches FreeRTOS
  setupTasks();
  
  // Vérifier l'état du système
  if (isSystemHealthy()) {
    LOG_INFO("INIT", "Système prêt et en bon état");
  } else {
    LOG_WARNING("INIT", "Système prêt avec avertissements");
  }
  
  // Afficher l'écran principal
  display.updateMainDisplay();
}

/**
 * Boucle principale exécutée en continu
 */
void loop() {
  // Nourrir les watchdogs pour éviter un reset
  feedWatchdogs();
  
  // Gestion des mises à jour OTA
  ElegantOTA.loop();
  
  // Vérification périodique de l'état du système
  unsigned long currentTime = millis();
  if (currentTime - lastSystemCheck >= SYSTEM_CHECK_INTERVAL) {
    lastSystemCheck = currentTime;
    
    // Vérifier l'état du système
    bool healthy = isSystemHealthy();
    
    // Vérifier l'état du WiFi et tenter de reconnecter si nécessaire
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      LOG_WARNING("WIFI", "Connexion perdue, tentative de reconnexion");
      wifiConnected = false;
      WiFi.reconnect();
    } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
      LOG_INFO("WIFI", "Reconnecté au réseau %s", WIFI_SSID);
      wifiConnected = true;
    }
    
    // Journaliser l'utilisation de la mémoire
    logMemoryUsage("MAIN");
  }
  
  // La plupart des opérations sont gérées par les tâches FreeRTOS,
  // donc la boucle principale reste légère
  
  // Court délai pour éviter de monopoliser le CPU
  delay(10);
}
