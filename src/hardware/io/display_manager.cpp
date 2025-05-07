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

// Variable globale pour l'état du système (utilisée dans l'affichage)
static uint8_t systemStatus = 100; // 100% par défaut, représente l'état de santé du système

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
DisplayManager::DisplayManager() : 
  lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS), 
  lcdInitialized(false), 
  lastInitTime(0),
  lastCheckTime(0),
  recoveryAttempts(0),
  lastUpdateTime(0),
  successfulUpdates(0) {
  // Les variables sont initialisées dans la liste d'initialisation
}

/**
 * Destructeur - libère les ressources
 */
DisplayManager::~DisplayManager() {
  // Rien à libérer spécifiquement
}

/**
 * Configure l'interface I2C pour la communication avec l'écran LCD
 * @return true si l'initialisation réussit, false sinon
 */
bool DisplayManager::setupI2C() {
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50); // Petit délai pour stabiliser l'I2C
  
  // Vérifier si le bus I2C fonctionne
  Wire.beginTransmission(LCD_I2C_ADDR);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    LOG_INFO("DISPLAY", "I2C configuré: SDA=%d, SCL=%d", I2C_SDA, I2C_SCL);
    i2cInitialized = true;
    return true;
  } else {
    LOG_ERROR("DISPLAY", "Erreur I2C lors de la configuration (code: %d)", error);
    i2cInitialized = false;
    return false;
  }
}

/**
 * Initialise l'écran LCD avec une stratégie plus robuste
 * @return true si l'initialisation réussit, false sinon
 */
bool DisplayManager::initLCD() {
  // Marquer comme non initialisé au début
  lcdInitialized = false;
  
  // Réinitialiser les variables d'état
  recoveryAttempts = 0;
  
  LOG_INFO("DISPLAY", "Initialisation de l'écran LCD");
  
  // Configuration I2C si nécessaire
  if (!i2cInitialized) {
    LOG_INFO("DISPLAY", "Configuration de l'I2C...");
    
    // Réinitialiser le bus I2C
    Wire.end();
    delay(100);
    
    // Commencer avec une fréquence plus basse
    if (!setupI2C()) {
      LOG_ERROR("DISPLAY", "Échec de configuration I2C - tentative avec fréquence basse");
      
      // Nouvelle tentative avec une fréquence plus basse
      Wire.setClock(100000); // 100 kHz au lieu du 400 kHz par défaut
      if (!setupI2C()) {
        LOG_ERROR("DISPLAY", "Échec de configuration I2C même à basse fréquence");
        return false;
      }
    }
  }
  
  // Tentatives d'initialisation multiples
  const uint8_t maxAttempts = 3;
  for (uint8_t attempt = 1; attempt <= maxAttempts; attempt++) {
    LOG_INFO("DISPLAY", "Tentative %d/%d", attempt, maxAttempts);
    
    // Force de réinitialisation matérielle du bus I2C entre les tentatives
    if (attempt > 1) {
      Wire.end();
      delay(200);
      Wire.begin(I2C_SDA, I2C_SCL);
      Wire.setClock(100000); // Basse fréquence pour plus de fiabilité
      delay(100);
    }
    
    // Test initial du bus I2C - vérification que le LCD répond
    Wire.beginTransmission(LCD_I2C_ADDR);
    byte error = Wire.endTransmission();
    
    if (error != 0) {
      LOG_WARNING("DISPLAY", "Erreur I2C pendant la tentative %d (code: %d)", attempt, error);
      continue; // Passer à la prochaine tentative
    }
    
    // Initialisation en deux étapes pour plus de stabilité
    lcd.init();
    delay(150); // Délai critique pour la stabilisation du matériel
    
    // Second init avec reset complet
    lcd.begin(LCD_COLS, LCD_ROWS);
    delay(100);
    
    // Tester l'écran avec des opérations de base
    lcd.clear();
    lcd.home();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Test LCD");
    
    // Vérification supplémentaire que l'écran est réellement fonctionnel
    if (checkLCDConnection()) {
      LOG_INFO("DISPLAY", "Écran LCD initialisé avec succès");
      
      // Compléter le processus d'initialisation
      lcd.clear();
      createCustomChars();
      
      // Mettre à jour l'état
      lcdInitialized = true;
      initAttemptCount = attempt;
      lastInitTime = millis();
      
      return true;
    }
    
    LOG_WARNING("DISPLAY", "Tentative %d a échoué", attempt);
    delay(300); // Délai entre les tentatives
  }
  
  LOG_ERROR("DISPLAY", "Échec d'initialisation après %d tentatives", maxAttempts);
  
  // Permettre le fonctionnement sans écran LCD
  LOG_WARNING("DISPLAY", "Poursuite du fonctionnement sans écran LCD");
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
 * Met à jour l'écran principal avec optimisation de la fréquence
 * de rafraîchissement et protection contre les mises à jour multiples
 */
void DisplayManager::updateMainDisplay() {
  // Protection contre les mises à jour trop fréquentes
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime < DISPLAY_UPDATE_THROTTLE) {
    return;  // Sortir si la dernière mise à jour est trop récente
  }
  lastUpdateTime = currentTime;
  
  // Vérifier que l'écran est initialisé
  if (!lcdInitialized) {
    return;
  }
  
  // Création d'un mutex pour éviter les accès concurrents
  static SemaphoreHandle_t displayMutex = NULL;
  if (displayMutex == NULL) {
    displayMutex = xSemaphoreCreateMutex();
  }
  
  // Tenter d'obtenir le mutex avec un court timeout
  if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    LOG_WARNING("DISPLAY", "Écran déjà en cours d'utilisation, mise à jour ignorée");
    return;
  }
  
  // Protéger contre les exceptions pour ne pas laisser le mutex verrouillé
  try {
    lcd.clear();
    
    // En-tête centrée
    centerText(0, "Kite PiloteV3");
    
    // Ligne 1 - État WiFi
    lcd.setCursor(0, 1);
    if (WiFi.status() == WL_CONNECTED) {
      lcd.print("WiFi: ");
      String ssid = WiFi.SSID();
      // Tronquer le SSID s'il est trop long
      if (ssid.length() > LCD_COLS - 6) {
        ssid = ssid.substring(0, LCD_COLS - 9) + "...";
      }
      lcd.print(ssid);
    } else {
      lcd.print("WiFi: Déconnecté");
    }
    
    // Ligne 2 - Information système
    lcd.setCursor(0, 2);
    lcd.print("Sys: ");
    // Récupérer une valeur de sysStatus sécurisée
    uint8_t status = systemStatus;
    status = constrain(status, 0, 100);
    // Afficher l'état du système de manière compacte
    if (status >= 90) lcd.print("Excellent");
    else if (status >= 75) lcd.print("Bon");
    else if (status >= 50) lcd.print("Normal");
    else if (status >= 25) lcd.print("Faible");
    else lcd.print("Critique");
    
    // Ligne 3 - Affichage du temps de fonctionnement
    lcd.setCursor(0, 3);
    unsigned long uptimeSeconds = millis() / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;
    
    lcd.print("Uptime: ");
    if (hours < 10) lcd.print("0");
    lcd.print(hours);
    lcd.print(":");
    if (minutes < 10) lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");
    if (seconds < 10) lcd.print("0");
    lcd.print(seconds);
    
    // Actualiser le compteur de mises à jour réussies
    successfulUpdates++;
  } 
  catch (...) {
    LOG_ERROR("DISPLAY", "Exception lors de la mise à jour de l'écran principal");
  }
  
  // Libérer le mutex quelle que soit l'issue
  xSemaphoreGive(displayMutex);
}

/**
 * Affiche un message sur l'écran avec protection contre les mises à jour multiples
 * @param title Titre du message
 * @param message Corps du message
 * @param duration Durée d'affichage en ms (0 = permanent)
 */
void DisplayManager::displayMessage(const char* title, const char* message, unsigned long duration) {
  // Vérifier que l'écran est initialisé
  if (!lcdInitialized) {
    LOG_WARNING("DISPLAY", "Écran non initialisé, impossible d'afficher le message");
    return;
  }
  
  // Création d'un mutex pour éviter les accès concurrents
  static SemaphoreHandle_t displayMutex = NULL;
  if (displayMutex == NULL) {
    displayMutex = xSemaphoreCreateMutex();
  }
  
  // Tenter d'obtenir le mutex avec un court timeout
  if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    LOG_WARNING("DISPLAY", "Écran déjà en cours d'utilisation, message ignoré");
    return;
  }
  
  // Protéger contre les exceptions
  try {
    // Effacer l'écran
    lcd.clear();
    
    // Afficher le titre centré
    centerText(0, title);
    
    // Calculer la longueur du message
    size_t msgLen = strlen(message);
    int startIdx = 0;
    
    // Afficher le message sur trois lignes maximum (1, 2, 3)
    for (int row = 1; row < LCD_ROWS && startIdx < msgLen; row++) {
      int charsLeft = msgLen - startIdx;
      int charsToShow = min(charsLeft, LCD_COLS);
      
      // Si la ligne contient plus de caractères que LCD_COLS, chercher un espace
      if (charsLeft > LCD_COLS) {
        int lastSpace = -1;
        for (int i = 0; i < LCD_COLS; i++) {
          if (message[startIdx + i] == ' ') {
            lastSpace = i;
          }
        }
        
        // S'il y a un espace, couper là
        if (lastSpace != -1) {
          charsToShow = lastSpace;
        }
      }
      
      // Positionner le curseur et afficher la partie du message
      lcd.setCursor(0, row);
      for (int i = 0; i < charsToShow; i++) {
        lcd.print(message[startIdx + i]);
      }
      
      // Avancer l'index de départ
      startIdx += charsToShow;
      if (charsToShow < charsLeft && message[startIdx] == ' ') {
        startIdx++; // Sauter l'espace en début de ligne suivante
      }
    }
    
    lastUpdateTime = millis();
  } 
  catch (...) {
    LOG_ERROR("DISPLAY", "Exception lors de l'affichage du message");
  }
  
  // Libérer le mutex
  xSemaphoreGive(displayMutex);
  
  // Si une durée est spécifiée, revenir à l'écran principal après cette durée
  if (duration > 0) {
    // Créer une tâche qui attendra la durée spécifiée puis reviendra à l'écran principal
    static TaskHandle_t messageTaskHandle = NULL;
    
    // Supprimer l'ancienne tâche si elle existe
    if (messageTaskHandle != NULL) {
      vTaskDelete(messageTaskHandle);
      messageTaskHandle = NULL;
    }
    
    // Structure pour passer les paramètres à la tâche
    struct MessageTaskParams {
      DisplayManager* display;
      unsigned long duration;
    };
    
    // Créer le paramètre
    MessageTaskParams* params = new MessageTaskParams{this, duration};
    
    // Créer la tâche
    xTaskCreate(
      [](void* pvParameters) {
        MessageTaskParams* params = static_cast<MessageTaskParams*>(pvParameters);
        vTaskDelay(pdMS_TO_TICKS(params->duration));
        params->display->updateMainDisplay();
        delete params;
        vTaskDelete(NULL);
      },
      "MessageTimer",
      2048,
      params,
      1,
      &messageTaskHandle
    );
  }
}

/**
 * Vérifie l'état de l'écran LCD et tente une récupération si nécessaire
 * @return true si l'écran est fonctionnel, false sinon
 */
bool DisplayManager::checkAndRecover() {
  // Vérifier d'abord si nous sommes déjà initialisés
  if (lcdInitialized && checkLCDConnection()) {
    // L'écran est déjà fonctionnel
    return true;
  }
  
  LOG_WARNING("DISPLAY", "Écran LCD non fonctionnel, tentative de récupération");
  
  // Tentative de réinitialisation matérielle du bus I2C
  Wire.end();
  delay(50);
  
  if (!setupI2C()) {
    LOG_ERROR("DISPLAY", "Échec de réinitialisation du bus I2C");
    return false;
  }
  
  // Tentative de réinitialisation de l'écran LCD
  bool recovered = initLCD();
  
  if (recovered) {
    LOG_INFO("DISPLAY", "Récupération de l'écran LCD réussie");
    
    // Réinitialiser les caractères personnalisés
    createCustomChars();
    
    // Forcer une mise à jour pour vérifier que tout fonctionne
    updateMainDisplay();
  } else {
    LOG_ERROR("DISPLAY", "Échec de récupération de l'écran LCD");
  }
  
  return recovered;
}

/**
 * Affiche l'état actuel OTA avec une barre de progression optimisée
 * @param current Nombre d'octets transférés
 * @param total Taille totale en octets
 */
void DisplayManager::displayOTAProgress(size_t current, size_t total) {
  // Vérifier que l'écran est initialisé
  if (!lcdInitialized) {
    return;
  }
  
  // Protection contre les mises à jour trop fréquentes
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime < 500) { // Limiter à max 2 FPS pour OTA
    return;
  }
  lastUpdateTime = currentTime;
  
  // Création d'un mutex pour éviter les accès concurrents
  static SemaphoreHandle_t displayMutex = NULL;
  if (displayMutex == NULL) {
    displayMutex = xSemaphoreCreateMutex();
  }
  
  // Tenter d'obtenir le mutex avec un court timeout
  if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    return;
  }
  
  try {
    lcd.clear();
    
    // Titre
    centerText(0, "Mise à jour OTA");
    
    // Afficher le pourcentage
    int percentage = (current * 100) / total;
    lcd.setCursor(0, 1);
    lcd.print("Progression: ");
    lcd.print(percentage);
    lcd.print("%");
    
    // Afficher la taille en Ko
    lcd.setCursor(0, 2);
    lcd.print(current / 1024);
    lcd.print("Ko / ");
    lcd.print(total / 1024);
    lcd.print("Ko");
    
    // Barre de progression
    int progressChars = (current * LCD_COLS) / total;
    lcd.setCursor(0, 3);
    for (int i = 0; i < LCD_COLS; i++) {
      if (i < progressChars) {
        lcd.write(byte(3)); // Caractère block plein
      } else {
        lcd.print(".");
      }
    }
  } 
  catch (...) {
    LOG_ERROR("DISPLAY", "Exception lors de l'affichage de la progression OTA");
  }
  
  // Libérer le mutex
  xSemaphoreGive(displayMutex);
}

/**
 * Vérifie la connexion avec l'écran LCD
 * @return true si l'écran répond, false sinon
 */
bool DisplayManager::checkLCDConnection() {
  // Vérifier que le périphérique I2C répond
  Wire.beginTransmission(LCD_I2C_ADDR);
  byte error = Wire.endTransmission();
  
  if (error != 0) {
    LOG_WARNING("DISPLAY", "Écran LCD non détecté (erreur I2C: %d)", error);
    lcdInitialized = false;
    return false;
  }
  
  // Si l'écran était déjà initialisé, on considère qu'il fonctionne toujours
  if (lcdInitialized) {
    return true;
  }
  
  return false;
}

/**
 * Tente de récupérer l'écran LCD en cas de dysfonctionnement
 * @return true si la récupération a réussi, false sinon
 */
bool DisplayManager::recoverLCD() {
  LOG_INFO("DISPLAY", "Tentative de récupération de l'écran LCD...");
  
  // Limiter le nombre de tentatives de récupération consécutives
  recoveryAttempts++;
  if (recoveryAttempts > 3) {
    unsigned long timeSinceLastRecovery = millis() - lastCheckTime;
    if (timeSinceLastRecovery < 60000) {  // 1 minute
      LOG_WARNING("DISPLAY", "Trop de tentatives récentes, attente avant nouvelle tentative");
      return false;
    }
    // Réinitialiser le compteur après une période d'attente
    recoveryAttempts = 1;
  }
  
  // Reset I2C et LCD
  Wire.end();
  delay(100);
  setupI2C();
  
  // Tenter de réinitialiser l'écran
  bool result = initLCD();
  if (result) {
    // Recréer les caractères personnalisés
    createCustomChars();
    recoveryAttempts = 0;
  }
  
  return result;
}

/**
 * Affiche l'écran de bienvenue
 * @param simpleMode Mode simple (true) ou complet (false)
 */
void DisplayManager::displayWelcomeScreen(bool simpleMode) {
  if (!lcdInitialized) return;
  
  lcd.clear();
  
  // En-tête avec titre centré
  centerText(0, "Kite PiloteV3");
  
  if (simpleMode) {
    // Version simple - uniquement "INIT" en grand
    centerText(1, "INITIALISATION");
    centerText(3, SYSTEM_VERSION);
  } else {
    // Version complète avec plus d'informations
    lcd.setCursor(0, 1);
    lcd.print("Version: ");
    lcd.print(SYSTEM_VERSION);
    
    lcd.setCursor(0, 2);
    lcd.print("Build: ");
    lcd.print(SYSTEM_BUILD_DATE);
    
    lcd.setCursor(0, 3);
    lcd.print("System starting...");
  }
}

/**
 * Affiche les informations WiFi
 * @param ssid SSID du réseau WiFi
 * @param ip Adresse IP
 */
void DisplayManager::displayWiFiInfo(const char* ssid, IPAddress ip) {
  if (!lcdInitialized) return;
  
  lcd.clear();
  
  centerText(0, "WiFi Info");
  
  lcd.setCursor(0, 1);
  lcd.print("SSID: ");
  lcd.print(ssid);
  
  lcd.setCursor(0, 2);
  lcd.print("IP: ");
  lcd.print(ip[0]);
  lcd.print(".");
  lcd.print(ip[1]);
  lcd.print(".");
  lcd.print(ip[2]);
  lcd.print(".");
  lcd.print(ip[3]);
  
  lcd.setCursor(0, 3);
  lcd.print("Port: ");
  lcd.print(SERVER_PORT);
}

/**
 * Affiche le statut de fin OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void DisplayManager::displayOTAStatus(bool success) {
  if (!lcdInitialized) return;
  
  lcd.clear();
  
  centerText(0, "Mise à jour OTA");
  
  if (success) {
    centerText(1, "Terminée avec succès");
    centerText(2, "Redémarrage...");
  } else {
    centerText(1, "ÉCHEC");
    centerText(2, "Fonctionnement normal");
    centerText(3, "conservé");
  }
}

/**
 * Affiche les informations de statut en direct
 * @param direction Angle de direction (-90 à +90)
 * @param trim Angle de trim (-45 à +45)
 * @param lineLength Longueur de ligne (0 à 100)
 * @param wifiConnected État de la connexion WiFi
 * @param uptime Temps de fonctionnement en secondes
 */
void DisplayManager::displayLiveStatus(int direction, int trim, int lineLength, bool wifiConnected, unsigned long uptime) {
  if (!lcdInitialized) return;
  
  // Protection contre les mises à jour trop fréquentes
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime < DISPLAY_UPDATE_THROTTLE) {
    return;
  }
  lastUpdateTime = currentTime;
  
  lcd.clear();
  
  // Afficher le titre
  centerText(0, "Statut Kite");
  
  // Ligne 1 - Direction et WiFi
  lcd.setCursor(0, 1);
  lcd.print("Dir:");
  
  // Afficher la direction avec une petite barre de progression
  int dirPos = map(constrain(direction, -90, 90), -90, 90, 0, 9);
  for (int i = 0; i < 10; i++) {
    if (i == dirPos) {
      lcd.print("|");
    } else {
      lcd.print("-");
    }
  }
  
  // Afficher l'icône WiFi
  lcd.setCursor(17, 1);
  if (wifiConnected) {
    lcd.write(byte(2)); // Bloc plein = WiFi connecté
  } else {
    lcd.print("X"); // X = WiFi déconnecté
  }
  
  // Ligne 2 - Trim
  lcd.setCursor(0, 2);
  lcd.print("Trim:");
  
  // Afficher le trim avec une petite barre de progression
  int trimPos = map(constrain(trim, -45, 45), -45, 45, 0, 9);
  for (int i = 0; i < 10; i++) {
    if (i == trimPos) {
      lcd.print("|");
    } else {
      lcd.print("-");
    }
  }
  
  // Ligne 3 - Longueur de ligne et uptime
  lcd.setCursor(0, 3);
  lcd.print("Ligne:");
  
  // Afficher la longueur avec une petite barre de progression
  int lenPos = map(constrain(lineLength, 0, 100), 0, 100, 0, 5);
  for (int i = 0; i < 6; i++) {
    if (i <= lenPos) {
      lcd.write(byte(2)); // Bloc plein
    } else {
      lcd.print(".");
    }
  }
  
  // Afficher le temps de fonctionnement
  lcd.setCursor(12, 3);
  unsigned long hours = uptime / 3600;
  unsigned long minutes = (uptime % 3600) / 60;
  
  lcd.print(hours);
  lcd.print("h");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
}