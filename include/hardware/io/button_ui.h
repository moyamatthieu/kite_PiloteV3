/**
 * @file button_ui.h
 * @brief Module de gestion de l'interface utilisateur à boutons pour Kite PiloteV3
 * 
 * Ce module définit la classe ButtonUIManager qui gère l'interaction utilisateur
 * via un écran LCD 2004 et quatre boutons poussoirs (bleu, vert, rouge, jaune).
 * 
 * Caractéristiques principales :
 * - Gestion de menus hiérarchiques
 * - Navigation et sélection via boutons
 * - Anti-rebond et gestion des états des boutons
 * 
 * @version 1.1.0
 * @date 1 mai 2025
 * @author Équipe Kite PiloteV3
 * @copyright Tous droits réservés
 */

#ifndef BUTTON_UI_H
#define BUTTON_UI_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "display.h"

// Constantes pour les boutons
#define BUTTON_BLUE 0   // Retour
#define BUTTON_GREEN 1  // Navigation haut
#define BUTTON_RED 2    // Validation
#define BUTTON_YELLOW 3 // Navigation bas

// Paramètres de l'interface
#define MAX_MENUS 10
#define MAX_MENU_ITEMS 10
#define BUTTON_DEBOUNCE_DELAY 50    // Délai de debounce en ms
#define BUTTON_CHECK_INTERVAL 20    // Intervalle de vérification des boutons en ms

/**
 * @struct Menu_UI
 * @brief Structure représentant un menu dans l'interface utilisateur
 * 
 * Cette structure définit un menu avec ses éléments et une fonction de rappel optionnelle.
 * Permet de créer des menus dynamiques avec des actions personnalisées.
 */
struct Menu_UI {
  String name;                        ///< Nom descriptif du menu
  String items[MAX_MENU_ITEMS];       ///< Liste des éléments du menu
  uint8_t itemCount;                  ///< Nombre d'éléments dans le menu
  void (*callback)(uint8_t);          ///< Fonction de rappel appelée lors de la sélection d'un élément
};

/**
 * @class ButtonUIManager
 * @brief Gestionnaire de l'interface utilisateur à boutons pour le système Kite PiloteV3
 * 
 * Cette classe gère la navigation et l'interaction dans les menus via quatre boutons :
 * - Bouton bleu : Retour au menu principal
 * - Bouton vert : Navigation vers le haut
 * - Bouton rouge : Validation/Sélection
 * - Bouton jaune : Navigation vers le bas
 * 
 * Fonctionnalités principales :
 * - Création et gestion de menus dynamiques
 * - Gestion de l'anti-rebond des boutons
 * - Interaction avec le gestionnaire d'affichage
 */
class ButtonUIManager {
public:
  /**
   * @brief Constructeur de ButtonUIManager
   * @param displayManager Pointeur vers le gestionnaire d'affichage
   * @note Initialise les variables internes et prépare l'interface
   */
  ButtonUIManager(DisplayManager* displayManager);
  
  /**
   * @brief Destructeur de ButtonUIManager
   * @note Libère les ressources si nécessaire
   */
  ~ButtonUIManager();
  
  // Méthodes d'initialisation
  /**
   * @brief Initialise l'interface utilisateur
   * @details Configure les pins des boutons et crée les menus par défaut
   */
  void begin();
  
  // Méthodes de gestion des menus
  /**
   * @brief Crée un nouveau menu
   * @param name Nom du menu
   * @param callback Fonction de rappel optionnelle pour les actions du menu
   * @return Identifiant du menu créé
   */
  uint8_t createMenu(const String& name, void (*callback)(uint8_t) = nullptr);
  
  /**
   * @brief Ajoute un élément à un menu existant
   * @param menuId Identifiant du menu
   * @param itemName Nom de l'élément à ajouter
   */
  void addMenuItem(uint8_t menuId, const String& itemName);
  
  /**
   * @brief Affiche un menu spécifique
   * @param menuId Identifiant du menu à afficher
   */
  void showMenu(uint8_t menuId);
  
  // Méthodes de gestion des boutons
  /**
   * @brief Traite les événements des boutons
   * @details Gère la navigation et la sélection dans les menus
   */
  void processButtons();
  
  /**
   * @brief Vérifie si un bouton est actuellement pressé
   * @param buttonId Identifiant du bouton à vérifier
   * @return true si le bouton est pressé, false sinon
   */
  bool isButtonPressed(uint8_t buttonId);
  
private:
  DisplayManager* display;            ///< Référence au gestionnaire d'affichage

  // Variables pour les menus
  Menu_UI menus[MAX_MENUS];           ///< Tableau des menus disponibles
  uint8_t menuCount;                  ///< Nombre total de menus créés
  uint8_t currentMenu;                ///< Index du menu actuellement affiché
  uint8_t currentSelection;           ///< Index de l'élément actuellement sélectionné
  
  // Variables pour les boutons
  bool buttonStates[4];               ///< État actuel des boutons
  bool lastButtonStates[4];           ///< État précédent des boutons
  unsigned long lastDebounceTime[4];  ///< Horodatage du dernier changement d'état pour anti-rebond
  unsigned long lastButtonCheck;      ///< Horodatage de la dernière vérification des boutons
  
  // Méthodes privées
  /**
   * @brief Lit l'état des boutons
   * @details Met à jour buttonStates avec l'état actuel des boutons
   */
  void readButtons();
  
  /**
   * @brief Gère une pression de bouton
   * @param buttonId Identifiant du bouton pressé
   */
  void handleButtonPress(uint8_t buttonId);
  
  /**
   * @brief Navigue vers le haut dans le menu
   */
  void navigateUp();
  
  /**
   * @brief Navigue vers le bas dans le menu
   */
  void navigateDown();
  
  /**
   * @brief Sélectionne l'élément de menu actuel
   */
  void selectMenuItem();
  
  // Création des menus standard
  /**
   * @brief Crée le menu principal par défaut
   */
  void createMainMenu();
  
  /**
   * @brief Crée le menu des paramètres par défaut
   */
  void createSettingsMenu();
  
  /**
   * @brief Crée le menu de contrôle par défaut
   */
  void createControlMenu();
};

#endif // BUTTON_UI_H
