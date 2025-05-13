/*
  -----------------------
  Kite PiloteV3 - Module UI Manager (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire unifié de l'interface utilisateur.
  
  Version: 2.0.0
  Date: 5 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "hardware/io/ui_manager.h"
#include "utils/logging.h" 
#include "core/module.h"
#include "core/system.h" // <<< AJOUTÉ
#include <WiFi.h>
#include <vector>
#include <string>

// Constructeur
UIManager::UIManager(DisplayManager* globalDisplay) // Modifié pour prendre un DisplayManager global
    : associatedDisplay(globalDisplay), // Stocke le pointeur vers le DisplayManager global
      lcdInitialized(false), // Sera mis à true dans begin() si l'écran associé est prêt
      displayNeedsUpdate(true),
      currentDisplayState(DISPLAY_MAIN),
      currentMenu(MENU_MAIN),
      currentMenuSelection(0),
      lastDisplayUpdate(0),
      lastDisplayCheck(0),
      lastButtonCheck(0) {
    
    // Initialisation des états des boutons
    for (int i = 0; i < 4; i++) {
        buttonStates[i] = false;
        lastButtonStates[i] = false;
        lastDebounceTime[i] = 0;
    }
}

UIManager::~UIManager() {
    // Nettoyage si nécessaire
}

/**
 * Initialise l'UIManager
 * @return true si l'initialisation réussit, false sinon
 */
bool UIManager::begin() {
    if (!associatedDisplay) {
        LOG_ERROR("UI_MGR", "DisplayManager associé non fourni (null). Abandon de l'initialisation de UIManager.");
        this->lcdInitialized = false;
        return false;
    }

    LOG_INFO("UI_MGR", "UIManager::begin() utilise le DisplayManager externe.");

    unsigned long uiMgrStartTime = millis();
    const unsigned long uiMgrTimeout = 3000; // Court timeout pour que UIManager attende l'écran si besoin.

    while (!associatedDisplay->isSuccessfullyInitialized()) {
        associatedDisplay->update(); // Donne une chance à la FSM de l'écran de progresser
        if (millis() - uiMgrStartTime > uiMgrTimeout) {
            LOG_ERROR("UI_MGR", "Timeout en attendant que le DisplayManager associé soit prêt.");
            this->lcdInitialized = false; // Marquer UIManager comme non initialisé si l'écran ne l'est pas
            return false;
        }
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            vTaskDelay(pdMS_TO_TICKS(20)); 
        } else {
            delay(20); 
        }
    }
    
    this->lcdInitialized = associatedDisplay->isSuccessfullyInitialized();

    if (this->lcdInitialized) {
        LOG_INFO("UI_MGR", "DisplayManager associé est prêt. UIManager peut continuer son initialisation.");
    } else {
        LOG_ERROR("UI_MGR", "DisplayManager associé n'est PAS prêt. UIManager ne peut pas s'initialiser correctement.");
        return false; // Ne pas continuer si l'écran n'est pas prêt
    }
    
    // Configurer les entrées pour les boutons (utiliser INPUT_PULLUP pour éviter les résistances externes)
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_BACK_PIN, INPUT_PULLUP);
    
    // Initialiser les variables d'état
    currentDisplayState = DISPLAY_MAIN;
    currentMenu = MENU_MAIN;
    currentMenuSelection = 0;
    lastButtonCheckTime = 0;
    lastDisplayUpdate = 0;
    displayNeedsUpdate = true;  // Forcer une mise à jour initiale
    
    // Initialiser le tableau des états des boutons
    for (int i = 0; i < MAX_BUTTONS; i++) {
        buttonStates[i] = HIGH;  // HIGH = non pressé avec INPUT_PULLUP
        lastButtonStates[i] = HIGH;
        lastDebounceTime[i] = 0;
    }
    
    if (this->lcdInitialized) {
        // Afficher un écran de bienvenue initial via le DisplayManager associé
        this->associatedDisplay->displayWelcomeScreen(true);
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            delay(1000); // Fallback
        }
        updateDisplay();  // Appeler UIManager::updateDisplay() pour afficher l'écran principal
        LOG_INFO("UI", "Interface utilisateur initialisée avec succès (basée sur le DisplayManager associé).");
    } else {
        LOG_ERROR("UI", "Échec d'initialisation de l'interface utilisateur - DisplayManager associé non fonctionnel.");
    }
    
    return this->lcdInitialized;
}

void UIManager::clear() {
    if (!this->lcdInitialized || !this->associatedDisplay) return;
    this->associatedDisplay->clear(); 
}

void UIManager::centerText(uint8_t row, const char* text) {
    if (!this->lcdInitialized || !this->associatedDisplay || row >= LCD_ROWS) return;
    this->associatedDisplay->centerText(row, text); 
    this->associatedDisplay->updateLCDDiff(); 
}

void UIManager::updateDisplay() {
    if (!this->associatedDisplay || !this->associatedDisplay->isSuccessfullyInitialized()) {
        return;
    }
    
    unsigned long currentTime = millis();
    if (!displayNeedsUpdate && (currentTime - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL)) {
        return;
    }
    
    lastDisplayUpdate = currentTime;
    displayNeedsUpdate = false;
    
    switch (currentDisplayState) {
        case DISPLAY_MAIN:
            updateMainDisplay();
            break;
        case DISPLAY_DIRECTION_TRIM:
            updateDirectionTrimDisplay(0, 0); // Valeurs à mettre à jour
            break;
        case DISPLAY_LINE_LENGTH:
            updateLineLengthDisplay(50); // Valeur à mettre à jour
            break;
        case DISPLAY_WIFI_INFO:
            displayWiFiInfo(WiFi.SSID(), WiFi.localIP());
            break;
        case DISPLAY_SYSTEM_STATS:
            displaySystemStats();
            break;
        default:
            currentDisplayState = DISPLAY_MAIN;
            updateMainDisplay();
            break;
    }
}

void UIManager::displayMessage(const char* title, const char* message) {
    if (!this->lcdInitialized || !this->associatedDisplay) return;
    this->associatedDisplay->displayMessage(title, message, 0); // Utilise le DisplayManager associé
    this->associatedDisplay->updateLCDDiff(); // S'assurer que le message est affiché
}

void UIManager::displayWiFiInfo(const String& ssid, const IPAddress& ip) {
    if (!this->lcdInitialized || !this->associatedDisplay) return;
    this->associatedDisplay->displayWiFiInfo(ssid.c_str(), ip); // Utilise le DisplayManager associé
    this->associatedDisplay->updateLCDDiff();
}

void UIManager::displaySystemStats() {
    if (!this->lcdInitialized || !this->associatedDisplay) return;
    this->associatedDisplay->clear(); // Effacer l'écran via le DisplayManager associé
    
    char buffer[LCD_COLS + 1];
    this->associatedDisplay->centerText(0, "System Stats");

    // Utiliser SystemInfo.uptimeSeconds
    SystemInfo currentSystemInfo = getSystemInfo();
    unsigned long uptime_s = currentSystemInfo.uptimeSeconds;
    unsigned long hours = uptime_s / 3600;
    unsigned long minutes = (uptime_s % 3600) / 60;
    unsigned long seconds = uptime_s % 60; // Afficher aussi les secondes

    snprintf(buffer, sizeof(buffer), "Up: %02lu:%02lu:%02lu", hours, minutes, seconds);
    this->associatedDisplay->centerText(1, buffer);

    // Exemple: Afficher la mémoire libre (à adapter)
    // snprintf(buffer, sizeof(buffer), "Free Mem: %u", ESP.getFreeHeap());
    // this->associatedDisplay->centerText(2, buffer);
    
    this->associatedDisplay->updateLCDDiff(); // Mettre à jour l'écran physique
}

void UIManager::checkButtons() {
    unsigned long currentTime = millis();
    
    const unsigned long currentButtonCheckInterval = BUTTON_CHECK_INTERVAL; 
    const unsigned long currentButtonDebounceDelay = BUTTON_DEBOUNCE_DELAY; 

    if (currentTime - lastButtonCheck < currentButtonCheckInterval) {
        return;
    }
    lastButtonCheck = currentTime;
    
    uint8_t pins[MAX_BUTTONS] = {BUTTON_BACK_PIN, BUTTON_UP_PIN, BUTTON_SELECT_PIN, BUTTON_DOWN_PIN}; 
    
    for (int i = 0; i < MAX_BUTTONS; i++) {
        bool reading = !digitalRead(pins[i]); 
        
        if (reading != lastButtonStates[i]) {
            lastDebounceTime[i] = currentTime;
        }
        
        if ((currentTime - lastDebounceTime[i]) > currentButtonDebounceDelay) {
            if (reading != buttonStates[i]) { 
                buttonStates[i] = reading;
                if (buttonStates[i]) { 
                    LOG_DEBUG("UI_MGR", "Bouton %d pressé (Pin: %d)", i, pins[i]);
                    switch (i) { 
                        case 0: 
                            menuBack();
                            break;
                        case 1: 
                            menuUp();
                            break;
                        case 2: 
                            menuSelect();
                            break;
                        case 3: 
                            menuDown();
                            break;
                    }
                    displayNeedsUpdate = true; 
                }
            }
        }
        lastButtonStates[i] = reading; 
    }
}

void UIManager::menuUp() {
    LOG_DEBUG("UI_MGR", "menuUp() appelé");
    if (currentMenuSelection > 0) {
        currentMenuSelection--;
    }
    displayNeedsUpdate = true;
    showMenu(currentMenu); 
}

void UIManager::menuDown() {
    LOG_DEBUG("UI_MGR", "menuDown() appelé");
    uint8_t itemCount = 3; 
    if (currentMenu == MENU_MAIN) itemCount = 3; 

    if (currentMenuSelection < itemCount) { 
        currentMenuSelection++;
    }
    displayNeedsUpdate = true;
    showMenu(currentMenu); 
}

void UIManager::menuSelect() {
    LOG_DEBUG("UI_MGR", "menuSelect() appelé - Menu: %d, Sélection: %d", currentMenu, currentMenuSelection);
    displayNeedsUpdate = true;
}

void UIManager::menuBack() {
    LOG_DEBUG("UI_MGR", "menuBack() appelé");
    if (currentMenu != MENU_MAIN) { 
        showMenu(MENU_MAIN); 
    }
    displayNeedsUpdate = true;
}

void UIManager::updateMainDisplay() {
    if (this->associatedDisplay) {
        this->associatedDisplay->updateMainDisplay();
    }
}

void UIManager::updateDirectionTrimDisplay(int direction, int trim) {
    this->associatedDisplay->clear();
    this->associatedDisplay->centerText(0, "Contrôle Direction");
    
    this->associatedDisplay->getLcd().setCursor(0, 1);
    this->associatedDisplay->getLcd().print("Dir:");
    drawDirection(1, direction);
    this->associatedDisplay->getLcd().setCursor(16, 1);
    if (direction >= 0) this->associatedDisplay->getLcd().print(" ");
    this->associatedDisplay->getLcd().print(direction);
    
    this->associatedDisplay->getLcd().setCursor(0, 2);
    this->associatedDisplay->getLcd().print("Trim:");
    drawDirection(2, trim);
    this->associatedDisplay->getLcd().setCursor(16, 2);
    if (trim >= 0) this->associatedDisplay->getLcd().print(" ");
    this->associatedDisplay->getLcd().print(trim);
    
    this->associatedDisplay->getLcd().setCursor(0, 3);
    this->associatedDisplay->getLcd().print("POT1:Dir POT2:Trim");
}

void UIManager::updateLineLengthDisplay(int length) {
    this->associatedDisplay->clear();
    this->associatedDisplay->centerText(0, "Longueur Lignes");
        
    this->associatedDisplay->getLcd().setCursor(0, 1);
    this->associatedDisplay->getLcd().print("Longueur: ");
    this->associatedDisplay->getLcd().print(length);
    this->associatedDisplay->getLcd().print("%");
    
    drawProgressBar(2, length);
    
    this->associatedDisplay->getLcd().setCursor(0, 3);
    this->associatedDisplay->getLcd().print("POT3: Longueur");
}

void UIManager::drawProgressBar(uint8_t row, uint8_t percent) {
    if (!this->lcdInitialized || row >= LCD_ROWS) return;
    
    percent = constrain(percent, 0, 100);
    int numBlocks = (percent * LCD_COLS) / 100;
    
    this->associatedDisplay->getLcd().setCursor(0, row);
    
    for (int i = 0; i < numBlocks; i++) {
        this->associatedDisplay->getLcd().write(byte(2)); 
    }
    
    for (int i = numBlocks; i < LCD_COLS; i++) {
        this->associatedDisplay->getLcd().print(" ");
    }
}

void UIManager::drawDirection(uint8_t row, int value) {
    if (!this->lcdInitialized || row >= LCD_ROWS) return;
    
    value = constrain(value, -100, 100);
    int center = LCD_COLS / 2; 
    int mappedVal = map(value, -100, 100, -(center-1), (center-1) );
    int position = center + mappedVal;

    position = constrain(position, 0, LCD_COLS - 1); 
    
    this->associatedDisplay->getLcd().setCursor(0, row);
    
    for (int i = 0; i < LCD_COLS; i++) {
        if (i == position) {
            this->associatedDisplay->getLcd().write(byte(0)); 
        } else if (i == center) {
            this->associatedDisplay->getLcd().print("|");
        } else {
            this->associatedDisplay->getLcd().print(".");
        }
    }
}

void UIManager::showMenu(MenuState menu) {
    if (!this->lcdInitialized || !this->associatedDisplay) return; 
    currentMenu = menu;
    currentMenuSelection = 0;
    this->associatedDisplay->clear();

    switch (menu) {
        case MENU_MAIN:
            this->associatedDisplay->centerText(0, "Menu Principal");
            printMenuItem(1, "Contrôle", currentMenuSelection == 0);
            printMenuItem(2, "Paramètres", currentMenuSelection == 1);
            printMenuItem(3, "Système", currentMenuSelection == 2);
            break;
            
        case MENU_SETTINGS:
            this->associatedDisplay->centerText(0, "Paramètres");
            printMenuItem(1, "Calibration", currentMenuSelection == 0);
            printMenuItem(2, "WiFi", currentMenuSelection == 1);
            printMenuItem(3, "Retour", currentMenuSelection == 2);
            break;
            
        case MENU_CONTROL:
            this->associatedDisplay->centerText(0, "Contrôle");
            printMenuItem(1, "Mode Direction", currentMenuSelection == 0);
            printMenuItem(2, "Mode Longueur", currentMenuSelection == 1);
            printMenuItem(3, "Retour", currentMenuSelection == 2);
            break;
            
        case MENU_WIFI:
            this->associatedDisplay->centerText(0, "WiFi");
            printMenuItem(1, "Connexion", currentMenuSelection == 0);
            printMenuItem(2, "Info", currentMenuSelection == 1);
            printMenuItem(3, "Retour", currentMenuSelection == 2);
            break;
            
        case MENU_SYSTEM:
            this->associatedDisplay->centerText(0, "Système");
            printMenuItem(1, "Info", currentMenuSelection == 0);
            printMenuItem(2, "Mise à jour OTA", currentMenuSelection == 1);
            printMenuItem(3, "Retour", currentMenuSelection == 2);
            break;

        case MENU_MODULES: {
            this->associatedDisplay->centerText(0, "Modules actifs");
            int row = 1;
            int idx = 0;
            for (Module* m : ModuleRegistry::instance().modules()) {
                if (row >= LCD_ROWS) break; 
                char line[LCD_COLS +1]; 
                snprintf(line, sizeof(line), "%c%s [%s]", (currentMenuSelection == idx ? '>' : ' '), m->name(), m->isEnabled() ? "ON" : "OFF");
                printMenuItem(row, line, currentMenuSelection == idx); 
                row++;
                idx++;
            }
            for (; row < LCD_ROWS; ++row) {
                 this->associatedDisplay->getLcd().setCursor(0, row);
                 for(int i=0; i<LCD_COLS; ++i) this->associatedDisplay->getLcd().print(" ");
            }
            break;
        }
    }
    this->associatedDisplay->updateLCDDiff(); 
}

void UIManager::printMenuItem(uint8_t row, const char* text, bool selected) {
    if (!this->lcdInitialized || row >= LCD_ROWS) return;

    char buffer[LCD_COLS + 1];
    snprintf(buffer, sizeof(buffer), "%c %s", (selected ? '>' : ' '), text);
    
    for (size_t i = strlen(buffer); i < LCD_COLS; ++i) {
        buffer[i] = ' ';
    }
    buffer[LCD_COLS] = '\0';

    this->associatedDisplay->getLcd().setCursor(0, row);
    this->associatedDisplay->getLcd().print(buffer);
}