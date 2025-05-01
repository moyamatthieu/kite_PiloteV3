/*
  -----------------------
  Kite PiloteV3 - Module d'affichage (Implémentation)
  -----------------------
  
  Implémentation des fonctions du module de gestion de l'affichage ILI9341.
  
  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../include/display.h"

/**
 * Constructeur de la classe DisplayManager
 * Initialise les variables membres
 */
DisplayManager::DisplayManager() 
  : tft(TFT_CS, TFT_DC, TFT_RST),
    tftInitialized(false),
    touchInitialized(false),
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
 * Initialise la communication SPI pour l'écran TFT
 */
void DisplayManager::setupSPI() {
  SPI.begin(TFT_CLK, TFT_MISO, TFT_MOSI);
  delay(50);  // Court délai pour stabiliser le bus SPI
  Serial.println("Initialisation SPI pour l'écran TFT");
}

/**
 * Initialise la communication I2C pour l'écran tactile capacitif
 */
void DisplayManager::setupI2C() {
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50);  // Court délai pour stabiliser le bus I2C
  Serial.println("Initialisation I2C pour l'écran tactile capacitif");
}

/**
 * Initialise l'écran TFT avec plusieurs tentatives en cas d'échec
 * @return true si l'initialisation a réussi, false sinon
 */
bool DisplayManager::initTFT() {
  Serial.println("Tentative d'initialisation de l'écran ILI9341...");
  
  // Plusieurs tentatives d'initialisation
  for (int tentative = 1; tentative <= MAX_INIT_ATTEMPTS; tentative++) {
    Serial.printf("Essai %d/%d...\n", tentative, MAX_INIT_ATTEMPTS);
    
    tft.begin();
    
    // Test simple pour vérifier si l'écran fonctionne correctement
    tft.fillScreen(COLOR_BLACK);
    tft.setRotation(0); // Portrait normal (pour inverser haut et bas par rapport à rotation 2)
    
    // Si nous arrivons ici sans plantage, l'initialisation a réussi
    Serial.println("Écran ILI9341 initialisé avec succès!");
    tftInitialized = true;
    return true;
    
    Serial.println("Échec d'initialisation de l'écran ILI9341, nouvelle tentative...");
    delay(INIT_RETRY_DELAY);
  }
  
  Serial.println("ERREUR CRITIQUE: Échec d'initialisation de l'écran ILI9341 après plusieurs tentatives");
  tftInitialized = false;
  return false;
}

/**
 * Initialise l'écran tactile capacitif
 * @return true si l'initialisation a réussi, false sinon
 */
bool DisplayManager::initTouch() {
  Serial.println("Tentative d'initialisation de l'écran tactile FT6206...");
  
  // Tentative d'initialisation
  if (ctp.begin(40)) { // Le paramètre 40 est la sensibilité du toucher
    Serial.println("Écran tactile FT6206 initialisé avec succès!");
    touchInitialized = true;
    return true;
  }
  
  Serial.println("ERREUR: Échec d'initialisation de l'écran tactile FT6206");
  touchInitialized = false;
  return false;
}

/**
 * Affiche l'écran de bienvenue sur l'écran TFT
 */
void DisplayManager::displayWelcomeScreen() {
  if (!tftInitialized) {
    return;
  }
  
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE);
  
  // Titre principal
  tft.setTextSize(TEXT_SIZE_TITLE);
  tft.setCursor(20, 40);
  tft.println("Kite PiloteV3");
  
  // Sous-titre
  tft.setTextSize(TEXT_SIZE_NORMAL);
  tft.setCursor(60, 100);
  tft.println("Initialisation...");
  
  // Logo ou graphique simple
  tft.fillRoundRect(120, 150, 80, 40, 8, COLOR_BLUE);
  tft.drawRoundRect(120, 150, 80, 40, 8, COLOR_WHITE);
  
  delay(1000);
}

/**
 * Affiche un message sur l'écran TFT et dans le moniteur série
 * @param title Titre du message à afficher
 * @param message Contenu du message
 * @param clear Si true, efface l'écran avant d'afficher
 */
void DisplayManager::displayMessage(const String& title, const String& message, bool clear) {
  if (!tftInitialized) {
    Serial.println(title + ": " + message);
    return;
  }
  
  if (clear) {
    tft.fillScreen(COLOR_BLACK);
  }
  
  // Afficher le titre avec un fond coloré
  tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(TEXT_SIZE_TITLE);
  
  // Centrer le titre
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((SCREEN_WIDTH - w) / 2, 10);
  tft.println(title);
  
  // Afficher le message avec gestion multiligne
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(TEXT_SIZE_NORMAL);
  int y = 60; // Position Y pour le message
  int lineHeight = 20; // Hauteur d'une ligne
  
  // Diviser le message en lignes si nécessaire
  int lastSpaceIndex = -1;
  String remainingMessage = message;
  
  while (remainingMessage.length() > 0) {
    String line = "";
    int maxLineLength = (SCREEN_WIDTH - 20) / (6 * TEXT_SIZE_NORMAL); // Calculer la longueur max de la ligne
    
    for (int i = 0; i < remainingMessage.length() && i < maxLineLength; i++) {
      if (remainingMessage[i] == ' ') {
        lastSpaceIndex = i;
      }
      line += remainingMessage[i];
      
      if (i == maxLineLength - 1 && lastSpaceIndex != -1) {
        line = remainingMessage.substring(0, lastSpaceIndex);
        remainingMessage = remainingMessage.substring(lastSpaceIndex + 1);
        break;
      }
    }
    
    if (line.length() == remainingMessage.length()) {
      remainingMessage = "";
    } else {
      if (lastSpaceIndex == -1) {
        remainingMessage = remainingMessage.substring(line.length());
      }
    }
    
    tft.setCursor(10, y);
    tft.println(line);
    y += lineHeight;
    lastSpaceIndex = -1;
    
    // Vérifier si on dépasse la hauteur de l'écran
    if (y > SCREEN_HEIGHT - lineHeight) {
      break;
    }
  }
  
  Serial.println(title + ": " + message);
}

/**
 * Met à jour l'affichage de l'écran TFT selon un cycle de rotation
 * @param ssid Nom du réseau WiFi
 * @param ip Adresse IP
 */
void DisplayManager::updateDisplayRotation(const String& ssid, const IPAddress& ip) {
  if (!tftInitialized || millis() - lastDisplayUpdate < DISPLAY_ROTATION_INTERVAL) {
    return;
  }
  
  lastDisplayUpdate = millis();
  
  // Rotation des informations affichées
  switch (currentDisplayState) {
    case DISPLAY_WIFI_INFO:
      // Affichage des informations WiFi
      tft.fillScreen(COLOR_BLACK);
      tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_BLUE);
      tft.setTextColor(COLOR_WHITE);
      tft.setTextSize(TEXT_SIZE_TITLE);
      tft.setCursor(20, 10);
      tft.println("État Système");
      
      tft.setTextSize(TEXT_SIZE_NORMAL);
      tft.setCursor(10, 60);
      tft.println("Système en marche");
      tft.setCursor(10, 90);
      tft.println("WiFi: " + ssid);
      
      // Indicateur visuel
      tft.fillCircle(280, 200, 20, COLOR_GREEN);
      break;
      
    case DISPLAY_IP_ADDRESS:
      // Affichage de l'adresse IP
      tft.fillScreen(COLOR_BLACK);
      tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_BLUE);
      tft.setTextColor(COLOR_WHITE);
      tft.setTextSize(TEXT_SIZE_TITLE);
      tft.setCursor(20, 10);
      tft.println("Réseau");
      
      tft.setTextSize(TEXT_SIZE_NORMAL);
      tft.setCursor(10, 60);
      tft.println("Adresse IP:");
      tft.setCursor(10, 90);
      tft.println(ip.toString());
      
      // Cadre décoratif autour de l'IP
      tft.drawRoundRect(5, 80, 310, 40, 5, COLOR_CYAN);
      break;
      
    case DISPLAY_OTA_INSTRUCTIONS:
      // Affichage des instructions OTA
      tft.fillScreen(COLOR_BLACK);
      tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_MAGENTA);
      tft.setTextColor(COLOR_WHITE);
      tft.setTextSize(TEXT_SIZE_TITLE);
      tft.setCursor(20, 10);
      tft.println("OTA Update");
      
      tft.setTextSize(TEXT_SIZE_NORMAL);
      tft.setCursor(10, 60);
      tft.println("Via navigateur:");
      tft.setCursor(10, 90);
      tft.println("http://" + ip.toString() + "/update");
      
      // Icône de mise à jour
      tft.fillTriangle(280, 180, 260, 220, 300, 220, COLOR_YELLOW);
      break;
      
    case DISPLAY_SYSTEM_STATS:
      // Affichage des stats système
      tft.fillScreen(COLOR_BLACK);
      tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_GREEN);
      tft.setTextColor(COLOR_BLACK);
      tft.setTextSize(TEXT_SIZE_TITLE);
      tft.setCursor(20, 10);
      tft.println("Système");
      
      tft.setTextColor(COLOR_WHITE);
      tft.setTextSize(TEXT_SIZE_NORMAL);
      tft.setCursor(10, 60);
      tft.println("Temps de fonctionnement:");
      tft.setCursor(10, 90);
      tft.println(String(millis() / 1000) + " secondes");
      
      // Barre de progression
      int barWidth = map(millis() % 60000, 0, 60000, 0, 300);
      tft.drawRect(10, 130, 300, 20, COLOR_WHITE);
      tft.fillRect(10, 130, barWidth, 20, COLOR_ORANGE);
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
  if (!tftInitialized) {
    return;
  }
  
  int percentage = (current * 100) / final;
  
  tft.fillScreen(COLOR_BLACK);
  
  // En-tête avec fond coloré
  tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_BLUE);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(TEXT_SIZE_TITLE);
  tft.setCursor(20, 10);
  tft.println("Mise à jour OTA");
  
  // Affichage du pourcentage
  tft.setTextSize(TEXT_SIZE_NORMAL);
  tft.setCursor(20, 60);
  tft.printf("Progression: %d%%", percentage);
  
  // Dessiner une barre de progression
  int barHeight = 30;
  int barY = 100;
  tft.drawRect(10, barY, SCREEN_WIDTH - 20, barHeight, COLOR_WHITE);
  tft.fillRect(10, barY, ((SCREEN_WIDTH - 20) * percentage) / 100, barHeight, COLOR_GREEN);
  
  // Afficher des détails supplémentaires
  tft.setCursor(20, 150);
  tft.printf("%u / %u octets", current, final);
  
  // Animation simple pour montrer l'activité
  static int animPos = 0;
  tft.fillCircle(20 + (animPos % 280), 200, 10, COLOR_YELLOW);
  animPos += 10;
}

/**
 * Affiche le résultat final d'une mise à jour OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void DisplayManager::displayOTAStatus(bool success) {
  if (!tftInitialized) {
    return;
  }
  
  tft.fillScreen(COLOR_BLACK);
  
  if (success) {
    // Fond vert pour succès
    tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_GREEN);
    tft.setTextColor(COLOR_BLACK);
    tft.setTextSize(TEXT_SIZE_TITLE);
    tft.setCursor(20, 10);
    tft.println("OTA Réussi");
    
    tft.setTextColor(COLOR_WHITE);
    tft.setTextSize(TEXT_SIZE_NORMAL);
    tft.setCursor(20, 60);
    tft.println("Mise à jour terminée");
    tft.setCursor(20, 90);
    tft.println("avec succès!");
    
    // Symbole de succès (coche)
    tft.fillCircle(160, 160, 40, COLOR_GREEN);
    tft.fillTriangle(130, 160, 150, 180, 190, 130, COLOR_WHITE);
  } else {
    // Fond rouge pour erreur
    tft.fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_RED);
    tft.setTextColor(COLOR_WHITE);
    tft.setTextSize(TEXT_SIZE_TITLE);
    tft.setCursor(20, 10);
    tft.println("OTA Échec");
    
    tft.setTextSize(TEXT_SIZE_NORMAL);
    tft.setCursor(20, 60);
    tft.println("Erreur lors de la");
    tft.setCursor(20, 90);
    tft.println("mise à jour!");
    
    // Symbole d'erreur (X)
    tft.fillCircle(160, 160, 40, COLOR_RED);
    tft.fillRect(140, 130, 40, 10, COLOR_WHITE);
    tft.fillRect(140, 180, 40, 10, COLOR_WHITE);
    tft.fillRect(130, 140, 10, 40, COLOR_WHITE);
    tft.fillRect(180, 140, 10, 40, COLOR_WHITE);
  }
}

/**
 * Dessine un bouton avec texte centré
 * @param x Position X du bouton
 * @param y Position Y du bouton
 * @param w Largeur du bouton
 * @param h Hauteur du bouton
 * @param label Texte du bouton
 * @param color Couleur du bouton
 */
void DisplayManager::drawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const String& label, uint16_t color) {
  // Dessiner le bouton
  tft.fillRoundRect(x, y, w, h, 8, color);
  tft.drawRoundRect(x, y, w, h, 8, COLOR_WHITE);
  
  // Centrer le texte
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(1);
  
  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
  
  tft.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
  tft.print(label);
}

/**
 * Vérifie périodiquement l'état de l'écran TFT et tente de le réinitialiser si nécessaire
 */
void DisplayManager::checkDisplayStatus() {
  static bool displayNeedsReset = false;
  
  // Vérification périodique de l'état de l'écran
  if (millis() - lastDisplayCheck > DISPLAY_CHECK_INTERVAL) {
    lastDisplayCheck = millis();
    
    // Pour l'écran TFT, nous n'avons pas de méthode simple pour vérifier l'état
    // Nous pouvons essayer de dessiner quelque chose et voir si cela fonctionne
    if (tftInitialized && displayNeedsReset) {
      Serial.println("Tentative de réinitialisation de l'écran TFT");
      tft.begin();
      tft.fillScreen(COLOR_BLACK);
      tft.setRotation(0);
      displayNeedsReset = false;
    }
  }
}

/**
 * Vérifie si l'écran TFT est initialisé
 * @return true si l'écran est initialisé, false sinon
 */
bool DisplayManager::isInitialized() const {
  return tftInitialized;
}

/**
 * Vérifie si l'écran tactile est initialisé
 * @return true si l'écran tactile est initialisé, false sinon
 */
bool DisplayManager::isTouchInitialized() const {
  return touchInitialized;
}

/**
 * Vérifie si l'écran est touché
 * @return true si l'écran est touché, false sinon
 */
bool DisplayManager::touched() {
  if (!touchInitialized) {
    return false;
  }
  
  return ctp.touched();
}

/**
 * Récupère les coordonnées du toucher sur l'écran tactile
 * @return Un point TS_Point contenant les coordonnées du toucher
 */
TS_Point DisplayManager::getTouch() {
  TS_Point p;
  
  if (!touchInitialized) {
    // Retourner un point par défaut si l'écran tactile n'est pas initialisé
    return p;
  }
  
  p = ctp.getPoint();
  
  // Adapter les coordonnées à l'orientation de l'écran
  // Pour le FT6206, aucun mapping n'est nécessaire car il est déjà
  // calibré pour correspondre à l'écran en mode portrait
  // On conserve ces lignes pour compatibilité future
  // p.x = map(p.x, 0, 240, 0, 240); 
  // p.y = map(p.y, 0, 320, 0, 320);
  
  return p;
}
