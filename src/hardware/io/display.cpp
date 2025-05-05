/*
  -----------------------
  Kite PiloteV3 - Module d'affichage (Implémentation)
  -----------------------
  
  Implémentation des fonctions du module de gestion de l'affichage LCD 2004.
  
  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../../hardware/io/display.h"
#include "../include/logging.h"
#include <WiFi.h>

// Caractères personnalisés pour les barres de progression et indicateurs
const uint8_t charDirection[] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

const uint8_t charLeft[] = {
  0b00010,
  0b00100,
  0b01000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00000
};

const uint8_t charRight[] = {
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00010,
  0b00100,
  0b01000,
  0b00000
};

const uint8_t charBlock[] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

const uint8_t charHalfBlock[] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

/**
 * Constructeur de la classe DisplayManager
 * Initialise les variables membres
 */
DisplayManager::DisplayManager() 
  : lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS),
    lcdInitialized(false),
    lastDisplayUpdate(0),
    lastMenuUpdate(0),
    lastDisplayCheck(0),
    currentDisplayState(0),
    displayNeedsUpdate(true),
    currentMenu(MENU_MAIN),
    currentMenuSelection(0) {
}

/**
 * Destructeur de la classe DisplayManager
 * Libère les ressources si nécessaire
 */
DisplayManager::~DisplayManager() {
  // Rien à libérer
}

/**
 * Initialise la communication I2C pour l'écran LCD
 */
void DisplayManager::setupI2C() {
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50);  // Court délai pour stabiliser le bus I2C
  LOG_INFO("DISPLAY", "Initialisation I2C pour l'écran LCD");
}

/**
 * Initialise l'écran LCD avec plusieurs tentatives en cas d'échec
 * @return true si l'initialisation a réussi, false sinon
 */
bool DisplayManager::initLCD() {
  LOG_INFO("DISPLAY", "Tentative d'initialisation de l'écran LCD 2004...");
  
  // Plusieurs tentatives d'initialisation
  for (int tentative = 1; tentative <= MAX_INIT_ATTEMPTS; tentative++) {
    LOG_INFO("DISPLAY", "Essai %d/%d...", tentative, MAX_INIT_ATTEMPTS);
    
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    // Test simple pour vérifier si l'écran fonctionne correctement
    lcd.setCursor(0, 0);
    lcd.print("Test LCD");
    
    // Si nous arrivons ici sans plantage, l'initialisation a réussi
    LOG_INFO("DISPLAY", "Écran LCD 2004 initialisé avec succès!");
    lcdInitialized = true;
    
    // Créer les caractères personnalisés
    createCustomChars();
    
    return true;
  }
  
  LOG_ERROR("DISPLAY", "ERREUR CRITIQUE: Échec d'initialisation de l'écran LCD après plusieurs tentatives");
  lcdInitialized = false;
  return false;
}

/**
 * Crée les caractères personnalisés pour l'affichage
 */
void DisplayManager::createCustomChars() {
  if (!lcdInitialized) {
    return;
  }
  
  // Création des caractères personnalisés
  lcd.createChar(0, const_cast<uint8_t*>(charDirection));  // Flèche de direction
  lcd.createChar(1, const_cast<uint8_t*>(charLeft));       // Flèche gauche
  lcd.createChar(2, const_cast<uint8_t*>(charRight));      // Flèche droite
  lcd.createChar(3, const_cast<uint8_t*>(charBlock));      // Bloc plein
  lcd.createChar(4, const_cast<uint8_t*>(charHalfBlock));  // Demi-bloc
  
  LOG_INFO("DISPLAY", "Caractères personnalisés créés");
}

/**
 * Affiche l'écran de bienvenue sur l'écran LCD
 * @param simpleInit Si true, affiche seulement "INIT" pour un démarrage rapide
 */
void DisplayManager::displayWelcomeScreen(bool simpleInit) {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
  
  if (simpleInit) {
    // Affichage simplifié "INIT" au centre de l'écran
    centerText(1, "INIT");
    return;
  }
  
  // Titre principal
  centerText(0, "Kite PiloteV3");
  
  // Sous-titre
  centerText(1, "Initialisation...");
  
  // Version
  char versionBuffer[16];
  snprintf(versionBuffer, sizeof(versionBuffer), "Version %s", SYSTEM_VERSION);
  centerText(2, versionBuffer);
  
  // Dessiner une barre de progression en bas
  drawProgressBar(3, 50);  // 50% pour commencer
  
  delay(1000);
  
  // Mettre à jour la barre de progression
  drawProgressBar(3, 100);  // 100% pour terminer
  
  delay(500);
}

/**
 * Efface l'écran LCD
 */
void DisplayManager::clear() {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
}

/**
 * Centre un texte sur une ligne de l'écran LCD
 * @param row Numéro de la ligne (0-3)
 * @param text Texte à centrer
 */
void DisplayManager::centerText(uint8_t row, const char* text) {
  if (!lcdInitialized || row >= LCD_ROWS) {
    return;
  }
  
  int textLength = strlen(text);
  int position = (LCD_COLS - textLength) / 2;
  
  if (position < 0) {
    position = 0;
  }
  
  lcd.setCursor(position, row);
  lcd.print(text);
}

/**
 * Affiche un élément de menu
 * @param row Ligne de l'écran (0-3)
 * @param text Texte du menu
 * @param selected Si l'élément est sélectionné
 */
void DisplayManager::printMenuItem(uint8_t row, const char* text, bool selected) {
  if (!lcdInitialized || row >= LCD_ROWS) {
    return;
  }
  
  lcd.setCursor(0, row);
  
  if (selected) {
    lcd.print(">");
  } else {
    lcd.print(" ");
  }
  
  lcd.print(text);
  
  // Effacer le reste de la ligne
  int textLength = strlen(text) + 1; // +1 pour le curseur
  for (int i = textLength; i < LCD_COLS; i++) {
    lcd.print(" ");
  }
}

/**
 * Fait défiler un texte long sur une ligne
 * @param row Ligne de l'écran (0-3)
 * @param text Texte à faire défiler
 * @param maxLength Longueur maximale visible
 */
void DisplayManager::scrollLongText(uint8_t row, const char* text, int maxLength) {
  if (!lcdInitialized || row >= LCD_ROWS) {
    return;
  }
  
  int textLength = strlen(text);
  
  if (textLength <= maxLength) {
    // Le texte tient dans l'espace disponible
    lcd.setCursor(0, row);
    lcd.print(text);
    
    // Effacer le reste de la ligne
    for (int i = textLength; i < LCD_COLS; i++) {
      lcd.print(" ");
    }
    
    return;
  }
  
  // Pour les textes plus longs, on implémentera un défilement dans une version future
  // Pour l'instant, on tronque simplement le texte
  lcd.setCursor(0, row);
  for (int i = 0; i < maxLength - 3; i++) {
    lcd.print(text[i]);
  }
  lcd.print("...");
}

/**
 * Affiche un message sur l'écran LCD
 * @param title Titre du message à afficher
 * @param message Contenu du message
 */
void DisplayManager::displayMessage(const char* title, const char* message) {
  if (!lcdInitialized) {
    LOG_INFO("DISPLAY", "%s: %s", title, message);
    return;
  }
  
  lcd.clear();
  
  // Titre sur la première ligne
  centerText(0, title);
  
  // Message sur les lignes suivantes
  int msgLen = strlen(message);
  int startIdx = 0;
  
  // Ajouter le support des sauts de ligne
  for (int row = 1; row < LCD_ROWS && startIdx < msgLen; row++) {
    // Rechercher un saut de ligne ou calculer combien de caractères on peut afficher
    int endIdx = startIdx;
    int lastSpaceIdx = -1;
    
    while (endIdx < msgLen && endIdx < startIdx + LCD_COLS) {
      if (message[endIdx] == '\n') {
        break;
      }
      if (message[endIdx] == ' ') {
        lastSpaceIdx = endIdx;
      }
      endIdx++;
    }
    
    // Déterminer où couper la ligne
    int cutIdx;
    if (endIdx >= msgLen) {
      // Fin du message atteinte
      cutIdx = msgLen;
    } else if (message[endIdx] == '\n') {
      // Saut de ligne trouvé
      cutIdx = endIdx;
    } else if (lastSpaceIdx != -1 && lastSpaceIdx > startIdx) {
      // Couper au dernier espace
      cutIdx = lastSpaceIdx;
    } else {
      // Pas d'espace trouvé, couper au maximum
      cutIdx = endIdx;
    }
    
    // Afficher la ligne
    lcd.setCursor(0, row);
    for (int i = startIdx; i < cutIdx; i++) {
      if (message[i] != '\n') {
        lcd.print(message[i]);
      }
    }
    
    // Effacer le reste de la ligne
    for (int i = cutIdx - startIdx; i < LCD_COLS; i++) {
      lcd.print(" ");
    }
    
    // Préparer pour la ligne suivante
    if (cutIdx < msgLen && message[cutIdx] == '\n') {
      startIdx = cutIdx + 1; // Sauter le \n
    } else if (cutIdx < msgLen && message[cutIdx] == ' ') {
      startIdx = cutIdx + 1; // Sauter l'espace
    } else {
      startIdx = cutIdx;
    }
  }
  
  // Log également pour le moniteur série
  LOG_INFO("DISPLAY", "%s: %s", title, message);
}

/**
 * Surcharge de displayMessage pour accepter des String
 */
void DisplayManager::displayMessage(const String& title, const String& message) {
  displayMessage(title.c_str(), message.c_str());
}

/**
 * Affiche l'écran principal
 * Affiche un résumé des informations essentielles
 */
void DisplayManager::updateMainDisplay() {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
  centerText(0, "Kite PiloteV3");
  
  // Seconde ligne: statut WiFi
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi: ");
    lcd.print(WiFi.SSID());
  } else {
    lcd.print("WiFi: Déconnecté");
  }
  
  // Troisième ligne: espace pour les messages système
  lcd.setCursor(0, 2);
  lcd.print("Appuyez pour menu");
  
  // Quatrième ligne: uptime
  lcd.setCursor(0, 3);
  lcd.print("Uptime: ");
  unsigned long uptime = millis() / 1000;
  lcd.print(uptime);
  lcd.print("s");
}

/**
 * Affiche les informations de direction et trim
 * @param direction Valeur de direction (-100 à +100)
 * @param trim Valeur de trim (-100 à +100)
 */
void DisplayManager::updateDirectionTrimDisplay(int direction, int trim) {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
  centerText(0, "Contrôle Direction");
  
  // Afficher la direction
  lcd.setCursor(0, 1);
  lcd.print("Dir:");
  drawDirection(1, direction);
  
  // Afficher la valeur numérique
  lcd.setCursor(16, 1);
  if (direction >= 0) {
    lcd.print("+");
  }
  lcd.print(direction);
  
  // Afficher le trim
  lcd.setCursor(0, 2);
  lcd.print("Trim:");
  drawDirection(2, trim);
  
  // Afficher la valeur numérique
  lcd.setCursor(16, 2);
  if (trim >= 0) {
    lcd.print("+");
  }
  lcd.print(trim);
  
  // Ligne d'aide
  lcd.setCursor(0, 3);
  lcd.print("POT1:Dir POT2:Trim");
}

/**
 * Affiche les informations de longueur de ligne
 * @param length Valeur de longueur (0 à 100)
 */
void DisplayManager::updateLineLengthDisplay(int length) {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
  centerText(0, "Longueur des Lignes");
  
  // Afficher la longueur
  lcd.setCursor(0, 1);
  lcd.print("Longueur: ");
  lcd.print(length);
  lcd.print("%");
  
  // Barre de progression
  drawProgressBar(2, length);
  
  // Ligne d'aide
  lcd.setCursor(0, 3);
  lcd.print("POT3: Longueur");
}

/**
 * Affiche les informations WiFi
 * @param ssid Nom du réseau WiFi
 * @param ip Adresse IP
 */
void DisplayManager::displayWiFiInfo(const String& ssid, const IPAddress& ip) {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
  centerText(0, "Informations WiFi");
  
  // SSID
  lcd.setCursor(0, 1);
  lcd.print("SSID: ");
  
  // Tronquer le SSID s'il est trop long
  if (ssid.length() > LCD_COLS - 6) {
    lcd.print(ssid.substring(0, LCD_COLS - 9));
    lcd.print("...");
  } else {
    lcd.print(ssid);
  }
  
  // Adresse IP
  lcd.setCursor(0, 2);
  lcd.print("IP: ");
  char ipBuffer[16];
  snprintf(ipBuffer, sizeof(ipBuffer), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  lcd.print(ipBuffer);
  
  // État
  lcd.setCursor(0, 3);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("État: Connecté");
  } else {
    lcd.print("État: Déconnecté");
  }
}

/**
 * Affiche les statistiques système
 */
void DisplayManager::updateSystemDisplay() {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
  centerText(0, "Système");
  
  // Utilisation de la mémoire
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint8_t heapPercent = (freeHeap * 100) / totalHeap;
  
  lcd.setCursor(0, 1);
  lcd.print("Mém: ");
  lcd.print(freeHeap / 1024);
  lcd.print("/");
  lcd.print(totalHeap / 1024);
  lcd.print("KB");
  
  // CPU et température
  lcd.setCursor(0, 2);
  lcd.print("CPU: ");
  lcd.print(ESP.getCpuFreqMHz());
  lcd.print("MHz ");
  
  // Uptime
  lcd.setCursor(0, 3);
  lcd.print("Uptime: ");
  unsigned long uptime = millis() / 1000;
  lcd.print(uptime);
  lcd.print("s");
}

/**
 * Affiche la progression d'une mise à jour OTA
 * @param current Nombre d'octets actuellement transférés
 * @param final Taille totale du fichier à transférer
 */
void DisplayManager::displayOTAProgress(size_t current, size_t final) {
  if (!lcdInitialized) {
    return;
  }
  
  // Calculer le pourcentage
  int percent = (current * 100) / final;
  
  // Afficher l'écran de progression OTA
  lcd.clear();
  centerText(0, "Mise à jour OTA");
  
  // Taille
  lcd.setCursor(0, 1);
  lcd.print("Taille: ");
  lcd.print(final / 1024);
  lcd.print("KB");
  
  // Pourcentage
  lcd.setCursor(0, 2);
  lcd.print("Prog: ");
  lcd.print(percent);
  lcd.print("%");
  
  // Barre de progression
  drawProgressBar(3, percent);
}

/**
 * Affiche le statut final d'une mise à jour OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void DisplayManager::displayOTAStatus(bool success) {
  if (!lcdInitialized) {
    return;
  }
  
  lcd.clear();
  centerText(0, "Statut OTA");
  
  if (success) {
    centerText(1, "Mise à jour");
    centerText(2, "terminée avec");
    centerText(3, "succès!");
  } else {
    centerText(1, "Échec de la");
    centerText(2, "mise à jour");
    centerText(3, "Réessayez...");
  }
}

/**
 * Dessine une barre de progression
 * @param row Ligne où afficher la barre (0-3)
 * @param percent Pourcentage de progression (0-100)
 */
void DisplayManager::drawProgressBar(uint8_t row, uint8_t percent) {
  if (!lcdInitialized || row >= LCD_ROWS) {
    return;
  }
  
  // Limiter le pourcentage à 0-100
  percent = constrain(percent, 0, 100);
  
  // Une barre de progression de 20 caractères pour LCD 2004
  int numBlocks = (percent * LCD_COLS) / 100;
  
  lcd.setCursor(0, row);
  
  // Afficher les blocs pleins
  for (int i = 0; i < numBlocks; i++) {
    lcd.write(3);  // Bloc plein (caractère personnalisé)
  }
  
  // Afficher les espaces vides
  for (int i = numBlocks; i < LCD_COLS; i++) {
    lcd.print(" ");
  }
}

/**
 * Dessine un indicateur de direction
 * @param row Ligne où afficher l'indicateur (0-3)
 * @param value Valeur de direction (-100 à +100)
 */
void DisplayManager::drawDirection(uint8_t row, int value) {
  if (!lcdInitialized || row >= LCD_ROWS) {
    return;
  }
  
  // Limiter la valeur à -100 à +100
  value = constrain(value, -100, 100);
  
  // Position centrale
  int center = 10;
  
  // Calculer la position de l'indicateur
  int position = center + (value * (center - 1)) / 100;
  position = constrain(position, 1, 19);
  
  lcd.setCursor(0, row);
  
  // Dessiner l'indicateur de direction
  for (int i = 0; i < LCD_COLS; i++) {
    if (i == position) {
      if (value < 0) {
        lcd.write(1);  // Flèche gauche
      } else if (value > 0) {
        lcd.write(2);  // Flèche droite
      } else {
        lcd.write(0);  // Indicateur central
      }
    } else if (i == center) {
      lcd.print("|");  // Marque centrale
    } else {
      lcd.print("-");  // Ligne de base
    }
  }
}

/**
 * Dessine un indicateur de niveau
 * @param row Ligne où afficher l'indicateur (0-3)
 * @param value Valeur de niveau (0 à 100)
 */
void DisplayManager::drawLevel(uint8_t row, int value) {
  if (!lcdInitialized || row >= LCD_ROWS) {
    return;
  }
  
  // Limiter la valeur à 0 à 100
  value = constrain(value, 0, 100);
  
  // Calculer le nombre de blocs à afficher
  int numBlocks = (value * LCD_COLS) / 100;
  
  lcd.setCursor(0, row);
  
  // Dessiner l'indicateur de niveau
  for (int i = 0; i < LCD_COLS; i++) {
    if (i < numBlocks) {
      lcd.write(3);  // Bloc plein
    } else {
      lcd.print(" ");  // Espace vide
    }
  }
}

/**
 * Force un rafraîchissement de l'affichage au prochain cycle
 */
void DisplayManager::forceRefresh() {
  displayNeedsUpdate = true;
}

/**
 * Passe à l'écran suivant dans la rotation
 */
void DisplayManager::nextScreen() {
  currentDisplayState = (currentDisplayState + 1) % DISPLAY_STATE_COUNT;
  displayNeedsUpdate = true;
}

/**
 * Met à jour l'affichage selon l'état actuel
 */
void DisplayManager::updateDisplay() {
  if (!lcdInitialized) {
    return;
  }
  
  // Vérifier si une mise à jour est nécessaire
  unsigned long currentTime = millis();
  if (!displayNeedsUpdate && currentTime - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
    return;
  }
  
  lastDisplayUpdate = currentTime;
  displayNeedsUpdate = false;
  
  // Mise à jour selon l'état d'affichage actuel
  switch (currentDisplayState) {
    case DISPLAY_MAIN:
      updateMainDisplay();
      break;
      
    case DISPLAY_DIRECTION_TRIM:
      // Pour le moment, on affiche des valeurs de test
      // Ces valeurs seront remplacées par les vraies valeurs des potentiomètres
      updateDirectionTrimDisplay(0, 0);
      break;
      
    case DISPLAY_LINE_LENGTH:
      // Pour le moment, on affiche une valeur de test
      updateLineLengthDisplay(50);
      break;
      
    case DISPLAY_WIFI_INFO:
      displayWiFiInfo(WiFi.SSID(), WiFi.localIP());
      break;
      
    case DISPLAY_SYSTEM_STATS:
      updateSystemDisplay();
      break;
      
    default:
      // En cas d'état inconnu, revenir à l'écran principal
      currentDisplayState = DISPLAY_MAIN;
      updateMainDisplay();
      break;
  }
}

/**
 * Vérifie l'état de l'écran LCD
 * Tente de le réinitialiser si nécessaire
 */
void DisplayManager::checkDisplayStatus() {
  unsigned long currentTime = millis();
  
  // Vérifier l'état de l'écran périodiquement
  if (currentTime - lastDisplayCheck < DISPLAY_CHECK_INTERVAL) {
    return;
  }
  
  lastDisplayCheck = currentTime;
  
  // Si l'écran n'est pas initialisé, tenter de l'initialiser
  if (!lcdInitialized) {
    LOG_WARNING("DISPLAY", "Écran LCD non initialisé, tentative de réinitialisation...");
    initLCD();
    
    if (lcdInitialized) {
      LOG_INFO("DISPLAY", "Réinitialisation réussie");
      displayNeedsUpdate = true;
    }
  }
}

/**
 * Vérifie si l'écran LCD est initialisé
 * @return true si l'écran est initialisé, false sinon
 */
bool DisplayManager::isInitialized() const {
  return lcdInitialized;
}

/**
 * Affiche un menu
 * @param menu Menu à afficher
 */
void DisplayManager::showMenu(MenuState menu) {
  if (!lcdInitialized) {
    return;
  }
  
  currentMenu = menu;
  currentMenuSelection = 0;
  
  lcd.clear();
  
  switch (menu) {
    case MENU_MAIN:
      centerText(0, "Menu Principal");
      printMenuItem(1, "Controle", currentMenuSelection == 0);
      printMenuItem(2, "Paramètres", currentMenuSelection == 1);
      printMenuItem(3, "Système", currentMenuSelection == 2);
      break;
      
    case MENU_SETTINGS:
      centerText(0, "Paramètres");
      printMenuItem(1, "Calibration", currentMenuSelection == 0);
      printMenuItem(2, "WiFi", currentMenuSelection == 1);
      printMenuItem(3, "Retour", currentMenuSelection == 2);
      break;
      
    case MENU_CONTROL:
      centerText(0, "Contrôle");
      printMenuItem(1, "Mode Direction", currentMenuSelection == 0);
      printMenuItem(2, "Mode Longueur", currentMenuSelection == 1);
      printMenuItem(3, "Retour", currentMenuSelection == 2);
      break;
      
    case MENU_WIFI:
      centerText(0, "WiFi");
      printMenuItem(1, "Connexion", currentMenuSelection == 0);
      printMenuItem(2, "Info", currentMenuSelection == 1);
      printMenuItem(3, "Retour", currentMenuSelection == 2);
      break;
      
    case MENU_SYSTEM:
      centerText(0, "Système");
      printMenuItem(1, "Info", currentMenuSelection == 0);
      printMenuItem(2, "Mise à jour OTA", currentMenuSelection == 1);
      printMenuItem(3, "Retour", currentMenuSelection == 2);
      break;
  }
}

/**
 * Déplace la sélection vers le haut dans le menu
 */
void DisplayManager::menuUp() {
  if (!lcdInitialized) {
    return;
  }
  
  // Nombre d'items dans chaque menu
  const uint8_t menuItemCounts[] = {3, 3, 3, 3, 3};
  
  if (currentMenuSelection > 0) {
    currentMenuSelection--;
    showMenu(currentMenu);
  }
}

/**
 * Déplace la sélection vers le bas dans le menu
 */
void DisplayManager::menuDown() {
  if (!lcdInitialized) {
    return;
  }
  
  // Nombre d'items dans chaque menu
  const uint8_t menuItemCounts[] = {3, 3, 3, 3, 3};
  
  if (currentMenuSelection < menuItemCounts[currentMenu] - 1) {
    currentMenuSelection++;
    showMenu(currentMenu);
  }
}

/**
 * Sélectionne l'élément actuel du menu
 */
void DisplayManager::menuSelect() {
  if (!lcdInitialized) {
    return;
  }
  
  switch (currentMenu) {
    case MENU_MAIN:
      switch (currentMenuSelection) {
        case 0: // Contrôle
          showMenu(MENU_CONTROL);
          break;
        case 1: // Paramètres
          showMenu(MENU_SETTINGS);
          break;
        case 2: // Système
          showMenu(MENU_SYSTEM);
          break;
      }
      break;
      
    case MENU_SETTINGS:
      switch (currentMenuSelection) {
        case 0: // Calibration
          // Fonction de calibration à implémenter
          displayMessage("Calibration", "Mode calibration\nDéplacez les pots");
          break;
        case 1: // WiFi
          showMenu(MENU_WIFI);
          break;
        case 2: // Retour
          showMenu(MENU_MAIN);
          break;
      }
      break;
      
    case MENU_CONTROL:
      switch (currentMenuSelection) {
        case 0: // Mode Direction
          currentDisplayState = DISPLAY_DIRECTION_TRIM;
          updateDisplay();
          break;
        case 1: // Mode Longueur
          currentDisplayState = DISPLAY_LINE_LENGTH;
          updateDisplay();
          break;
        case 2: // Retour
          showMenu(MENU_MAIN);
          break;
      }
      break;
      
    case MENU_WIFI:
      switch (currentMenuSelection) {
        case 0: // Connexion
          // Fonction de connexion WiFi à implémenter
          displayMessage("WiFi", "Connexion en cours...");
          break;
        case 1: // Info
          currentDisplayState = DISPLAY_WIFI_INFO;
          updateDisplay();
          break;
        case 2: // Retour
          showMenu(MENU_SETTINGS);
          break;
      }
      break;
      
    case MENU_SYSTEM:
      switch (currentMenuSelection) {
        case 0: // Info
          currentDisplayState = DISPLAY_SYSTEM_STATS;
          updateDisplay();
          break;
        case 1: // Mise à jour OTA
          // Fonction OTA à implémenter
          displayMessage("OTA", "Préparation OTA...");
          break;
        case 2: // Retour
          showMenu(MENU_MAIN);
          break;
      }
      break;
  }
}

/**
 * Retourne au menu précédent
 */
void DisplayManager::menuBack() {
  if (!lcdInitialized) {
    return;
  }
  
  switch (currentMenu) {
    case MENU_MAIN:
      // Dans le menu principal, on revient à l'affichage normal
      currentDisplayState = DISPLAY_MAIN;
      updateDisplay();
      break;
      
    case MENU_SETTINGS:
    case MENU_CONTROL:
    case MENU_SYSTEM:
      showMenu(MENU_MAIN);
      break;
      
    case MENU_WIFI:
      showMenu(MENU_SETTINGS);
      break;
  }
}
