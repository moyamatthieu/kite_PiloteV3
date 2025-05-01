/*
  -----------------------
  Kite PiloteV3 - Module d'interface tactile
  -----------------------
  
  Module de gestion de l'interface tactile et des interactions utilisateur.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef TOUCH_UI_H
#define TOUCH_UI_H

#include <Arduino.h>
#include "../include/config.h"
#include "../include/display.h"

// Structure pour définir un bouton tactile
typedef struct {
  uint16_t x;          // Position X du bouton
  uint16_t y;          // Position Y du bouton
  uint16_t width;      // Largeur du bouton
  uint16_t height;     // Hauteur du bouton
  uint16_t color;      // Couleur du bouton
  String label;        // Texte du bouton
  bool enabled;        // État d'activation du bouton
  uint8_t id;          // Identifiant unique du bouton
} TouchButton;

// Structure pour définir un écran tactile
typedef struct {
  String name;                       // Nom de l'écran
  TouchButton buttons[MAX_BUTTONS_PER_SCREEN]; // Tableau de boutons (utilisation de la constante de config.h)
  uint8_t buttonCount;               // Nombre de boutons sur cet écran
  void (*callback)(uint8_t buttonId); // Fonction callback pour les actions de l'écran
} TouchScreen_UI;

class TouchUIManager {
  public:
    // Constructeur et destructeur
    TouchUIManager(DisplayManager* displayManager);
    ~TouchUIManager();
    
    // Initialisation et configuration
    void begin();
    void calibrateTouch();
    
  // Gestion des écrans et des boutons - callback direct sans lambda
  uint8_t createScreen(const String& name, void (*callback)(uint8_t) = nullptr);
    uint8_t addButton(uint8_t screenId, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const String& label, uint16_t color);
    void showScreen(uint8_t screenId);
    
    // Traitement des événements tactiles
    void processTouch();
    bool isTouched();
    
    // Écrans prédéfinis
    void createMainScreen();
    void createSettingsScreen();
    void createDashboardScreen();
    
  private:
    DisplayManager* display;            // Référence au gestionnaire d'affichage
    TouchScreen_UI screens[MAX_SCREENS]; // Tableau d'écrans tactiles (utilisation de la constante de config.h)
    uint8_t screenCount;                // Nombre d'écrans configurés
    uint8_t currentScreen;              // Écran actuellement affiché
    
    // Variables pour la calibration
    int16_t touchMinX;
    int16_t touchMaxX;
    int16_t touchMinY;
    int16_t touchMaxY;
    
    // Données du dernier toucher
    int16_t lastTouchX;
    int16_t lastTouchY;
    bool touchActive;
    unsigned long lastTouchTime;
    
    // Fonctions utilitaires privées
    void drawButton(const TouchButton& button);
    bool isButtonPressed(const TouchButton& button, int16_t x, int16_t y);
    int16_t mapTouchX(int16_t rawX);
    int16_t mapTouchY(int16_t rawY);
};

#endif // TOUCH_UI_H
