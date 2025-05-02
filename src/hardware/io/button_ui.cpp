/**
 * @file button_ui.cpp
 * @brief Implémentation du module de gestion de l'interface utilisateur à boutons
 * 
 * Ce fichier contient l'implémentation de la classe ButtonUIManager,
 * qui gère l'interaction utilisateur via quatre boutons poussoirs sur un écran LCD 2004.
 * 
 * Fonctionnalités principales :
 * - Gestion des menus dynamiques
 * - Anti-rebond des boutons
 * - Navigation et sélection dans les menus
 * 
 * @version 1.1.0
 * @date 1 mai 2025
 * @author Équipe Kite PiloteV3
 * @copyright Tous droits réservés
 */

#include "../include/button_ui.h"

/**
 * @brief Constructeur de ButtonUIManager
 * 
 * Initialise les variables membres et prépare l'interface utilisateur.
 * 
 * @param displayManager Pointeur vers le gestionnaire d'affichage
 * @note Réinitialise tous les états des boutons et des menus
 */
ButtonUIManager::ButtonUIManager(DisplayManager* displayManager) {
  display = displayManager;
  menuCount = 0;
  currentMenu = 0;
  currentSelection = 0;
  lastButtonCheck = 0;
  
  // Initialisation des états des boutons
  for (int i = 0; i < 4; i++) {
    buttonStates[i] = false;
    lastButtonStates[i] = false;
    lastDebounceTime[i] = 0;
  }
}

/**
 * @brief Destructeur de ButtonUIManager
 * 
 * Libère les ressources utilisées par le gestionnaire d'interface.
 * Dans cette implémentation, aucune ressource dynamique n'est allouée.
 */
ButtonUIManager::~ButtonUIManager() {
  // Pas de ressources dynamiques à libérer
}

/**
 * @brief Initialise le gestionnaire d'interface utilisateur
 * 
 * Configure les pins des boutons, crée les menus par défaut et affiche le menu principal.
 * 
 * Étapes :
 * 1. Configuration des pins des boutons en mode INPUT_PULLUP
 * 2. Création des menus standard
 * 3. Affichage du menu principal
 * 
 * @note Les boutons sont configurés avec des résistances pull-up internes
 */
void ButtonUIManager::begin() {
  // Configuration des pins des boutons en mode INPUT_PULLUP
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);   // Bouton retour
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);  // Bouton navigation haut
  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);    // Bouton de validation
  pinMode(BUTTON_YELLOW_PIN, INPUT_PULLUP); // Bouton navigation bas
  
  // Création des menus par défaut
  createMainMenu();
  createSettingsMenu();
  createControlMenu();
  
  // Affichage du menu principal
  showMenu(0); // Menu principal
}

// Création d'un nouveau menu
uint8_t ButtonUIManager::createMenu(const String& name, void (*callback)(uint8_t)) {
  if (menuCount >= MAX_MENUS) {
    return 0xFF; // Erreur: nombre maximal de menus atteint
  }
  
  Menu_UI newMenu;
  newMenu.name = name;
  newMenu.itemCount = 0;
  newMenu.callback = callback;
  
  menus[menuCount] = newMenu;
  return menuCount++;
}

// Ajout d'un élément à un menu
void ButtonUIManager::addMenuItem(uint8_t menuId, const String& itemName) {
  if (menuId >= menuCount || menus[menuId].itemCount >= MAX_MENU_ITEMS) {
    return; // Erreur: menu inexistant ou nombre max d'éléments atteint
  }
  
  menus[menuId].items[menus[menuId].itemCount++] = itemName;
}

// Affichage d'un menu
void ButtonUIManager::showMenu(uint8_t menuId) {
  if (menuId >= menuCount) {
    return; // Erreur: menu inexistant
  }
  
  currentMenu = menuId;
  currentSelection = 0; // Réinitialiser la sélection
  
  display->clear();
  display->displayMessage(menus[menuId].name.c_str(), "");
  
  // Afficher jusqu'à 3 éléments du menu (l'écran LCD a 4 lignes, la première pour le titre)
  for (uint8_t i = 0; i < min(3, (int)menus[menuId].itemCount); i++) {
    // Afficher un indicateur ">" pour l'élément sélectionné
    char buffer[21]; // 20 caractères + null terminator
    if (i == currentSelection) {
      snprintf(buffer, sizeof(buffer), "> %s", menus[menuId].items[i].c_str());
    } else {
      snprintf(buffer, sizeof(buffer), "  %s", menus[menuId].items[i].c_str());
    }
    display->printMenuItem(i+1, buffer, (i == currentSelection)); // +1 car ligne 0 pour le titre
  }
}

// Traitement des boutons
void ButtonUIManager::processButtons() {
  unsigned long currentMillis = millis();
  
  // Vérifier les boutons selon l'intervalle défini
  if (currentMillis - lastButtonCheck < BUTTON_CHECK_INTERVAL) {
    return;
  }
  lastButtonCheck = currentMillis;
  
  readButtons();
  
  // Vérifier les changements d'état des boutons
  for (uint8_t i = 0; i < 4; i++) {
    if (buttonStates[i] != lastButtonStates[i]) {
      // Si le bouton a changé d'état, mettre à jour l'heure du debounce
      lastDebounceTime[i] = currentMillis;
    }
    
    // Si suffisamment de temps s'est écoulé depuis le dernier changement
    if ((currentMillis - lastDebounceTime[i]) > BUTTON_DEBOUNCE_DELAY) {
      // Si le bouton est enfoncé (état bas en mode PULLUP) et l'état précédent était relâché
      if (buttonStates[i] == true && lastButtonStates[i] == false) {
        handleButtonPress(i);
      }
    }
    
    lastButtonStates[i] = buttonStates[i];
  }
}

// Gestion des pressions de boutons
void ButtonUIManager::handleButtonPress(uint8_t buttonId) {
  switch (buttonId) {
    case BUTTON_BLUE:   // Bouton retour
      // Retour au menu précédent/principal
      showMenu(0);
      break;
    case BUTTON_GREEN:  // Bouton navigation haut
      navigateUp();
      break;
    case BUTTON_RED:    // Bouton de validation
      selectMenuItem();
      break;
    case BUTTON_YELLOW: // Bouton navigation bas
      navigateDown();
      break;
  }
}

// Création du menu principal
void ButtonUIManager::createMainMenu() {
  uint8_t mainMenuId = createMenu("Menu Principal", nullptr);
  addMenuItem(mainMenuId, "Controle");
  addMenuItem(mainMenuId, "Parametres");
  addMenuItem(mainMenuId, "Wifi");
  addMenuItem(mainMenuId, "Systeme");
  addMenuItem(mainMenuId, "Retour");
}

// Création du menu des paramètres
void ButtonUIManager::createSettingsMenu() {
  uint8_t settingsMenuId = createMenu("Parametres", nullptr);
  addMenuItem(settingsMenuId, "Calibration");
  addMenuItem(settingsMenuId, "Luminosite");
  addMenuItem(settingsMenuId, "Contraste");
  addMenuItem(settingsMenuId, "Son");
  addMenuItem(settingsMenuId, "Retour");
}

// Création du menu de contrôle
void ButtonUIManager::createControlMenu() {
  uint8_t controlMenuId = createMenu("Controle", nullptr);
  addMenuItem(controlMenuId, "Manuel");
  addMenuItem(controlMenuId, "Auto-Pilot");
  addMenuItem(controlMenuId, "Mode Sport");
  addMenuItem(controlMenuId, "Mode Eco");
  addMenuItem(controlMenuId, "Retour");
}

// Lecture de l'état des boutons
void ButtonUIManager::readButtons() {
  // Lecture inversée car en mode PULLUP: LOW = bouton pressé
  buttonStates[BUTTON_BLUE] = (digitalRead(BUTTON_BLUE_PIN) == LOW);
  buttonStates[BUTTON_GREEN] = (digitalRead(BUTTON_GREEN_PIN) == LOW);
  buttonStates[BUTTON_RED] = (digitalRead(BUTTON_RED_PIN) == LOW);
  buttonStates[BUTTON_YELLOW] = (digitalRead(BUTTON_YELLOW_PIN) == LOW);
}

// Vérification si un bouton est pressé
bool ButtonUIManager::isButtonPressed(uint8_t buttonId) {
  if (buttonId > 3) return false;
  return buttonStates[buttonId];
}

// Navigation vers le haut dans le menu
void ButtonUIManager::navigateUp() {
  if (currentSelection > 0) {
    currentSelection--;
  } else {
    // Boucler à la fin de la liste
    currentSelection = menus[currentMenu].itemCount - 1;
  }
  
  // Rafraîchir l'affichage du menu
  showMenu(currentMenu);
}

// Navigation vers le bas dans le menu
void ButtonUIManager::navigateDown() {
  if (currentSelection < menus[currentMenu].itemCount - 1) {
    currentSelection++;
  } else {
    // Boucler au début de la liste
    currentSelection = 0;
  }
  
  // Rafraîchir l'affichage du menu
  showMenu(currentMenu);
}

// Sélection d'un élément du menu
void ButtonUIManager::selectMenuItem() {
  // Si le menu a une fonction de callback, l'appeler avec l'élément sélectionné
  if (menus[currentMenu].callback != nullptr) {
    menus[currentMenu].callback(currentSelection);
  } else {
    // Comportement par défaut pour les éléments spéciaux
    String selectedItem = menus[currentMenu].items[currentSelection];
    
    if (selectedItem == "Retour") {
      // Retour au menu principal
      showMenu(0);
    } else if (currentMenu == 0) { // Menu principal
      if (selectedItem == "Controle") {
        showMenu(2); // Menu de contrôle
      } else if (selectedItem == "Parametres") {
        showMenu(1); // Menu des paramètres
      } else if (selectedItem == "Wifi") {
        // Afficher les informations WiFi
        display->clear();
        display->displayWiFiInfo(WIFI_SSID, WiFi.localIP());
      } else if (selectedItem == "Systeme") {
        // Afficher les infos système
        display->clear();
        display->updateSystemDisplay();
      }
    }
  }
}
