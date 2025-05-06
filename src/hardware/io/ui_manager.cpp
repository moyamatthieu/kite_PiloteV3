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
#include <WiFi.h>

// Caractères personnalisés pour l'affichage
const uint8_t charDirection[] = {
    0b00100, 0b01110, 0b11111, 0b00100,
    0b00100, 0b00100, 0b00100, 0b00000
};

const uint8_t charLeft[] = {
    0b00010, 0b00100, 0b01000, 0b10000,
    0b01000, 0b00100, 0b00010, 0b00000
};

const uint8_t charRight[] = {
    0b01000, 0b00100, 0b00010, 0b00001,
    0b00010, 0b00100, 0b01000, 0b00000
};

const uint8_t charBlock[] = {
    0b11111, 0b11111, 0b11111, 0b11111,
    0b11111, 0b11111, 0b11111, 0b11111
};

// Constructeur
UIManager::UIManager()
    : lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS),
      lcdInitialized(false),
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

bool UIManager::begin() {
    // Configuration I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(50);
    
    LOG_INFO("UI", "Initialisation de l'interface LCD...");
    
    // Initialisation LCD avec plusieurs tentatives
    for (int attempt = 1; attempt <= MAX_INIT_ATTEMPTS; attempt++) {
        LOG_INFO("UI", "Tentative %d/%d", attempt, MAX_INIT_ATTEMPTS);
        
        lcd.init();
        lcd.backlight();
        lcd.clear();
        
        lcd.setCursor(0, 0);
        lcd.print("Test LCD");
        
        lcdInitialized = true;
        createCustomChars();
        
        // Configuration des boutons
        pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
        pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
        pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
        pinMode(BUTTON_YELLOW_PIN, INPUT_PULLUP);
        
        LOG_INFO("UI", "Interface initialisée avec succès!");
        return true;
    }
    
    LOG_ERROR("UI", "Échec de l'initialisation de l'interface");
    lcdInitialized = false;
    return false;
}

void UIManager::createCustomChars() {
    if (!lcdInitialized) return;
    
    lcd.createChar(0, const_cast<uint8_t*>(charDirection));
    lcd.createChar(1, const_cast<uint8_t*>(charLeft));
    lcd.createChar(2, const_cast<uint8_t*>(charRight));
    lcd.createChar(3, const_cast<uint8_t*>(charBlock));
}

void UIManager::clear() {
    if (!lcdInitialized) return;
    lcd.clear();
}

void UIManager::centerText(uint8_t row, const char* text) {
    if (!lcdInitialized || row >= LCD_ROWS) return;
    
    int textLength = strlen(text);
    int position = (LCD_COLS - textLength) / 2;
    position = max(0, position);
    
    lcd.setCursor(position, row);
    lcd.print(text);
}

void UIManager::updateDisplay() {
    if (!lcdInitialized) return;
    
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

void UIManager::checkButtons() {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonCheck < BUTTON_CHECK_INTERVAL) {
        return;
    }
    lastButtonCheck = currentTime;
    
    // Lecture des boutons avec anti-rebond
    uint8_t pins[4] = {BUTTON_BLUE_PIN, BUTTON_GREEN_PIN, BUTTON_RED_PIN, BUTTON_YELLOW_PIN};
    
    for (int i = 0; i < 4; i++) {
        bool reading = !digitalRead(pins[i]); // Inversé car INPUT_PULLUP
        
        if (reading != lastButtonStates[i]) {
            lastDebounceTime[i] = currentTime;
        }
        
        if ((currentTime - lastDebounceTime[i]) > BUTTON_DEBOUNCE_DELAY) {
            if (reading != buttonStates[i]) {
                buttonStates[i] = reading;
                if (reading) {
                    // Action sur pression du bouton
                    switch (i) {
                        case 0: // BLUE
                            menuBack();
                            break;
                        case 1: // GREEN
                            menuUp();
                            break;
                        case 2: // RED
                            menuSelect();
                            break;
                        case 3: // YELLOW
                            menuDown();
                            break;
                    }
                }
            }
        }
        
        lastButtonStates[i] = reading;
    }
}

bool UIManager::isButtonPressed(uint8_t buttonId) {
    if (buttonId >= 4) return false;
    return buttonStates[buttonId];
}

void UIManager::updateMainDisplay() {
    clear();
    centerText(0, "Kite PiloteV3");
    
    // Statut WiFi
    lcd.setCursor(0, 1);
    if (WiFi.status() == WL_CONNECTED) {
        lcd.print("WiFi: ");
        lcd.print(WiFi.SSID());
    } else {
        lcd.print("WiFi: Déconnecté");
    }
    
    // Message système
    lcd.setCursor(0, 2);
    lcd.print("Appuyez pour menu");
    
    // Uptime
    lcd.setCursor(0, 3);
    lcd.print("Uptime: ");
    unsigned long uptime = millis() / 1000;
    lcd.print(uptime);
    lcd.print("s");
}

void UIManager::updateDirectionTrimDisplay(int direction, int trim) {
    clear();
    centerText(0, "Contrôle Direction");
    
    // Direction
    lcd.setCursor(0, 1);
    lcd.print("Dir:");
    drawDirection(1, direction);
    lcd.setCursor(16, 1);
    if (direction >= 0) lcd.print(" ");
    lcd.print(direction);
    
    // Trim
    lcd.setCursor(0, 2);
    lcd.print("Trim:");
    drawDirection(2, trim);
    lcd.setCursor(16, 2);
    if (trim >= 0) lcd.print(" ");
    lcd.print(trim);
    
    // Aide
    lcd.setCursor(0, 3);
    lcd.print("POT1:Dir POT2:Trim");
}

void UIManager::updateLineLengthDisplay(int length) {
    clear();
    centerText(0, "Longueur Lignes");
    
    lcd.setCursor(0, 1);
    lcd.print("Longueur: ");
    lcd.print(length);
    lcd.print("%");
    
    drawProgressBar(2, length);
    
    lcd.setCursor(0, 3);
    lcd.print("POT3: Longueur");
}

void UIManager::displayMessage(const char* title, const char* message) {
    if (!lcdInitialized) {
        LOG_INFO("UI", "%s: %s", title, message);
        return;
    }
    
    clear();
    centerText(0, title);
    
    // Affichage du message sur plusieurs lignes si nécessaire
    int msgLen = strlen(message);
    int startIdx = 0;
    
    for (int row = 1; row < LCD_ROWS && startIdx < msgLen; row++) {
        int endIdx = startIdx;
        int lastSpaceIdx = -1;
        
        while (endIdx < msgLen && endIdx < startIdx + LCD_COLS) {
            if (message[endIdx] == ' ') {
                lastSpaceIdx = endIdx;
            }
            endIdx++;
        }
        
        int cutIdx = (lastSpaceIdx != -1 && endIdx < msgLen) ? lastSpaceIdx : endIdx;
        
        lcd.setCursor(0, row);
        for (int i = startIdx; i < cutIdx; i++) {
            lcd.print(message[i]);
        }
        
        startIdx = (lastSpaceIdx != -1 && endIdx < msgLen) ? lastSpaceIdx + 1 : endIdx;
    }
}

void UIManager::displayWiFiInfo(const String& ssid, const IPAddress& ip) {
    clear();
    centerText(0, "Info WiFi");
    
    // SSID
    lcd.setCursor(0, 1);
    lcd.print("SSID: ");
    if (ssid.length() > LCD_COLS - 6) {
        lcd.print(ssid.substring(0, LCD_COLS - 9));
        lcd.print("...");
    } else {
        lcd.print(ssid);
    }
    
    // IP
    lcd.setCursor(0, 2);
    lcd.print("IP: ");
    char ipBuffer[16];
    snprintf(ipBuffer, sizeof(ipBuffer), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    lcd.print(ipBuffer);
    
    // État
    lcd.setCursor(0, 3);
    if (WiFi.status() == WL_CONNECTED) {
        char rssiBuffer[16];
        snprintf(rssiBuffer, sizeof(rssiBuffer), "Signal: %d dBm", WiFi.RSSI());
        lcd.print(rssiBuffer);
    } else {
        lcd.print("Déconnecté");
    }
}

void UIManager::displaySystemStats() {
    clear();
    centerText(0, "Système");
    
    // Mémoire
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint8_t heapPercent = (freeHeap * 100) / totalHeap;
    
    lcd.setCursor(0, 1);
    lcd.print("Mém: ");
    lcd.print(freeHeap / 1024);
    lcd.print("/");
    lcd.print(totalHeap / 1024);
    lcd.print("KB");
    
    // CPU
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

void UIManager::drawProgressBar(uint8_t row, uint8_t percent) {
    if (!lcdInitialized || row >= LCD_ROWS) return;
    
    percent = constrain(percent, 0, 100);
    int numBlocks = (percent * LCD_COLS) / 100;
    
    lcd.setCursor(0, row);
    
    for (int i = 0; i < numBlocks; i++) {
        lcd.write(byte(3)); // Bloc plein
    }
    
    for (int i = numBlocks; i < LCD_COLS; i++) {
        lcd.print(" ");
    }
}

void UIManager::drawDirection(uint8_t row, int value) {
    if (!lcdInitialized || row >= LCD_ROWS) return;
    
    value = constrain(value, -100, 100);
    int center = 10;
    int position = center + (value * (center - 1)) / 100;
    position = constrain(position, 1, 19);
    
    lcd.setCursor(0, row);
    
    for (int i = 0; i < LCD_COLS; i++) {
        if (i == position) {
            lcd.write(byte(0)); // Flèche de direction
        } else if (i == center) {
            lcd.print("|");
        } else {
            lcd.print(".");
        }
    }
}

void UIManager::showMenu(MenuState menu) {
    if (!lcdInitialized) return;
    
    currentMenu = menu;
    currentMenuSelection = 0;
    clear();
    
    switch (menu) {
        case MENU_MAIN:
            centerText(0, "Menu Principal");
            printMenuItem(1, "Contrôle", currentMenuSelection == 0);
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

void UIManager::printMenuItem(uint8_t row, const char* text, bool selected) {
    if (!lcdInitialized || row >= LCD_ROWS) return;
    
    lcd.setCursor(0, row);
    lcd.print(selected ? ">" : " ");
    lcd.print(text);
    
    // Effacer le reste de la ligne
    for (int i = strlen(text) + 1; i < LCD_COLS; i++) {
        lcd.print(" ");
    }
}

void UIManager::menuUp() {
    if (!lcdInitialized) return;
    
    if (currentMenuSelection > 0) {
        currentMenuSelection--;
        showMenu(currentMenu);
    }
}

void UIManager::menuDown() {
    if (!lcdInitialized) return;
    
    const uint8_t menuItemCounts[] = {3, 3, 3, 3, 3};
    
    if (currentMenuSelection < menuItemCounts[currentMenu] - 1) {
        currentMenuSelection++;
        showMenu(currentMenu);
    }
}

void UIManager::menuSelect() {
    if (!lcdInitialized) return;
    
    switch (currentMenu) {
        case MENU_MAIN:
            switch (currentMenuSelection) {
                case 0:
                    showMenu(MENU_CONTROL);
                    break;
                case 1:
                    showMenu(MENU_SETTINGS);
                    break;
                case 2:
                    showMenu(MENU_SYSTEM);
                    break;
            }
            break;
            
        case MENU_SETTINGS:
            switch (currentMenuSelection) {
                case 0:
                    displayMessage("Calibration", "Mode calibration\nDéplacez les pots");
                    break;
                case 1:
                    showMenu(MENU_WIFI);
                    break;
                case 2:
                    showMenu(MENU_MAIN);
                    break;
            }
            break;
            
        case MENU_CONTROL:
            switch (currentMenuSelection) {
                case 0:
                    currentDisplayState = DISPLAY_DIRECTION_TRIM;
                    updateDisplay();
                    break;
                case 1:
                    currentDisplayState = DISPLAY_LINE_LENGTH;
                    updateDisplay();
                    break;
                case 2:
                    showMenu(MENU_MAIN);
                    break;
            }
            break;
            
        case MENU_WIFI:
            switch (currentMenuSelection) {
                case 0:
                    displayMessage("WiFi", "Connexion en cours...");
                    break;
                case 1:
                    currentDisplayState = DISPLAY_WIFI_INFO;
                    updateDisplay();
                    break;
                case 2:
                    showMenu(MENU_SETTINGS);
                    break;
            }
            break;
            
        case MENU_SYSTEM:
            switch (currentMenuSelection) {
                case 0:
                    currentDisplayState = DISPLAY_SYSTEM_STATS;
                    updateDisplay();
                    break;
                case 1:
                    displayMessage("OTA", "Préparation OTA...");
                    break;
                case 2:
                    showMenu(MENU_MAIN);
                    break;
            }
            break;
    }
}

void UIManager::menuBack() {
    if (!lcdInitialized) return;
    
    switch (currentMenu) {
        case MENU_MAIN:
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

void UIManager::checkDisplayStatus() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastDisplayCheck < DISPLAY_CHECK_INTERVAL) {
        return;
    }
    
    lastDisplayCheck = currentTime;
    
    if (!lcdInitialized) {
        begin();
    }
}