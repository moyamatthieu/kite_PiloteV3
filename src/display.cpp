/*
  -----------------------
  Kite PiloteV3 - Module d'affichage (Implémentation)
  -----------------------
  
  Implémentation des fonctions du module de gestion de l'affichage OLED.
  
  Version: 1.0.0
  Date: 30 avril 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../include/display.h"

/**
 * Constructeur de la classe DisplayManager
 * Initialise les variables membres
 */
DisplayManager::DisplayManager() 
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    oledInitialized(false),
    lastDisplayUpdate(0),
    lastDisplayCheck(0),
    currentDisplayState(0) {
}

/**
 * Destructeur de la classe DisplayManager
 * Libère les ressources si nécessaire
 */
DisplayManager::~DisplayManager() {
  // Rien à libérer pour l'instant
}

/**
 * Initialise la communication I2C avec les pins configurés
 * Réduit la vitesse d'horloge pour améliorer la stabilité
 */
void DisplayManager::setupI2C() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_SPEED);
  delay(50);  // Court délai pour stabiliser le bus I2C
  Serial.println("Initialisation I2C (SDA:" + String(I2C_SDA_PIN) + ", SCL:" + String(I2C_SCL_PIN) + ")");
}

/**
 * Initialise l'écran OLED avec plusieurs tentatives en cas d'échec
 * @return true si l'initialisation a réussi, false sinon
 */
bool DisplayManager::initOLED() {
  Serial.println("Tentative d'initialisation de l'écran OLED (Adresse: 0x" + String(SCREEN_ADDRESS, HEX) + ")...");
  
  // Réinitialisation logicielle de l'écran avant initialisation
  Wire.beginTransmission(SCREEN_ADDRESS);
  Wire.endTransmission();
  delay(100);
  
  // Plusieurs tentatives d'initialisation
  for (int tentative = 1; tentative <= MAX_INIT_ATTEMPTS; tentative++) {
    Serial.printf("Essai %d/%d...\n", tentative, MAX_INIT_ATTEMPTS);
    
    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println("Écran OLED initialisé avec succès!");
      
      // Procédure de démarrage propre pour l'écran
      display.clearDisplay();
      display.display();
      delay(50);  // Attendre que l'écran se stabilise
      
      oledInitialized = true;
      return true;
    }
    
    Serial.println("Échec d'initialisation de l'écran SSD1306, nouvelle tentative...");
    delay(INIT_RETRY_DELAY);
    
    // Réinitialiser le bus I2C entre les tentatives
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    delay(50);
  }
  
  Serial.println("ERREUR CRITIQUE: Échec d'initialisation de l'écran SSD1306 après plusieurs tentatives");
  oledInitialized = false;
  return false;
}

/**
 * Affiche l'écran de bienvenue sur l'écran OLED
 */
void DisplayManager::displayWelcomeScreen() {
  if (!oledInitialized) {
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(TEXT_SIZE_TITLE);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Kite PiloteV3");
  display.setCursor(0, 16);
  display.println("Initialisation...");
  display.display();
  delay(1000);
}

/**
 * Affiche un message sur l'écran OLED et dans le moniteur série
 * @param title Titre du message à afficher
 * @param message Contenu du message
 * @param clear Si true, efface l'écran avant d'afficher
 */
void DisplayManager::displayMessage(const String& title, const String& message, bool clear) {
  if (!oledInitialized) {
    Serial.println(title + ": " + message);
    return;
  }
  
  if(clear) {
    display.clearDisplay();
  }
  
  // Paramètres d'affichage optimisés
  display.setTextSize(TEXT_SIZE_TITLE);
  display.setTextColor(SSD1306_WHITE);
  
  // Titre en haut
  display.setCursor(0, 0);
  display.println(title);
  
  // Message principal
  display.setCursor(0, 16);
  display.println(message);
  
  // Forcer l'affichage et s'assurer que tout est dessiné
  display.display();
  delay(10);  // Court délai pour stabiliser l'affichage
  
  // Afficher également sur le port série
  Serial.println(title + ": " + message);
}

/**
 * Met à jour l'affichage de l'écran OLED selon un cycle de rotation
 * @param ssid Nom du réseau WiFi
 * @param ip Adresse IP
 */
void DisplayManager::updateDisplayRotation(const String& ssid, const IPAddress& ip) {
  if (!oledInitialized || millis() - lastDisplayUpdate < DISPLAY_ROTATION_INTERVAL) {
    return;
  }
  
  lastDisplayUpdate = millis();
  
  // Rotation des informations affichées
  switch (currentDisplayState) {
    case DISPLAY_WIFI_INFO:
      // Affichage des informations WiFi
      displayMessage("État", "Système en marche", true);
      displayMessage("WiFi", "SSID: " + ssid, false);
      break;
      
    case DISPLAY_IP_ADDRESS:
      // Affichage de l'adresse IP
      displayMessage("Réseau", "Adresse IP:", true);
      displayMessage("", ip.toString(), false);
      break;
      
    case DISPLAY_OTA_INSTRUCTIONS:
      // Affichage des instructions OTA
      displayMessage("OTA Update", "Via navigateur:", true);
      displayMessage("", "http://" + ip.toString() + "/update", false);
      break;
      
    case DISPLAY_SYSTEM_STATS:
      // Affichage des stats système
      displayMessage("Système", "Temps:", true);
      displayMessage("", String(millis() / 1000) + " secondes", false);
      break;
  }
  
  // Passer à l'état suivant (rotation)
  currentDisplayState = (currentDisplayState + 1) % DISPLAY_STATE_COUNT;
}

/**
 * Affiche la progression d'une mise à jour OTA
 * @param current Nombre d'octets actuellement transférés
 * @param final Taille totale du fichier à transférer
 */
void DisplayManager::displayOTAProgress(size_t current, size_t final) {
  if (!oledInitialized) {
    return;
  }
  
  int percentage = (current * 100) / final;
  
  display.clearDisplay();
  display.setTextSize(TEXT_SIZE_TITLE);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Mise à jour OTA");
  display.setCursor(0, 16);
  display.printf("Progression: %d%%", percentage);
  
  // Dessiner une barre de progression
  display.drawRect(0, 30, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.fillRect(0, 30, (percentage * SCREEN_WIDTH) / 100, 10, SSD1306_WHITE);
  display.display();
}

/**
 * Affiche le résultat final d'une mise à jour OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void DisplayManager::displayOTAStatus(bool success) {
  if (!oledInitialized) {
    return;
  }
  
  if (success) {
    displayMessage("OTA", "Mise à jour terminée avec succès!");
  } else {
    displayMessage("OTA", "Erreur lors de la mise à jour!");
  }
}

/**
 * Vérifie périodiquement l'état de l'écran OLED et tente de le réinitialiser si nécessaire
 */
void DisplayManager::checkDisplayStatus() {
  static bool displayNeedsReset = false;
  
  // Vérification périodique de l'état de l'écran
  if (millis() - lastDisplayCheck > DISPLAY_CHECK_INTERVAL) {
    lastDisplayCheck = millis();
    
    // Test simple pour vérifier si l'écran fonctionne correctement
    if (oledInitialized) {
      Wire.beginTransmission(SCREEN_ADDRESS);
      byte error = Wire.endTransmission();
      
      if (error != 0) {
        Serial.println("Problème détecté avec l'écran OLED - tentative de réinitialisation");
        displayNeedsReset = true;
      }
    }
  }
  
  // Si l'écran a besoin d'être réinitialisé
  if (displayNeedsReset) {
    // Tentative de réinitialisation de l'écran
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    delay(50);
    
    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      display.clearDisplay();
      display.display();
      Serial.println("Écran OLED réinitialisé avec succès");
      displayNeedsReset = false;
      oledInitialized = true;
    } else {
      oledInitialized = false;
    }
  }
}

/**
 * Vérifie si l'écran OLED est initialisé
 * @return true si l'écran est initialisé, false sinon
 */
bool DisplayManager::isInitialized() const {
  return oledInitialized;
}