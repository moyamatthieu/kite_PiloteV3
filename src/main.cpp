/*
  -----------------------
  Kite PiloteV3 - ESP32 avec ElegantOTA et écran tactile
  -----------------------
  
  Système de contrôle ESP32 avec mise à jour OTA et interface utilisateur tactile TFT.
  Ce programme permet le contrôle et la surveillance d'équipements via une interface
  web accessible à distance et un écran TFT avec tactile pour visualisation et contrôle local.
  
  Version: 2.0.0
  Date: 30 avril 2025
  Auteurs: Équipe Kite PiloteV3
*/

// === INCLUSIONS DES BIBLIOTHÈQUES ===
#include <WiFi.h>                // Gestion de la connexion WiFi
#include <AsyncTCP.h>            // Gestion des communications TCP asynchrones
#include <ESPAsyncWebServer.h>   // Serveur web asynchrone
#include <ElegantOTA.h>          // Mise à jour OTA (Over The Air)
#include <SPIFFS.h>              // Système de fichiers SPIFFS
#include <Wire.h>                // Communication I2C pour l'écran tactile
#include "../include/config.h"   // Fichier de configuration centralisé
#include "../include/display.h"  // Module de gestion de l'affichage
#include "../include/kite_webserver.h" // Module de gestion du serveur web
#include "../include/touch_ui.h" // Module de gestion de l'interface tactile
#include "../include/task_manager.h" // Module de gestion des tâches multiples

// === DÉFINITION DES VARIABLES GLOBALES ===

// Objets principaux
DisplayManager display;                       // Gestionnaire d'affichage TFT
TouchUIManager touchUI(&display);             // Gestionnaire d'interface tactile
AsyncWebServer server(SERVER_PORT);           // Serveur web asynchrone
TaskManager taskManager;                      // Gestionnaire de tâches multiples

// Variables d'état système
bool wifiConnected = false;                  // État de la connexion WiFi
uint8_t systemState = 0;                     // État général du système
unsigned long ota_progress_millis = 0;       // Timestamp pour la progression OTA
unsigned long lastSystemCheck = 0;           // Dernière vérification système

// === DÉCLARATION DES FONCTIONS ===

// Fonctions d'initialisation
void setupSerial();
void setupWiFi();
void setupServer();
void setupGPIO();

// Fonctions OTA
void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

// Fonctions système
void checkSystemStatus();
String getSystemStatusString();

// === IMPLÉMENTATION DES FONCTIONS ===

#include "../include/logging.h"  // Système de journalisation avancé

// Variables pour contrôler le système
bool systemInitialized = false;
unsigned long lastMemoryReport = 0;

/**
 * Initialise la communication série avec le système de journalisation avancé
 */
void setupSerial() {
  // Initialiser avec niveau LOG_INFO (défini dans platformio.ini via LOG_LEVEL)
  #if defined(LOG_LEVEL)
    logInit((LogLevel)LOG_LEVEL);
  #else
    logInit(LOG_INFO);  // Par défaut si non défini
  #endif
  
  // Message de démarrage
  LOG_INFO("SYSTEM", "=============================================");
  LOG_INFO("SYSTEM", "Démarrage du système Kite PiloteV3");
  LOG_INFO("SYSTEM", "Version: %s", SYSTEM_VERSION);
  LOG_INFO("SYSTEM", "Build: %s %s", SYSTEM_BUILD_DATE, SYSTEM_BUILD_TIME);
  LOG_INFO("SYSTEM", "=============================================");
}

/**
 * Initialise et connecte le WiFi avec timeout
 */
void setupWiFi() {
  if (!display.isInitialized()) {
    return;  // Si l'écran n'est pas initialisé, on ne peut pas afficher les messages
  }
  
  Serial.println("Tentative de connexion WiFi...");
  display.displayMessage("Système", "Connexion WiFi...");
  
  // Configuration du mode WiFi en mode station
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Attente de la connexion WiFi avec timeout
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  // Vérifier si la connexion a réussi
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  
  if (wifiConnected) {
    Serial.print("Connecté à ");
    Serial.println(WIFI_SSID);
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
    
    // Afficher les informations de connexion sur l'écran TFT
    display.displayMessage("WiFi", "Connecté à " + String(WIFI_SSID), true);
    display.displayMessage("IP", WiFi.localIP().toString(), false);
    delay(2000);
  } else {
    Serial.println("Échec de connexion au WiFi");
    display.displayMessage("Erreur", "Échec WiFi", true);
  }
}

/**
 * Configure et démarre le serveur web avec support OTA
 */
void setupServer() {
  if (!wifiConnected) {
    Serial.println("WiFi non connecté, serveur web non démarré");
    return;
  }
  
  display.displayMessage("Système", "Démarrage serveur...");
  
  // Définition du mode de fonctionnement du serveur web
  setWebServerMode(USE_SPIFFS_FILES);
  
  // Configuration du serveur web asynchrone selon le mode choisi
  setupServerRoutes(&server);
  
  // Démarrer ElegantOTA et configurer les callbacks
  ElegantOTA.begin(&server);
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  
  // Démarrer le serveur web asynchrone
  server.begin();
  Serial.println("Serveur HTTP démarré sur le port " + String(SERVER_PORT));
  
  // Message indiquant le mode de fonctionnement du serveur web
  String modeMsg = USE_SPIFFS_FILES ? "Mode fichiers SPIFFS" : "Mode génération HTML";
  Serial.println(modeMsg);
  
  // Affichage final avec l'adresse pour accéder à l'interface OTA
  display.displayMessage("Système", "Prêt!", true);
  display.displayMessage("OTA", "http://" + WiFi.localIP().toString() + "/update", false);
}

/**
 * Configure les GPIOs (LED et boutons)
 */
void setupGPIO() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
  
  // LED clignote une fois pour indiquer l'initialisation réussie
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("GPIOs configurés");
}

/**
 * Callback appelé au démarrage d'une mise à jour OTA
 */
void onOTAStart() {
  Serial.println("OTA update started!");
  digitalWrite(LED_PIN, HIGH);  // LED allumée pendant la mise à jour
  display.displayMessage("OTA", "Mise à jour démarrée");
}

/**
 * Callback appelé régulièrement pendant la progression d'une mise à jour OTA
 * @param current Nombre d'octets actuellement transférés
 * @param final Taille totale du fichier à transférer
 */
void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    
    // Afficher la progression sur l'écran TFT
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
    Serial.println("OTA update finished successfully!");
    
    // Double clignotement pour indiquer le succès
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  } else {
    Serial.println("There was an error during OTA update!");
    
    // Clignotement rapide pour indiquer l'erreur
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(50);
      digitalWrite(LED_PIN, LOW);
      delay(50);
    }
  }
}

/**
 * Vérifie l'état général du système (WiFi, etc.)
 */
void checkSystemStatus() {
  if (millis() - lastSystemCheck > 10000) {  // Vérification toutes les 10 secondes
    lastSystemCheck = millis();
    
    // Vérifier l'état WiFi
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    if (!wifiConnected) {
      Serial.println("Connexion WiFi perdue - tentative de reconnexion...");
      WiFi.reconnect();
    }
    
    // Autres vérifications système peuvent être ajoutées ici
  }
}

/**
 * Retourne une chaîne décrivant l'état actuel du système
 */
String getSystemStatusString() {
  String status = "OK";
  if (!wifiConnected) status = "WiFi déconnecté";
  if (!display.isInitialized()) status += ", Écran inactif";
  if (!display.isTouchInitialized()) status += ", Tactile inactif";
  return status;
}

// Callbacks pour les interactions tactiles
// Fonction sécurisée pour afficher un message et revenir à l'écran précédent
void showMessageAndReturnSafely(DisplayManager& display, TouchUIManager& touchUI, const char* title, const char* message, uint8_t returnScreen) {
  static bool messagePending = false;
  
  // Protection contre les appels multiples ou récursifs
  if (messagePending || !display.isInitialized()) {
    return;
  }
  
  messagePending = true;
  
  // Utiliser le gestionnaire de tâches pour envoyer un message d'affichage (évite les allocations)
  TaskManager::sendMessage(MSG_DISPLAY_UPDATE, 0, message);
  
  // Afficher le message sur l'écran
  display.displayMessage(title, message);
  
  // Programmez un changement d'écran sans utiliser delay()
  // Nous le ferons à la prochaine invocation
  static uint8_t targetScreen = 0;
  static unsigned long messageTimeout = 0;
  targetScreen = returnScreen;
  messageTimeout = millis() + 1500; // 1.5 secondes au lieu de 2 pour l'affichage
  
  // Vérifier dans la boucle principale si le délai est écoulé
  if (millis() > messageTimeout) {
    touchUI.showScreen(targetScreen);
    messagePending = false;
  }
}

void onMainScreenButton(uint8_t buttonId) {
  Serial.printf("Bouton principal pressé: %d\n", buttonId);
  
  // Protection contre les valeurs hors limites
  if (buttonId > 2) {
    return;
  }
  
  switch (buttonId) {
    case 0: // Tableau de bord
      touchUI.showScreen(2); // Pas de délai, changement direct
      break;
    case 1: // Paramètres
      touchUI.showScreen(1); // Pas de délai, changement direct
      break;
    case 2: // Informations
      // Afficher des informations système avec un message simple sans string
      if (display.isInitialized()) {
        static const char* infoMessage = "Système actif\nVersion: 2.0.0";
        display.displayMessage("Informations", infoMessage);
        // On pourrait utiliser showMessageAndReturnSafely ici si on veut revenir à l'écran principal
      }
      break;
  }
}

void onSettingsScreenButton(uint8_t buttonId) {
  Serial.printf("Bouton paramètres pressé: %d\n", buttonId);
  
  // Protection contre les valeurs hors limites
  if (buttonId > 4) {
    return;
  }
  
  switch (buttonId) {
    case 0: // WiFi
      // Afficher des informations WiFi de façon sécurisée (sans concaténation)
      if (display.isInitialized()) {
        static char wifiMessage[50]; // Buffer statique
        snprintf(wifiMessage, sizeof(wifiMessage), "SSID: %s", WIFI_SSID);
        display.displayMessage("WiFi", wifiMessage);
      }
      break;
    case 4: // Retour
      touchUI.showScreen(0); // Retour immédiat sans délai
      break;
  }
}

void onDashboardScreenButton(uint8_t buttonId) {
  Serial.printf("Bouton tableau de bord pressé: %d\n", buttonId);
  
  // Protection contre les valeurs hors limites
  if (buttonId > 4) {
    return;
  }
  
  switch (buttonId) {
    case 0: // Graphiques
      // Afficher un message simple
      display.displayMessage("Graphiques", "Fonctionnalité à venir");
      break;
    case 4: // Retour
      touchUI.showScreen(0); // Retour immédiat sans délai
      break;
  }
}

/**
 * Fonction setup appelée une fois au démarrage du système
 * Initialise tous les composants dans l'ordre approprié
 */
void setup() {
  setupSerial();
  
  // Initialiser le module d'affichage - écran TFT
  display.setupSPI();
  if (display.initTFT()) {
    display.displayWelcomeScreen();
  }
  
  // Initialiser l'écran tactile capacitif via I2C
  display.setupI2C();
  if (display.initTouch()) {
    Serial.println("Écran tactile capacitif FT6206 initialisé avec succès!");
  } else {
    Serial.println("Échec d'initialisation de l'écran tactile capacitif FT6206");
  }
  
  setupGPIO();
  setupWiFi();
  
  // Comme nous utilisons le mode de génération HTML par défaut,
  // nous n'initialisons pas SPIFFS pour éviter des erreurs inutiles
  Serial.println("Mode génération HTML activé, SPIFFS non utilisé");
  
  setupServer();
  
  // Initialiser l'interface tactile
  touchUI.begin();
  touchUI.calibrateTouch(); // Pour l'écran FT6206, cette fonction ne fait rien mais affiche un message
  
  // Configurer les callbacks pour l'interface tactile
  // Les dimensions de l'écran sont SCREEN_WIDTH=240, SCREEN_HEIGHT=320
  // Layout optimisé pour utiliser l'espace disponible
  
  // Écran principal - boutons centrés avec espacement uniforme
  uint8_t mainScreenId = touchUI.createScreen("Menu Principal", onMainScreenButton);
  touchUI.addButton(mainScreenId, 20, 60, 200, 50, "Tableau de bord", COLOR_BLUE);
  touchUI.addButton(mainScreenId, 20, 130, 200, 50, "Paramètres", COLOR_GREEN);
  touchUI.addButton(mainScreenId, 20, 200, 200, 50, "Informations", COLOR_MAGENTA);
  
  // Écran paramètres - grille 2x2 de boutons avec bouton retour en bas
  uint8_t settingsScreenId = touchUI.createScreen("Paramètres", onSettingsScreenButton);
  touchUI.addButton(settingsScreenId, 15, 60, 100, 50, "WiFi", COLOR_BLUE);
  touchUI.addButton(settingsScreenId, 125, 60, 100, 50, "Affichage", COLOR_GREEN);
  touchUI.addButton(settingsScreenId, 15, 130, 100, 50, "Système", COLOR_RED);
  touchUI.addButton(settingsScreenId, 125, 130, 100, 50, "OTA", COLOR_MAGENTA);
  touchUI.addButton(settingsScreenId, 70, 210, 100, 50, "Retour", COLOR_ORANGE);
  
  // Écran tableau de bord - grille 2x2 de boutons avec bouton retour en bas
  uint8_t dashboardScreenId = touchUI.createScreen("Tableau de bord", onDashboardScreenButton);
  touchUI.addButton(dashboardScreenId, 15, 60, 100, 50, "Graphiques", COLOR_BLUE);
  touchUI.addButton(dashboardScreenId, 125, 60, 100, 50, "Données", COLOR_GREEN);
  touchUI.addButton(dashboardScreenId, 15, 130, 100, 50, "Alarmes", COLOR_RED);
  touchUI.addButton(dashboardScreenId, 125, 130, 100, 50, "Log", COLOR_MAGENTA);
  touchUI.addButton(dashboardScreenId, 70, 210, 100, 50, "Retour", COLOR_ORANGE);
  
  // Afficher l'écran principal
  touchUI.showScreen(mainScreenId);
  
  // Initialiser et démarrer le gestionnaire de tâches multiples
  taskManager.begin(&display, &touchUI, &server);
  taskManager.startTasks();
  
  Serial.println("Système initialisé et prêt!");
}

/**
 * Boucle principale exécutée en continu
 * Dans cette nouvelle version, la boucle principale fait très peu de choses 
 * car les tâches sont gérées par FreeRTOS dans des threads séparés
 */
void loop() {
  // La majorité des tâches est maintenant gérée par le TaskManager
  // Cette boucle reste légère pour assurer une meilleure réactivité
  
  // Gestion des mises à jour OTA (toujours dans la boucle principale)
  ElegantOTA.loop();
  
  // On peut ajouter ici des tâches qui doivent absolument rester dans la boucle principale
  // Mais l'essentiel est géré dans des tâches séparées via FreeRTOS
  
  // Court délai pour éviter de saturer le CPU
  delay(10);
}
