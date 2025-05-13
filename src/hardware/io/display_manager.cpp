/*
  -----------------------
  Kite PiloteV3 - Module de Gestion de l'Affichage (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire d'affichage LCD pour l'interface utilisateur locale.
  
  Version: 3.0.2
  Date: 11 mai 2025
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
*/

#include "hardware/io/display_manager.h" 
#include "utils/logging.h"             
#include "core/config.h"               
#include "core/system.h" // Ajout de l'include pour SystemInfo et getSystemInfo
#include <Arduino.h>                   
#include <Wire.h>                      
#include <WiFi.h> // Ajout pour WiFi.status() et WiFi.SSID()
#include <IPAddress.h> // Ajout pour IPAddress type

#ifndef DISPLAY_UPDATE_THROTTLE
#define DISPLAY_UPDATE_THROTTLE 100 
#endif

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
    if (displayInitFsm) {
        delete displayInitFsm;
        displayInitFsm = nullptr;
    }
    LOG_INFO("DISPLAY", "Gestionnaire d'affichage détruit");
}

// Caractères personnalisés
const uint8_t charUp[] = {0b00000,0b00100,0b01110,0b11111,0b00000,0b00000,0b00000,0b00000};
const uint8_t charDown[] = {0b00000,0b00000,0b00000,0b00000,0b11111,0b01110,0b00100,0b00000};
const uint8_t charBlock[] = {0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111};

/**
 * Configure le bus I2C pour la communication avec le LCD
 * @return true si la configuration réussit, false sinon
 */
bool DisplayManager::setupI2C() {
  Wire.setClock(400000); 
  
  Wire.beginTransmission(LCD_I2C_ADDR);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    LOG_INFO("DISPLAY", "I2C configuré (ou déjà actif) pour LCD à 0x%X", LCD_I2C_ADDR);
    i2cInitialized = true;
    return true;
  } else {
    LOG_ERROR("DISPLAY", "Erreur I2C lors de la communication avec LCD 0x%X (code: %d)", LCD_I2C_ADDR, error);
    i2cInitialized = false;
    return false;
  }
}

/**
 * Fait progresser la FSM d'initialisation de l'écran.
 * Doit être appelée régulièrement jusqu'à ce que isSuccessfullyInitialized() soit vrai.
 */
void DisplayManager::update() {
    if (displayInitFsm && !isSuccessfullyInitialized()) { 
        displayInitFsm->update();
    }
}

// Implémentation du constructeur de la FSM
DisplayInitFSM::DisplayInitFSM(DisplayManager* mgr) : 
    StateMachine("DisplayInitFSM", INIT_START, 5000, -1), 
    manager(mgr), 
    lastActionTime(0),
    retryCount(0)
{}

// Add implementations for onEnterState and onExitState
void DisplayInitFSM::onEnterState(int newState, int oldState) {
    StateMachine::onEnterState(newState, oldState); 
    LOG_INFO("DisplayInitFSM_Trace", "Entering state %d from %d. FSM stateEntryTime: %lu. Current millis: %lu", newState, oldState, this->stateEntryTime, millis());
}

void DisplayInitFSM::onExitState(int oldState, int newState) {
    StateMachine::onExitState(oldState, newState); 
    unsigned long timeSpent = millis() - this->stateEntryTime; 
    LOG_INFO("DisplayInitFSM_Trace", "Exiting state %d to %d. Time spent in state %d was: %lu ms. Current millis: %lu", oldState, newState, oldState, timeSpent, millis());
}

void DisplayInitFSM::reset() {
    LOG_INFO("DISPLAY_FSM", "Réinitialisation FSM écran -> INIT_START.");
    int oldStateForLog = currentState; // Sauvegarde pour le log

    currentState = INIT_START; 
    stateEntryTime = millis();    // Réinitialiser le temps d'entrée pour le nouvel état INIT_START
    lastActionTime = millis();    // Réinitialiser aussi lastActionTime
    retryCount = 0;
    // manager->setLcdHardwareInitialized(false); // Sera géré par l'état INIT_START lors du prochain processState

    // Log informatif sur le changement d'état forcé
    LOG_INFO("DISPLAY_FSM", "FSM forcée à l'état %d depuis l'état %d. stateEntryTime mis à %lu.", currentState, oldStateForLog, stateEntryTime);
}

int DisplayInitFSM::processState(int state) {
    unsigned long now = millis();
    switch (state) {
        case INIT_START:
            manager->setLcdHardwareInitialized(false); 
            retryCount = 0;
            lastActionTime = now;
            LOG_INFO("DISPLAY_FSM", "Démarrage FSM init écran.");
            return I2C_CONFIG;

        case I2C_CONFIG:
            if (!manager->setupI2C()) {
                retryCount++;
                if (retryCount >= maxRetries) {
                    LOG_ERROR("DISPLAY_FSM", "Echec config I2C après %d essais. Abandon FSM.", maxRetries);
                    manager->setLcdHardwareInitialized(false);
                    return DONE; 
                }
                LOG_WARNING("DISPLAY_FSM", "Essai config I2C %d échoué. Nouvel essai...", retryCount);
                lastActionTime = now; 
                return I2C_CONFIG; 
            }
            LOG_INFO("DISPLAY_FSM", "I2C configuré.");
            lastActionTime = now;
            retryCount = 0; 
            return LCD_INIT;

        case LCD_INIT:
            if (now - lastActionTime < 100) return LCD_INIT; 
            LOG_INFO("DISPLAY_FSM", "Init objet LCD...");
            manager->getLcd().init(); 
            manager->setLcdHardwareInitialized(true); 
            LOG_INFO("DISPLAY_FSM", "Objet LCD initialisé.");
            lastActionTime = now;
            return BACKLIGHT_ON;

        case BACKLIGHT_ON:
            if (now - lastActionTime < 50) return BACKLIGHT_ON;
            LOG_INFO("DISPLAY_FSM", "Allumage rétroéclairage LCD.");
            manager->getLcd().backlight();
            lastActionTime = now;
            return CLEAR_DISPLAY;

        case CLEAR_DISPLAY:
            if (now - lastActionTime < 50) return CLEAR_DISPLAY;
            LOG_INFO("DISPLAY_FSM", "Effacement LCD.");
            manager->getLcd().clear();
            LOG_INFO("DISPLAY_FSM", "Création caractères perso.");
            manager->createCustomChars();
            lastActionTime = now;
            return DONE;

        case DONE:
            if (manager->isLcdHardwareInternallyInitialized()) { 
                 LOG_INFO("DISPLAY_FSM", "FSM écran terminée. LCD prêt (matériellement).");
            } else {
                 LOG_ERROR("DISPLAY_FSM", "FSM écran terminée, mais init matériel LCD échouée.");
            }
            return DONE; 

        default:
            LOG_ERROR("DISPLAY_FSM", "Etat FSM inconnu: %d. Passage à DONE.", state);
            manager->setLcdHardwareInitialized(false);
            return DONE; 
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
  if (!isSuccessfullyInitialized()) { 
    LOG_DEBUG("DISPLAY", "Tentative d'effacement mais LCD non prêt.");
    return;
  }
  for (int row = 0; row < LCD_ROWS; row++) {
    for (int col = 0; col < LCD_COLS; col++) {
      screenBuffer[row][col] = ' ';
    }
    screenBuffer[row][LCD_COLS] = '\0';
  }
  updateLCDDiff();
}

/**
 * Centre le texte sur une ligne du LCD
 * @param row Numéro de ligne (0-3)
 * @param text Texte à centrer
 */
void DisplayManager::centerText(uint8_t row, const char* text) {
  if (!isSuccessfullyInitialized() || row >= LCD_ROWS) {
    LOG_DEBUG("DISPLAY", "Tentative centerText mais LCD non prêt ou ligne invalide.");
    return;
  }
  size_t textLen = strlen(text);
  int position = (LCD_COLS - textLen) / 2;
  position = max(0, (int)position); // Cast en int pour max si textLen est size_t
  
  for (int i = 0; i < LCD_COLS; ++i) { 
      screenBuffer[row][i] = ' ';
  }
  for (size_t i = 0; i < textLen && (position + i) < LCD_COLS; i++) {
    screenBuffer[row][position + i] = text[i];
  }
  screenBuffer[row][LCD_COLS] = '\0'; 
}

/**
 * Met à jour l'écran principal avec optimisation de la fréquence
 * de rafraîchissement et protection contre les mises à jour multiples
 */
void DisplayManager::updateMainDisplay() {
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime < DISPLAY_UPDATE_THROTTLE) {
    return;
  }
  lastUpdateTime = currentTime;
  
  if (!isSuccessfullyInitialized()) {
    LOG_DEBUG("DISPLAY", "Tentative updateMainDisplay mais LCD non prêt.");
    return;
  }
  
  clear(); 
  centerText(0, "Kite PiloteV3");

  char temp[LCD_COLS + 1];
  if (WiFi.status() == WL_CONNECTED) {
    snprintf(temp, LCD_COLS + 1, "WiFi: %s", WiFi.SSID().c_str());
  } else {
    snprintf(temp, LCD_COLS + 1, "WiFi: Déconnecté");
  }
  strncpy(screenBuffer[1], temp, LCD_COLS);
  screenBuffer[1][LCD_COLS] = '\0';

  centerText(2, "System OK"); // Placeholder

  // Utiliser SystemInfo.uptimeSeconds
  SystemInfo currentSystemInfo = getSystemInfo();
  unsigned long uptimeSeconds = currentSystemInfo.uptimeSeconds;
  unsigned long hours = uptimeSeconds / 3600;
  unsigned long minutes = (uptimeSeconds % 3600) / 60;
  unsigned long seconds = uptimeSeconds % 60;
  snprintf(temp, LCD_COLS + 1, "Up: %02lu:%02lu:%02lu", hours, minutes, seconds);
  strncpy(screenBuffer[3], temp, LCD_COLS);
  screenBuffer[3][LCD_COLS] = '\0';
  
  updateLCDDiff();
  successfulUpdates++;
}

/**
 * Met à jour uniquement les caractères modifiés sur l'écran LCD pour éviter le scintillement
 * Cette méthode compare le contenu actuel du buffer avec le contenu précédent
 * et ne met à jour que les caractères qui ont changé.
 */
void DisplayManager::updateLCDDiff() {
  if (!lcdInitialized) { 
      LOG_DEBUG("DISPLAY", "Tentative updateLCDDiff mais LCD matériel non initialisé.");
      return;
  }
    bool hasChanges = false;
    for (int row = 0; row < LCD_ROWS; row++) {
        for (int col = 0; col < LCD_COLS; col++) {
            if (screenBuffer[row][col] != previousBuffer[row][col]) {
                hasChanges = true;
                lcd.setCursor(col, row);
                if (screenBuffer[row][col] == '\0') { 
                    lcd.print(' '); 
                } else if (screenBuffer[row][col] < 4 && screenBuffer[row][col] >= 0) { 
                    lcd.write(byte(screenBuffer[row][col]));
                } else {
                    lcd.print(screenBuffer[row][col]);
                }
                previousBuffer[row][col] = screenBuffer[row][col];
            }
        }
    }
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
  if (!isSuccessfullyInitialized()) {
    LOG_WARNING("DISPLAY", "Écran non prêt, message '%s' non affiché.", title);
    return;
  }
  clear(); 
  centerText(0, title);

  size_t msgLen = strlen(message);
  int bufferRow = 1;
  size_t currentMsgChar = 0;
  while(bufferRow < LCD_ROWS && currentMsgChar < msgLen) {
      size_t lenToCopy = min((size_t)LCD_COLS, msgLen - currentMsgChar);
      strncpy(screenBuffer[bufferRow], message + currentMsgChar, lenToCopy);
      screenBuffer[bufferRow][lenToCopy] = '\0'; 
      currentMsgChar += lenToCopy;
      bufferRow++;
  }
  updateLCDDiff();
  lastUpdateTime = millis();
}

/**
 * Affiche l'écran de bienvenue
 * @param simpleMode Mode simple (true) ou complet (false)
 */
void DisplayManager::displayWelcomeScreen(bool simpleMode) {
  if (!isSuccessfullyInitialized()) { 
      LOG_WARNING("DISPLAY", "Welcome screen non affiché: LCD non prêt.");
      return;
  }
  clear();
  centerText(0, "Kite PiloteV3");
  if (!simpleMode) {
    centerText(1, "Version 3.0.2"); 
    for (int i = 0; i < LCD_COLS; i++) { 
      screenBuffer[2][i] = char(2); 
      updateLCDDiff(); 
      delay(25); 
    }
    screenBuffer[2][LCD_COLS] = '\0'; // Assurer la terminaison après la boucle
    centerText(3, "Initialisation...");
  } else {
    centerText(2, "Démarrage rapide");
  }
  updateLCDDiff();
}

/**
 * Affiche les informations WiFi
 * @param ssid SSID du réseau WiFi
 * @param ip Adresse IP
 */
void DisplayManager::displayWiFiInfo(const char* ssid, IPAddress ip) {
    if (!isSuccessfullyInitialized()) return;
    clear();
    centerText(0, "Infos WiFi");
    char temp[LCD_COLS + 1];
    snprintf(temp, LCD_COLS + 1, "SSID: %s", ssid ? ssid : "N/A");
    strncpy(screenBuffer[1], temp, LCD_COLS);
    screenBuffer[1][LCD_COLS] = '\0';

    snprintf(temp, LCD_COLS + 1, "IP: %s", ip.toString().c_str());
    strncpy(screenBuffer[2], temp, LCD_COLS);
    screenBuffer[2][LCD_COLS] = '\0';
    updateLCDDiff();
}

/**
 * Affiche la progression OTA
 * @param current Taille actuelle téléchargée
 * @param total Taille totale
 */
void DisplayManager::displayOTAProgress(size_t current, size_t total) {
    if (!isSuccessfullyInitialized()) return;
    clear();
    centerText(0, "Mise à Jour OTA");
    char temp[LCD_COLS + 1];
    if (total > 0) {
        int progress = (current * 100) / total;
        snprintf(temp, LCD_COLS + 1, "Prog: %d%%", progress);
        strncpy(screenBuffer[1], temp, LCD_COLS);
        screenBuffer[1][LCD_COLS] = '\0';
        
        int barWidth = map(progress, 0, 100, 0, LCD_COLS);
        for(int i=0; i<LCD_COLS; ++i) {
            screenBuffer[2][i] = (i < barWidth) ? char(2) : ' ';
        }
        screenBuffer[2][LCD_COLS] = '\0';
    } else {
        strncpy(screenBuffer[1], "Préparation...", LCD_COLS);
        screenBuffer[1][LCD_COLS] = '\0';
    }
    updateLCDDiff();
}

/**
 * Affiche le statut de fin OTA
 * @param success true si la mise à jour a réussi, false sinon
 */
void DisplayManager::displayOTAStatus(bool success) {
    if (!isSuccessfullyInitialized()) return;
    clear();
    centerText(0, "Mise à Jour OTA");
    if(success) {
        centerText(1, "Réussie!");
        centerText(2, "Redémarrage...");
    } else {
        centerText(1, "Échouée!");
    }
    updateLCDDiff();
}

/**
 * Affiche les informations de statut en direct
 * @param direction Angle de direction (-90 à +90)
 * @param trim Angle de trim (-45 à +45)
 * @param lineLength Longueur de ligne (0 à 100)
 * @param wifiConnected État de la connexion WiFi
 */
void DisplayManager::displayLiveStatus(int direction, int trim, int lineLength, bool wifiConnected) { // Suppression du paramètre uptime
    if (!isSuccessfullyInitialized()) return;
    clear();
    char temp[LCD_COLS+1];
    snprintf(temp, LCD_COLS + 1, "Dir:%3d Trim:%3d", direction, trim);
    strncpy(screenBuffer[0], temp, LCD_COLS);
    screenBuffer[0][LCD_COLS] = '\0';

    snprintf(temp, LCD_COLS + 1, "Ligne:%3d WiFi:%s", lineLength, wifiConnected ? "OK" : "!!");
    strncpy(screenBuffer[1], temp, LCD_COLS);
    screenBuffer[1][LCD_COLS] = '\0';
    
    // Utiliser SystemInfo.uptimeSeconds
    SystemInfo currentSystemInfo = getSystemInfo();
    unsigned long currentUptimeSeconds = currentSystemInfo.uptimeSeconds;
    unsigned long hours = currentUptimeSeconds / 3600;
    unsigned long minutes = (currentUptimeSeconds % 3600) / 60;
    // Affichage des secondes également pour plus de précision
    unsigned long seconds = currentUptimeSeconds % 60;
    snprintf(temp, LCD_COLS + 1, "Up: %02lu:%02lu:%02lu", hours, minutes, seconds);
    strncpy(screenBuffer[3], temp, LCD_COLS); // Affichage sur la 4ème ligne (index 3)
    screenBuffer[3][LCD_COLS] = '\0';

    updateLCDDiff();
}

/**
 * Vérifie la connexion avec l'écran LCD de manière plus robuste
 * @return true si l'écran répond correctement, false sinon
 */
bool DisplayManager::checkLCDConnection() {
    if (!i2cInitialized) { 
        if (!setupI2C()) {
            LOG_WARNING("DISPLAY", "Echec setup I2C pendant checkLCDConnection.");
            return false;
        }
    }
    Wire.beginTransmission(LCD_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
        return true;
    }
    LOG_WARNING("DISPLAY", "checkLCDConnection: LCD ne répond pas à 0x%X", LCD_I2C_ADDR);
    return false;
}

/**
 * Vérifie l'état du LCD et tente une récupération si nécessaire.
 * @return true si l'écran est fonctionnel, false sinon.
 */
bool DisplayManager::checkAndRecover() {
    if (isSuccessfullyInitialized() && checkLCDConnection()) {
        successfulUpdates++; 
        return true;
    }
    LOG_WARNING("DISPLAY", "Problème connexion LCD détecté, tentative de récupération...");
    return recoverLCD();
}

/**
 * Tente de récupérer la connexion avec le LCD en réinitialisant.
 * @return true si la récupération réussit, false sinon.
 */
bool DisplayManager::recoverLCD() {
    LOG_INFO("DISPLAY", "Tentative de récupération du LCD...");
    setLcdHardwareInitialized(false); 
    i2cInitialized = false;
    if (displayInitFsm) {
        displayInitFsm->reset(); 
    }
    
    unsigned long recoveryStartTime = millis();
    while (!isSuccessfullyInitialized() && (millis() - recoveryStartTime < 5000)) {
        update(); 
        delay(20); 
    }

    if (isSuccessfullyInitialized()) {
        LOG_INFO("DISPLAY", "Récupération du LCD réussie.");
        return true;
    } else {
        LOG_ERROR("DISPLAY", "Échec de la récupération du LCD.");
        return false;
    }
}
