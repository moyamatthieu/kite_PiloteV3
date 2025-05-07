/*
  -----------------------
  Kite PiloteV3 - Module DisplayManager (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire d'affichage LCD.
  
  Version: 1.0.0
  Date: 6 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/io/display_manager.h"
#include "utils/logging.h"

// Caractères personnalisés
const uint8_t charUp[] = {
  0b00100, 0b01110, 0b11111, 0b00100, 0b00100, 0b00000, 0b00000, 0b00000
};
const uint8_t charDown[] = {
  0b00000, 0b00000, 0b00000, 0b00100, 0b00100, 0b11111, 0b01110, 0b00100
};
const uint8_t charBlock[] = {
  0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111
};

/**
 * Constructeur - initialise l'écran LCD
 */
DisplayManager::DisplayManager() : lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS), lcdInitialized(false) {
}

/**
 * Destructeur - libère les ressources
 */
DisplayManager::~DisplayManager() {
  // Rien à libérer spécifiquement
}

/**
 * Configure l'interface I2C pour la communication avec l'écran LCD
 */
void DisplayManager::setupI2C() {
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50); // Petit délai pour stabiliser l'I2C
  LOG_INFO("DISPLAY", "I2C configuré: SDA=%d, SCL=%d", I2C_SDA, I2C_SCL);
}

/**
 * Initialise l'écran LCD
 * @return true si l'initialisation réussit, false sinon
 */
bool DisplayManager::initLCD() {
  LOG_INFO("DISPLAY", "Initialisation de l'écran LCD...");
  
  // Initialisation avec plusieurs tentatives
  for (int attempt = 1; attempt <= 3; attempt++) {
    LOG_INFO("DISPLAY", "Tentative %d/3", attempt);
    
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    // Test d'affichage simple
    lcd.setCursor(0, 0);
    lcd.print("Initialisation...");
    delay(100);
    
    // Si on arrive ici, l'initialisation a réussi
    lcdInitialized = true;
    LOG_INFO("DISPLAY", "Écran LCD initialisé avec succès");
    return true;
  }
  
  // Échec après plusieurs tentatives
  LOG_ERROR("DISPLAY", "Échec d'initialisation de l'écran LCD");
  lcdInitialized = false;
  return false;
}

/**
 * Crée les caractères personnalisés dans la mémoire du LCD
 */
void DisplayManager::createCustomChars() {
  if (!lcdInitialized) return;
  
  lcd.createChar(0, (uint8_t*)charUp);
  lcd.createChar(1, (uint8_t*)charDown);
  lcd.createChar(2, (uint8_t*)charBlock);
  
  LOG_INFO("DISPLAY", "Caractères personnalisés créés");
}

/**
 * Efface l'écran LCD
 */
void DisplayManager::clear() {
  if (!lcdInitialized) return;
  lcd.clear();
}

/**
 * Centre le texte sur une ligne du LCD
 * @param row Numéro de ligne (0-3)
 * @param text Texte à centrer
 */
void DisplayManager::centerText(uint8_t row, const char* text) {
  if (!lcdInitialized || row >= LCD_ROWS) return;
  
  int textLen = strlen(text);
  int position = (LCD_COLS - textLen) / 2;
  position = max(0, position);
  
  lcd.setCursor(position, row);
  lcd.print(text);
}

/**
 * Affiche un message avec titre sur l'écran LCD
 * @param title Titre du message (1ère ligne)
 * @param message Corps du message (lignes suivantes)
 */
void DisplayManager::displayMessage(const char* title, const char* message) {
  if (!lcdInitialized) {
    LOG_INFO("DISPLAY", "%s: %s", title, message);
    return;
  }
  
  clear();
  centerText(0, title);
  
  // Afficher le message sur plusieurs lignes si nécessaire
  int msgLen = strlen(message);
  int charsPerLine = LCD_COLS;
  
  for (int line = 0; line < 3 && msgLen > 0; line++) {
    int toCopy = min(msgLen, charsPerLine);
    lcd.setCursor(0, line + 1);
    
    // Copier une partie du message
    char lineBuffer[LCD_COLS + 1];
    strncpy(lineBuffer, message, toCopy);
    lineBuffer[toCopy] = '\0';
    
    lcd.print(lineBuffer);
    message += toCopy;
    msgLen -= toCopy;
  }
}

/**
 * Met à jour l'affichage principal
 * Affiche les informations principales du système
 */
void DisplayManager::updateMainDisplay() {
  if (!lcdInitialized) return;
  
  clear();
  centerText(0, "Kite PiloteV3");
  
  // Ligne 1: État du WiFi
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi: ");
    lcd.print(WiFi.SSID());
  } else {
    lcd.print("WiFi: Deconnecte");
  }
  
  // Ligne 2: État du système
  lcd.setCursor(0, 2);
  lcd.print("Systeme: Pret");
  
  // Ligne 3: Temps de fonctionnement
  lcd.setCursor(0, 3);
  unsigned long uptime = millis() / 1000;
  lcd.print("Uptime: ");
  lcd.print(uptime);
  lcd.print("s");
}

/**
 * Affiche les informations WiFi sur l'écran LCD
 * @param ssid Nom du réseau WiFi
 * @param ip Adresse IP attribuée
 */
void DisplayManager::displayWiFiInfo(const char* ssid, IPAddress ip) {
  if (!lcdInitialized) return;
  
  clear();
  centerText(0, "Info WiFi");
  
  // Ligne 1: SSID
  lcd.setCursor(0, 1);
  lcd.print("SSID: ");
  lcd.print(ssid);
  
  // Ligne 2: Adresse IP
  lcd.setCursor(0, 2);
  lcd.print("IP: ");
  lcd.print(ip.toString());
  
  // Ligne 3: Force du signal
  lcd.setCursor(0, 3);
  lcd.print("Signal: ");
  lcd.print(WiFi.RSSI());
  lcd.print(" dBm");
}

/**
 * Affiche l'écran d'accueil
 * @param simpleMode Mode simple (true) ou complet (false)
 */
void DisplayManager::displayWelcomeScreen(bool simpleMode) {
  if (!lcdInitialized) return;
  
  clear();
  
  if (simpleMode) {
    // Mode simple: juste le titre
    centerText(0, "Kite PiloteV3");
    centerText(1, "Bienvenue");
    centerText(3, "Initialisation...");
  } else {
    // Mode complet: plus d'informations
    centerText(0, "Kite PiloteV3");
    centerText(1, "Systeme d'autopilote");
    centerText(2, "Version 3.0.0");
    centerText(3, "Initialisation...");
  }
}

/**
 * Affiche les statistiques système sur l'écran LCD
 */
void DisplayManager::displaySystemStats() {
  if (!lcdInitialized) return;
  
  clear();
  centerText(0, "Statistiques");
  
  // Mémoire
  lcd.setCursor(0, 1);
  lcd.print("Mem: ");
  lcd.print(ESP.getFreeHeap() / 1024);
  lcd.print("/");
  lcd.print(ESP.getHeapSize() / 1024);
  lcd.print(" KB");
  
  // CPU
  lcd.setCursor(0, 2);
  lcd.print("CPU: ");
  lcd.print(ESP.getCpuFreqMHz());
  lcd.print(" MHz");
  
  // Uptime
  lcd.setCursor(0, 3);
  lcd.print("Uptime: ");
  lcd.print(millis() / 1000);
  lcd.print("s");
}

/**
 * Affiche la progression d'une mise à jour OTA
 * @param current Nombre d'octets téléchargés
 * @param total Taille totale du fichier
 */
void DisplayManager::displayOTAProgress(size_t current, size_t total) {
  if (!lcdInitialized) return;
  
  clear();
  centerText(0, "Mise a jour OTA");
  
  // Afficher les valeurs
  lcd.setCursor(0, 1);
  lcd.print(current);
  lcd.print(" / ");
  lcd.print(total);
  lcd.print(" octets");
  
  // Pourcentage
  int percent = (current * 100) / total;
  lcd.setCursor(0, 2);
  lcd.print("Progression: ");
  lcd.print(percent);
  lcd.print("%");
  
  // Barre de progression
  drawProgressBar(3, percent);
}

/**
 * Affiche le résultat d'une mise à jour OTA
 * @param success true si la mise à jour est réussie, false sinon
 */
void DisplayManager::displayOTAStatus(bool success) {
  if (!lcdInitialized) return;
  
  clear();
  centerText(0, "Resultat OTA");
  
  if (success) {
    centerText(1, "Mise a jour reussie!");
    centerText(3, "Redemarrage...");
  } else {
    centerText(1, "Echec de la mise");
    centerText(2, "a jour!");
    centerText(3, "Veuillez reessayer");
  }
}

/**
 * Dessine une barre de progression sur une ligne du LCD
 * @param row Numéro de ligne (0-3)
 * @param percent Pourcentage de progression (0-100)
 */
void DisplayManager::drawProgressBar(uint8_t row, uint8_t percent) {
  if (!lcdInitialized || row >= LCD_ROWS) return;
  
  // Limiter le pourcentage
  percent = constrain(percent, 0, 100);
  
  // Calculer le nombre de blocs pleins
  int numBlocks = (percent * LCD_COLS) / 100;
  
  lcd.setCursor(0, row);
  
  // Dessiner les blocs pleins
  for (int i = 0; i < numBlocks; i++) {
    lcd.write(2);  // Caractère de bloc plein
  }
  
  // Espaces pour le reste
  for (int i = numBlocks; i < LCD_COLS; i++) {
    lcd.print(" ");
  }
}

/**
 * Affiche l'état en direct sur l'écran LCD
 * @param direction Direction actuelle
 * @param trim Valeur de trim
 * @param lineLength Longueur de ligne en pourcentage
 * @param wifi État du WiFi (true si connecté)
 * @param uptime Temps de fonctionnement en secondes
 */
void DisplayManager::displayLiveStatus(int direction, int trim, int lineLength, bool wifi, unsigned long uptime) {
  if (!lcdInitialized) return;
  clear();
  // Ligne 1 : WiFi et uptime
  lcd.setCursor(0, 0);
  lcd.print(wifi ? "WiFi:OK " : "WiFi:-- ");
  lcd.print("Up:");
  lcd.print(uptime);
  lcd.print("s");
  // Ligne 2 : Direction et Trim
  lcd.setCursor(0, 1);
  lcd.print("Dir:");
  lcd.print(direction);
  lcd.print("   Trim:");
  lcd.print(trim);
  // Ligne 3 : Longueur de ligne
  lcd.setCursor(0, 2);
  lcd.print("Longueur:");
  lcd.print(lineLength);
  lcd.print("%");
  // Ligne 4 : Aide
  lcd.setCursor(0, 3);
  lcd.print("Pots=Ctrl  Btns=Menu");
}