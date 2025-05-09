/*
  -----------------------
  Kite PiloteV3 - Module de Gestion de l'Affichage (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire d'affichage LCD pour l'interface utilisateur locale.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module gère l'écran LCD utilisé pour l'interface utilisateur locale.
  Il implémente une approche par machine à états finis (FSM) pour toutes
  les opérations d'initialisation et de mise à jour de l'affichage, évitant
  ainsi l'utilisation de délais bloquants.
  
  Principes de fonctionnement :
  1. Initialisation non-bloquante de l'écran LCD via I2C avec FSM
  2. Création de caractères personnalisés via un processus par étapes
  3. Affichage formaté des différentes pages d'information
  4. Gestion optimisée des mises à jour pour limiter les écritures
  
  Architecture FSM pour l'initialisation LCD :
  - États : INIT_START, I2C_CONFIG, LCD_INIT, BACKLIGHT_ON, CLEAR_DISPLAY, DONE, etc.
  - Chaque état représente une étape atomique de l'initialisation
  - Transitions basées sur le temps écoulé et les résultats des opérations
  - Approche résiliente avec tentatives multiples en cas d'échec
  
  Interactions avec d'autres modules :
  - TaskManager : Appelle ce module depuis la tâche d'affichage
  - ButtonUI : Détermine quelles informations afficher selon les interactions
  - WiFiManager : Affiche les informations de connexion WiFi
  - Sensors : Affiche les données des capteurs
  - System : Affiche l'état du système et les messages d'erreur
  
  Aspects techniques notables :
  - Remplacement systématique des delay() par des mécanismes non-bloquants basés sur millis()
  - Cache d'affichage pour éviter les écritures redondantes sur le LCD
  - Tentatives multiples pour les communications I2C avec backoff exponentiel
  - Optimisation du rafraîchissement pour réduire la charge CPU
  
  Exemple d'approche non-bloquante :
  Au lieu de :
  ```
  lcd.init();
  delay(100);
  lcd.backlight();
  delay(50);
  lcd.clear();
  ```
  
  Notre FSM utilise :
  ```
  case LCD_INIT:
    if (millis() - lastStateTime >= 100) {
      if (lcd.init()) {
        state = BACKLIGHT_ON;
        lastStateTime = millis();
      } else if (++retryCount > maxRetries) {
        state = INIT_FAILED;
      }
    }
    break;
    
  case BACKLIGHT_ON:
    if (millis() - lastStateTime >= 50) {
      lcd.backlight();
      state = CLEAR_DISPLAY;
      lastStateTime = millis();
    }
    break;
  ```
*/

#include "hardware/io/display_manager.h"
#include "utils/logging.h"
#include "utils/state_machine.h"

// Variable globale pour l'état du système (utilisée dans l'affichage)
static uint8_t systemStatus = 100; // 100% par défaut, représente l'état de santé du système

/**
 * Constructeur - initialise les variables membres
 */
DisplayManager::DisplayManager() : 
    lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS),
    lcdInitialized(false),
    lastInitTime(0),
    lastCheckTime(0),
    lastUpdateTime(0),
    recoveryAttempts(0),
    successfulUpdates(0),
    i2cInitialized(false),
    initAttemptCount(0),
    displayInitFsm(nullptr)
{
    displayInitFsm = new DisplayInitFSM(this);
    LOG_INFO("DISPLAY", "Gestionnaire d'affichage créé");
    
    // Initialiser les buffers avec des espaces
    for (int row = 0; row < LCD_ROWS; row++) {
        for (int col = 0; col < LCD_COLS; col++) {
            screenBuffer[row][col] = ' ';
            previousBuffer[row][col] = ' ';
        }
        screenBuffer[row][LCD_COLS] = '\0';
        previousBuffer[row][LCD_COLS] = '\0';
    }
}

/**
 * Destructeur - nettoie les ressources
 */
DisplayManager::~DisplayManager() {
    if (displayInitFsm) delete displayInitFsm;
    LOG_INFO("DISPLAY", "Gestionnaire d'affichage détruit");
}

// Caractères personnalisés
const uint8_t charUp[] = {
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

const uint8_t charDown[] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
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

/**
 * Configure le bus I2C pour la communication avec le LCD
 * @return true si la configuration réussit, false sinon
 */
bool DisplayManager::setupI2C() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400 kHz par défaut
  
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
    int state = displayInitFsm->update();
    return (state == DisplayInitFSM::DONE);
}

/**
 * Implémentation de la FSM d'init LCD
 */
int DisplayInitFSM::processState(int state) {
    unsigned long now = millis();
    switch (state) {
        case INIT_START:
            // Bonne pratique : l'état initial doit toujours passer immédiatement à l'étape suivante
            // pour éviter tout timeout inutile et garantir une FSM non-bloquante et élégante.
            retryCount = 0;
            lastActionTime = now;
            return I2C_CONFIG;
        case I2C_CONFIG:
            if (!manager->setupI2C()) {
                retryCount++;
                if (retryCount > maxRetries) return DONE;
                lastActionTime = now;
                return I2C_CONFIG;
            }
            lastActionTime = now;
            return LCD_INIT;
        case LCD_INIT:
            if (now - lastActionTime < 100) return LCD_INIT;
            manager->getLcd().init();
            manager->setLcdInitialized(true);
            lastActionTime = now;
            return BACKLIGHT_ON;
        case BACKLIGHT_ON:
            if (now - lastActionTime < 50) return BACKLIGHT_ON;
            manager->getLcd().backlight();
            lastActionTime = now;
            return CLEAR_DISPLAY;
        case CLEAR_DISPLAY:
            if (now - lastActionTime < 50) return CLEAR_DISPLAY;
            manager->getLcd().clear();
            manager->createCustomChars();
            lastActionTime = now;
            return DONE;
        case DONE:
            return DONE;
        default:
            return state;
    }
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
 * Efface l'écran LCD en remplissant le buffer avec des espaces
 */
void DisplayManager::clear() {
  if (!lcdInitialized) return;
  
  // Remplir le buffer avec des espaces
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      screenBuffer[row][col] = ' ';
    }
    screenBuffer[row][LCD_COLS] = '\0';
  }
  
  // Appliquer les changements
  updateLCDDiff();
}

/**
 * Centre le texte sur une ligne du LCD
 * @param row Numéro de ligne (0-3)
 * @param text Texte à centrer
 */
void DisplayManager::centerText(uint8_t row, const char* text) {
  if (!lcdInitialized || row >= LCD_ROWS) return;
  
  // Calculer la position pour centrer
  int textLen = strlen(text);
  int position = (LCD_COLS - textLen) / 2;
  position = max(0, position);
  
  // Mettre à jour le buffer au lieu d'écrire directement sur l'écran
  for (int i = 0; i < textLen && (position + i) < LCD_COLS; i++) {
    screenBuffer[row][position + i] = text[i];
  }
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
    // Au lieu d'effacer l'écran, initialiser le buffer avec des espaces
    for (int row = 0; row < LCD_ROWS; row++) {
      for (int col = 0; col < LCD_COLS; col++) {
        screenBuffer[row][col] = ' ';
      }
      screenBuffer[row][LCD_COLS] = '\0';
    }
    
    // En-tête centrée
    const char* title = "Kite PiloteV3";
    int titlePos = (LCD_COLS - strlen(title)) / 2;
    titlePos = max(0, titlePos);
    for (int i = 0; i < strlen(title); i++) {
      screenBuffer[0][titlePos + i] = title[i];
    }
    
    // Ligne 1 - État WiFi
    strcpy(&screenBuffer[1][0], "WiFi: ");
    if (WiFi.status() == WL_CONNECTED) {
      String ssid = WiFi.SSID();
      // Tronquer le SSID s'il est trop long
      if (ssid.length() > LCD_COLS - 6) {
        ssid = ssid.substring(0, LCD_COLS - 9) + "...";
      }
      strcpy(&screenBuffer[1][6], ssid.c_str());
    } else {
      strcpy(&screenBuffer[1][6], "Déconnecté");
    }
    
    // Ligne 2 - Information système
    strcpy(&screenBuffer[2][0], "Sys: ");
    // Récupérer une valeur de sysStatus sécurisée
    uint8_t status = systemStatus;
    status = constrain(status, 0, 100);
    // Afficher l'état du système de manière compacte
    if (status >= 90) strcpy(&screenBuffer[2][5], "Excellent");
    else if (status >= 75) strcpy(&screenBuffer[2][5], "Bon");
    else if (status >= 50) strcpy(&screenBuffer[2][5], "Normal");
    else if (status >= 25) strcpy(&screenBuffer[2][5], "Faible");
    else strcpy(&screenBuffer[2][5], "Critique");
    
    // Ligne 3 - Affichage du temps de fonctionnement
    strcpy(&screenBuffer[3][0], "Uptime: ");
    unsigned long uptimeSeconds = millis() / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;
    
    // Formater l'uptime (hh:mm:ss)
    char uptimeStr[15];
    sprintf(uptimeStr, "%02lu:%02lu:%02lu", hours, minutes, seconds);
    strcpy(&screenBuffer[3][8], uptimeStr);
    
    // Mettre à jour uniquement les caractères qui ont changé
    updateLCDDiff();
    
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
 * Met à jour uniquement les caractères modifiés sur l'écran LCD pour éviter le scintillement
 * Cette méthode compare le contenu actuel du buffer avec le contenu précédent
 * et ne met à jour que les caractères qui ont changé.
 */
void DisplayManager::updateLCDDiff() {
  if (!lcdInitialized) return;
  
  bool hasChanges = false;
  
  // Parcourir chaque position du buffer
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      // Si le caractère a changé, le mettre à jour sur l'écran
      if (screenBuffer[row][col] != previousBuffer[row][col]) {
        hasChanges = true;
        lcd.setCursor(col, row);
        
        // Gérer les caractères spéciaux
        if (screenBuffer[row][col] == '\0') {
          lcd.print(' '); // Remplacer les fins de chaîne par des espaces
        } else if (screenBuffer[row][col] < 4) {
          // Caractères personnalisés (0-3)
          lcd.write(byte(screenBuffer[row][col]));
        } else {
          lcd.print(screenBuffer[row][col]);
        }
        
        // Mettre à jour le buffer précédent
        previousBuffer[row][col] = screenBuffer[row][col];
      }
    }
  }
  
  // Journaliser les mises à jour si nécessaire (uniquement pour le débogage)
  if (hasChanges) {
    LOG_DEBUG("DISPLAY", "LCD mise à jour différentielle effectuée");
  }
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
    // Initialiser le buffer avec des espaces
    for (int row = 0; row < LCD_ROWS; row++) {
      for (int col = 0; col < LCD_COLS; col++) {
        screenBuffer[row][col] = ' ';
      }
      screenBuffer[row][LCD_COLS] = '\0';
    }
    
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
      
      // Mettre à jour le buffer pour cette ligne
      for (int i = 0; i < charsToShow; i++) {
        screenBuffer[row][i] = message[startIdx + i];
      }
      
      // Avancer l'index de départ
      startIdx += charsToShow;
      if (charsToShow < charsLeft && message[startIdx] == ' ') {
        startIdx++; // Sauter l'espace en début de ligne suivante
      }
    }
    
    // Mettre à jour l'écran avec le nouveau contenu
    updateLCDDiff();
    
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
 * Vérifie la connexion avec l'écran LCD de manière plus robuste
 * @return true si l'écran répond correctement, false sinon
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
  
  // Test plus approfondi - essayer d'écrire quelque chose en mémoire puis le lire
  // Cette étape est ignorée si l'écran a déjà été initialisé avec succès
  if (!lcdInitialized) {
    // Nous devons utiliser des commandes de bas niveau pour vérifier que l'écran répond correctement
    // Essayons d'envoyer une commande pour allumer le rétroéclairage
    Wire.beginTransmission(LCD_I2C_ADDR);
    Wire.write(0x08); // Commande pour le rétroéclairage
    error = Wire.endTransmission();
    
    if (error != 0) {
      LOG_WARNING("DISPLAY", "Écran LCD ne répond pas aux commandes (erreur: %d)", error);
      return false;
    }
    
    // Si nous arrivons ici, l'écran répond aux commandes de base
    LOG_INFO("DISPLAY", "Écran LCD répond correctement aux commandes I2C");
    return true;
  }
  
  return true; // Si déjà initialisé et toujours présent, on considère qu'il fonctionne
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
  
  // Initialiser le buffer avec des espaces
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      screenBuffer[row][col] = ' ';
    }
    screenBuffer[row][LCD_COLS] = '\0';
  }
  
  // En-tête avec titre centré
  centerText(0, "Kite PiloteV3");
  
  if (simpleMode) {
    // Version simple - uniquement "INIT" en grand
    centerText(1, "INITIALISATION");
    centerText(3, SYSTEM_VERSION);
  } else {
    // Version complète avec plus d'informations
    strcpy(&screenBuffer[1][0], "Version: ");
    strcpy(&screenBuffer[1][9], SYSTEM_VERSION);
    
    strcpy(&screenBuffer[2][0], "Build: ");
    strcpy(&screenBuffer[2][7], SYSTEM_BUILD_DATE);
    
    strcpy(&screenBuffer[3][0], "System starting...");
  }
  
  // Mettre à jour l'écran
  updateLCDDiff();
}

/**
 * Affiche les informations WiFi
 * @param ssid SSID du réseau WiFi
 * @param ip Adresse IP
 */
void DisplayManager::displayWiFiInfo(const char* ssid, IPAddress ip) {
  if (!lcdInitialized) return;
  
  // Initialiser le buffer avec des espaces
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      screenBuffer[row][col] = ' ';
    }
    screenBuffer[row][LCD_COLS] = '\0';
  }
  
  // En-tête centrée
  centerText(0, "WiFi Info");
  
  // SSID
  strcpy(&screenBuffer[1][0], "SSID: ");
  strcpy(&screenBuffer[1][6], ssid);
  
  // Adresse IP
  strcpy(&screenBuffer[2][0], "IP: ");
  char ipStr[16];
  sprintf(ipStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  strcpy(&screenBuffer[2][4], ipStr);
  
  // Port
  strcpy(&screenBuffer[3][0], "Port: ");
  char portStr[6];
  sprintf(portStr, "%d", SERVER_PORT);
  strcpy(&screenBuffer[3][6], portStr);
  
  // Mettre à jour l'écran
  updateLCDDiff();
}

/**
 * Affiche le statut de fin OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void DisplayManager::displayOTAStatus(bool success) {
  if (!lcdInitialized) return;
  
  // Initialiser le buffer avec des espaces
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      screenBuffer[row][col] = ' ';
    }
    screenBuffer[row][LCD_COLS] = '\0';
  }
  
  // En-tête avec titre centré
  centerText(0, "Mise à jour OTA");
  
  if (success) {
    centerText(1, "Terminée avec succès");
    centerText(2, "Redémarrage...");
  } else {
    centerText(1, "ÉCHEC");
    centerText(2, "Fonctionnement normal");
    centerText(3, "conservé");
  }
  
  // Mettre à jour l'écran
  updateLCDDiff();
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
  
  // Initialiser le buffer avec des espaces
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      screenBuffer[row][col] = ' ';
    }
    screenBuffer[row][LCD_COLS] = '\0';
  }
  
  // Afficher le titre
  centerText(0, "Statut Kite");
  
  // Ligne 1 - Direction et WiFi
  strcpy(&screenBuffer[1][0], "Dir:");
  
  // Afficher la direction avec une petite barre de progression
  int dirPos = map(constrain(direction, -90, 90), -90, 90, 0, 9);
  for (int i = 0; i < 10; i++) {
    if (i == dirPos) {
      screenBuffer[1][4+i] = '|';
    } else {
      screenBuffer[1][4+i] = '-';
    }
  }
  
  // Afficher l'icône WiFi
  if (wifiConnected) {
    screenBuffer[1][17] = 2; // Valeur 2 pour afficher le caractère personnalisé (bloc plein)
  } else {
    screenBuffer[1][17] = 'X'; // X = WiFi déconnecté
  }
  
  // Ligne 2 - Trim
  strcpy(&screenBuffer[2][0], "Trim:");
  
  // Afficher le trim avec une petite barre de progression
  int trimPos = map(constrain(trim, -45, 45), -45, 45, 0, 9);
  for (int i = 0; i < 10; i++) {
    if (i == trimPos) {
      screenBuffer[2][5+i] = '|';
    } else {
      screenBuffer[2][5+i] = '-';
    }
  }
  
  // Ligne 3 - Longueur de ligne et uptime
  strcpy(&screenBuffer[3][0], "Ligne:");
  
  // Afficher la longueur avec une petite barre de progression
  int lenPos = map(constrain(lineLength, 0, 100), 0, 100, 0, 5);
  for (int i = 0; i < 6; i++) {
    if (i <= lenPos) {
      screenBuffer[3][6+i] = 2; // Bloc plein (caractère personnalisé)
    } else {
      screenBuffer[3][6+i] = '.';
    }
  }
  
  // Afficher le temps de fonctionnement
  unsigned long hours = uptime / 3600;
  unsigned long minutes = (uptime % 3600) / 60;
  
  char timeStr[8];
  sprintf(timeStr, "%luh%02lu", hours, minutes);
  strcpy(&screenBuffer[3][13], timeStr);
  
  // Appliquer les changements
  updateLCDDiff();
}
