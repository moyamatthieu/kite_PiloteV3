/*
  -----------------------
  Kite PiloteV3 - ESP32 avec ElegantOTA
  -----------------------
  
  Système de contrôle ESP32 avec mise à jour OTA et interface utilisateur OLED.
  Ce programme permet le contrôle et la surveillance d'équipements via une interface
  web accessible à distance et un écran OLED pour visualisation locale.
  
  Version: 1.0.3
  Date: 30 avril 2025
  Auteurs: Équipe Kite PiloteV3
*/

// === INCLUSIONS DES BIBLIOTHÈQUES ===
#include <WiFi.h>                // Gestion de la connexion WiFi
#include <AsyncTCP.h>            // Gestion des communications TCP asynchrones
#include <ESPAsyncWebServer.h>   // Serveur web asynchrone
#include <ElegantOTA.h>          // Mise à jour OTA (Over The Air)
#include <SPIFFS.h>              // Système de fichiers SPIFFS
#include "../include/display.h"  // Module de gestion de l'affichage
#include "../include/kite_webserver.h" // Module de gestion du serveur web

// === DÉFINITION DES CONSTANTES ===

// Configuration WiFi
const char* WIFI_SSID = "Wokwi-GUEST";      // SSID du réseau WiFi
const char* WIFI_PASSWORD = "";             // Mot de passe WiFi (vide pour réseau ouvert)
const uint16_t SERVER_PORT = 80;            // Port du serveur web

// Mode de fonctionnement du serveur web
const bool USE_SPIFFS_FILES = true;         // true: utilise les fichiers SPIFFS, false: génère le HTML en code

// Pins
const uint8_t LED_PIN = 2;                  // Pin pour la LED indicatrice
const uint8_t BUTTON_BLUE_PIN = 4;          // Pin pour le bouton bleu
const uint8_t BUTTON_GREEN_PIN = 5;         // Pin pour le bouton vert

// Timeouts et délais
const uint16_t WIFI_TIMEOUT_MS = 20000;     // Timeout pour la connexion WiFi (ms)

// === DÉFINITION DES VARIABLES GLOBALES ===

// Objets principaux
DisplayManager display;                       // Gestionnaire d'affichage OLED
AsyncWebServer server(SERVER_PORT);           // Serveur web asynchrone

// Variables d'état système
bool wifiConnected = false;                  // État de la connexion WiFi
uint8_t systemState = 0;                     // État général du système
unsigned long ota_progress_millis = 0;       // Timestamp pour la progression OTA

// Variables pour la gestion du temps (non-bloquante)
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

/**
 * Initialise la communication série avec un débit de 115200 bauds
 * Effectue plusieurs tests pour vérifier que la communication fonctionne
 */
void setupSerial() {
  Serial.begin(115200);
  delay(100);  // Court délai pour stabiliser la connexion
  
  // Tests de communication série
  Serial.println("\n\n=============================================");
  Serial.println("Démarrage du système Kite PiloteV3");
  Serial.println("Version: 1.0.3");
  Serial.println("=============================================");
  Serial.flush();
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
    
    // Afficher les informations de connexion sur l'écran OLED
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
    
    // Afficher la progression sur l'écran OLED
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
  return status;
}

/**
 * Fonction setup appelée une fois au démarrage du système
 * Initialise tous les composants dans l'ordre approprié
 */
void setup() {
  setupSerial();
  
  // Initialiser le module d'affichage
  display.setupI2C();
  if (display.initOLED()) {
    display.displayWelcomeScreen();
  }
  
  setupGPIO();
  setupWiFi();
  
  // Initialiser SPIFFS avant de configurer le serveur
  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur lors de l'initialisation de SPIFFS");
  }
  
  setupServer();
}

/**
 * Boucle principale exécutée en continu
 * Utilise des approches non-bloquantes pour gérer les différentes tâches
 */
void loop() {
  // Gestion des mises à jour OTA
  ElegantOTA.loop();
  
  // Vérification de l'état du système (toutes les 10s)
  checkSystemStatus();
  
  // Vérification de l'état de l'écran (toutes les 30s)
  display.checkDisplayStatus();
  
  // Mise à jour de l'affichage (toutes les 5s)
  if (wifiConnected) {
    display.updateDisplayRotation(WIFI_SSID, WiFi.localIP());
  }
  
  // Autres tâches non-bloquantes peuvent être ajoutées ici
  // ...
}
