/*
  -----------------------
  Kite PiloteV3 - Module d'interface tactile (Implémentation)
  -----------------------
  
  Implémentation des fonctions du module de gestion de l'interface tactile.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../include/touch_ui.h"
#include "../include/logging.h"

/**
 * Constructeur de la classe TouchUIManager
 * @param displayManager Pointeur vers le gestionnaire d'affichage
 */
TouchUIManager::TouchUIManager(DisplayManager* displayManager)
  : display(displayManager),
    screenCount(0),
    currentScreen(0),
    touchMinX(0),
    touchMaxX(240),
    touchMinY(0),
    touchMaxY(320),
    lastTouchX(0),
    lastTouchY(0),
    touchActive(false),
    lastTouchTime(0) {
}

/**
 * Destructeur de la classe TouchUIManager
 */
TouchUIManager::~TouchUIManager() {
  // Rien à libérer pour l'instant
}

/**
 * Initialise le gestionnaire d'interface tactile
 */
void TouchUIManager::begin() {
  Serial.println("Initialisation de l'interface tactile");
  
  // Initialiser les écrans par défaut
  createMainScreen();
  createSettingsScreen();
  createDashboardScreen();
  
  // Afficher l'écran principal par défaut
  showScreen(0);
}

/**
 * Calibre l'écran tactile
 * Ne fait rien pour l'écran capacitif FT6206 qui ne nécessite pas de calibration
 */
void TouchUIManager::calibrateTouch() {
  if (!display->isInitialized() || !display->isTouchInitialized()) {
    Serial.println("Impossible de calibrer - affichage ou écran tactile non initialisé");
    return;
  }
  
  Serial.println("Le contrôleur tactile capacitif FT6206 ne nécessite pas de calibration");
  
  // Afficher un message de confirmation
  display->displayMessage("Écran Tactile", "Contrôleur FT6206 initialisé");
  
  delay(1000);
}

/**
 * Crée un nouvel écran dans l'interface tactile
 * @param name Nom de l'écran
 * @param callback Fonction de rappel pour les actions sur cet écran
 * @return ID de l'écran créé
 */
uint8_t TouchUIManager::createScreen(const String& name, void (*callback)(uint8_t)) {
  if (screenCount >= 10) {
    LOG_ERROR("UI", "Nombre maximum d'écrans atteint");
    return 0;
  }
  
  uint8_t screenId = screenCount;
  screens[screenId].name = name;
  screens[screenId].buttonCount = 0;
  screens[screenId].callback = callback;
  
  screenCount++;
  return screenId;
}

/**
 * Ajoute un bouton à un écran spécifique
 * @param screenId ID de l'écran
 * @param x Position X du bouton
 * @param y Position Y du bouton
 * @param width Largeur du bouton
 * @param height Hauteur du bouton
 * @param label Texte du bouton
 * @param color Couleur du bouton
 * @return ID du bouton créé
 */
uint8_t TouchUIManager::addButton(uint8_t screenId, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const String& label, uint16_t color) {
  if (screenId >= screenCount || screens[screenId].buttonCount >= 10) {
    Serial.println("Erreur: écran invalide ou nombre maximum de boutons atteint");
    return 0;
  }
  
  uint8_t buttonId = screens[screenId].buttonCount;
  
  TouchButton button;
  button.x = x;
  button.y = y;
  button.width = width;
  button.height = height;
  button.label = label;
  button.color = color;
  button.enabled = true;
  button.id = buttonId;
  
  screens[screenId].buttons[buttonId] = button;
  screens[screenId].buttonCount++;
  
  return buttonId;
}

/**
 * Affiche un écran spécifique
 * @param screenId ID de l'écran à afficher
 */
void TouchUIManager::showScreen(uint8_t screenId) {
  if (screenId >= screenCount) {
    Serial.println("Erreur: ID d'écran invalide");
    return;
  }
  
  currentScreen = screenId;
  
  if (!display->isInitialized()) {
    Serial.println("Erreur: affichage non initialisé");
    return;
  }
  
  // Effacer l'écran et dessiner l'en-tête
  display->tft.fillScreen(COLOR_BLACK);
  display->tft.fillRect(0, 0, SCREEN_WIDTH, 30, COLOR_BLUE);
  display->tft.setTextColor(COLOR_WHITE);
  display->tft.setTextSize(2);
  
  // Centrer le titre
  int16_t x1, y1;
  uint16_t w, h;
  String title = screens[screenId].name;
  display->tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  display->tft.setCursor((SCREEN_WIDTH - w) / 2, 9);
  display->tft.print(title);
  
  // Dessiner tous les boutons de cet écran
  for (uint8_t i = 0; i < screens[screenId].buttonCount; i++) {
    if (screens[screenId].buttons[i].enabled) {
      drawButton(screens[screenId].buttons[i]);
    }
  }
}

/**
 * Traite les événements tactiles
 * Cette fonction doit être appelée régulièrement dans la boucle principale
 */
void TouchUIManager::processTouch() {
  // Vérification de la validité des objets
  if (display == nullptr) {
    return;
  }
  
  // Vérification de l'initialisation
  if (!display->isInitialized() || !display->isTouchInitialized()) {
    return;
  }
  
  // Protection contre les accès en dehors des limites
  if (currentScreen >= screenCount || currentScreen >= MAX_SCREENS) {
    currentScreen = 0;
    return; // Sortir immédiatement pour éviter d'accéder à un écran non valide
  }
  
  // Utiliser une variable static pour éviter les déclenchements multiples
  static unsigned long lastTouchTime = 0;
  unsigned long now = millis();
  
  // Vérifie si l'écran est touché et si assez de temps s'est écoulé depuis la dernière pression
  if (display->touched() && now - lastTouchTime > 300) {
    // Récupérer les coordonnées du point touché
    TS_Point p;
    try {
      p = display->getTouch();
    } catch (...) {
      Serial.println("Exception dans getTouch()");
      return;
    }
    
    // Mettre à jour le temps du dernier toucher
    lastTouchTime = now;
    
    // Vérifier que le nombre de boutons est valide
    if (screens[currentScreen].buttonCount > MAX_BUTTONS_PER_SCREEN) {
      screens[currentScreen].buttonCount = MAX_BUTTONS_PER_SCREEN;
    }
    
    // Coordonnées du point touché
    int16_t touchX = p.x;
    int16_t touchY = p.y;
    
    // Structure pour stocker l'action à exécuter - remplace les callbacks directs
    static struct {
      bool actionPending;
      uint8_t buttonId;
      uint8_t screenId;
    } nextAction = {false, 0, 0};
    
    // Vérifier si nous devons exécuter une action différée
    if (nextAction.actionPending) {
      // Limitation des actions pour éviter les corruptions de mémoire
      nextAction.actionPending = false;
      
      // Limiter les appels de callback aux écrans valides
      if (nextAction.screenId < screenCount && 
          screens[nextAction.screenId].callback != nullptr &&
          nextAction.buttonId < screens[nextAction.screenId].buttonCount) {
        screens[nextAction.screenId].callback(nextAction.buttonId);
      }
    }
    
    // Vérifier si un bouton a été touché
    for (uint8_t i = 0; i < screens[currentScreen].buttonCount && i < MAX_BUTTONS_PER_SCREEN; i++) {
      TouchButton& button = screens[currentScreen].buttons[i];
      
      if (button.enabled && isButtonPressed(button, touchX, touchY)) {
        // Feedback visuel - redessiner le bouton sans utiliser delay()
        display->tft.fillRoundRect(button.x, button.y, button.width, button.height, 8, COLOR_WHITE);
        
        // Planifier le redessinage après le feedback visuel et l'appel du callback
        nextAction.actionPending = true;
        nextAction.buttonId = button.id;
        nextAction.screenId = currentScreen;
        
        // Sortir immédiatement de la boucle
        break;
      }
    }
  }
}

/**
 * Vérifie si l'écran est actuellement touché
 * @return true si l'écran est touché, false sinon
 */
bool TouchUIManager::isTouched() {
  if (!display->isInitialized() || !display->isTouchInitialized()) {
    return false;
  }
  
  return display->touched();
}

/**
 * Crée l'écran principal avec les boutons correspondants
 */
void TouchUIManager::createMainScreen() {
  uint8_t screenId = createScreen("Menu Principal", [](uint8_t buttonId) {
    // Cette fonction lambda sera appelée lorsqu'un bouton est pressé
    Serial.printf("Bouton pressé sur l'écran principal: %d\n", buttonId);
  });
  
  // Ajouter des boutons à l'écran principal
  addButton(screenId, 40, 60, 240, 50, "Tableau de bord", COLOR_BLUE);
  addButton(screenId, 40, 120, 240, 50, "Paramètres", COLOR_GREEN);
  addButton(screenId, 40, 180, 240, 50, "Informations", COLOR_MAGENTA);
}

/**
 * Crée l'écran des paramètres
 */
void TouchUIManager::createSettingsScreen() {
  uint8_t screenId = createScreen("Paramètres", [](uint8_t buttonId) {
    Serial.printf("Bouton pressé sur l'écran des paramètres: %d\n", buttonId);
  });
  
  // Ajouter des boutons à l'écran des paramètres
  addButton(screenId, 20, 60, 130, 50, "WiFi", COLOR_BLUE);
  addButton(screenId, 170, 60, 130, 50, "Affichage", COLOR_GREEN);
  addButton(screenId, 20, 120, 130, 50, "Système", COLOR_RED);
  addButton(screenId, 170, 120, 130, 50, "OTA", COLOR_MAGENTA);
  addButton(screenId, 95, 180, 130, 50, "Retour", COLOR_ORANGE);
}

/**
 * Crée l'écran du tableau de bord
 */
void TouchUIManager::createDashboardScreen() {
  uint8_t screenId = createScreen("Tableau de bord", [](uint8_t buttonId) {
    Serial.printf("Bouton pressé sur le tableau de bord: %d\n", buttonId);
  });
  
  // Ajouter des boutons à l'écran du tableau de bord
  addButton(screenId, 20, 60, 130, 50, "Graphiques", COLOR_BLUE);
  addButton(screenId, 170, 60, 130, 50, "Données", COLOR_GREEN);
  addButton(screenId, 20, 120, 130, 50, "Alarmes", COLOR_RED);
  addButton(screenId, 170, 120, 130, 50, "Log", COLOR_MAGENTA);
  addButton(screenId, 95, 180, 130, 50, "Retour", COLOR_ORANGE);
}

/**
 * Dessine un bouton sur l'écran
 * @param button Objet bouton à dessiner
 */
void TouchUIManager::drawButton(const TouchButton& button) {
  display->tft.fillRoundRect(button.x, button.y, button.width, button.height, 8, button.color);
  display->tft.drawRoundRect(button.x, button.y, button.width, button.height, 8, COLOR_WHITE);
  
  // Centrer le texte
  display->tft.setTextColor(COLOR_WHITE);
  display->tft.setTextSize(2);
  
  int16_t x1, y1;
  uint16_t w, h;
  display->tft.getTextBounds(button.label, 0, 0, &x1, &y1, &w, &h);
  
  display->tft.setCursor(button.x + (button.width - w) / 2, button.y + (button.height - h) / 2 + 4);
  display->tft.print(button.label);
}

/**
 * Vérifie si les coordonnées correspondent à un bouton
 * @param button Objet bouton à vérifier
 * @param x Coordonnée X du point touché
 * @param y Coordonnée Y du point touché
 * @return true si le bouton est pressé, false sinon
 */
bool TouchUIManager::isButtonPressed(const TouchButton& button, int16_t x, int16_t y) {
  return (x >= button.x && x <= button.x + button.width &&
          y >= button.y && y <= button.y + button.height);
}

/**
 * Convertit une coordonnée X brute en coordonnée d'écran
 * @param rawX Coordonnée X brute
 * @return Coordonnée X convertie
 */
int16_t TouchUIManager::mapTouchX(int16_t rawX) {
  // Pour l'écran capacitif FT6206, les coordonnées sont déjà adaptées
  return rawX;
}

/**
 * Convertit une coordonnée Y brute en coordonnée d'écran
 * @param rawY Coordonnée Y brute
 * @return Coordonnée Y convertie
 */
int16_t TouchUIManager::mapTouchY(int16_t rawY) {
  // Pour l'écran capacitif FT6206, les coordonnées sont déjà adaptées
  return rawY;
}
