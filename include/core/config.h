/*
  -----------------------
  Kite PiloteV3 - Fichier de configuration
  -----------------------
  
  Ce fichier regroupe toutes les constantes configurables et définitions de pins
  pour faciliter la maintenance et la personnalisation du système.
  
  Version: 1.0.0
  Date: 2 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === CONFIGURATION HARDWARE ===

// Paramètres de l'écran LCD 2004 I2C
#define LCD_COLS           20      // Nombre de colonnes de l'écran LCD
#define LCD_ROWS           4       // Nombre de lignes de l'écran LCD
#define LCD_I2C_ADDR       0x27    // Adresse I2C du module LCD

// Pins I2C pour l'écran LCD (connexions dans diagram.json)
#define I2C_SDA           21       // Pin SDA pour I2C -> lcd1:SDA
#define I2C_SCL           22       // Pin SCL pour I2C -> lcd1:SCL

// Pins pour les potentiomètres (connexions dans diagram.json)
#define POT_DIRECTION     34       // Pin pour le potentiomètre de direction (ADC1_CH6) -> pot1:SIG
#define POT_TRIM          35       // Pin pour le potentiomètre de trim (ADC1_CH7) -> pot2:SIG
#define POT_LENGTH        32       // Pin pour le potentiomètre de longueur des lignes (ADC1_CH4) -> pot3:SIG

// Pins GPIO pour LED
#define LED_PIN           2        // Pin pour la LED indicatrice -> led1:A via r1

// Pins pour les boutons de l'interface utilisateur (connexions dans diagram.json)
#define BUTTON_UP_PIN       25       // Pin pour le bouton de navigation haut (vert) -> btn2:2.l
#define BUTTON_DOWN_PIN     26       // Pin pour le bouton de navigation bas (jaune) -> btn3:2.l
#define BUTTON_SELECT_PIN   27       // Pin pour le bouton de sélection (rouge) -> btn4:2.l
#define BUTTON_BACK_PIN     35       // Pin pour le bouton de retour (bleu) -> btn1:2.l

// Pins pour les servomoteurs (connexions dans diagram.json)
#define SERVO_DIRECTION_PIN 13       // Pin pour le servomoteur de direction -> servo2:PWM
#define SERVO_TRIM_PIN      14       // Pin pour le servomoteur de trim -> servo1:PWM
#define SERVO_LINEMOD_PIN   15       // Pin pour le servomoteur de modulation de ligne

// Pins pour le moteur pas à pas (connexions dans diagram.json)
#define STEPPER_IN1       5        // Pin IN1 du moteur pas à pas -> stepper1:A+
#define STEPPER_IN2       18       // Pin IN2 du moteur pas à pas -> stepper1:A-
#define STEPPER_IN3       19       // Pin IN3 du moteur pas à pas -> stepper1:B+
#define STEPPER_IN4       23       // Pin IN4 du moteur pas à pas -> stepper1:B-

// Configuration du pilote automatique
#define AUTOPILOT_DISABLED 0        // Mode pilote automatique désactivé
#undef AUTOPILOT_OFF
#define AUTOPILOT_ON      1        // Mode pilote automatique activé
#define AUTOPILOT_SMOOTH_FACTOR 0.8  // Facteur de lissage pour les transitions du pilote automatique
#define POT_CHANGE_THRESHOLD   5    // Seuil de changement pour détecter une action sur un potentiomètre

// === CONFIGURATION RÉSEAU ===

// Configuration WiFi
#define WIFI_SSID         "Wokwi-GUEST"   // SSID du réseau WiFi
#define WIFI_PASSWORD     ""              // Mot de passe WiFi (vide pour réseau ouvert)
#define SERVER_PORT       80              // Port du serveur web
#define WIFI_TIMEOUT_MS   10000           // Timeout pour la connexion WiFi (ms)

// === CONFIGURATION INTERFACE ===

// Caractères spéciaux pour LCD
#define CHAR_RIGHT_ARROW  0x7E    // Caractère flèche droite (→)
#define CHAR_LEFT_ARROW   0x7F    // Caractère flèche gauche (←)
#define CHAR_UP_ARROW     0x5E    // Caractère flèche haut (^)
#define CHAR_DOWN_ARROW   0x76    // Caractère flèche bas (v)
#define CHAR_DEGREE       0xDF    // Caractère degré (°)
#define CHAR_BLOCK        0xFF    // Caractère bloc plein (█)
#define CHAR_DOT          0xA5    // Caractère point (•)

// Définition des boutons
#define BUTTON_BLUE       0       // Identifiant du bouton bleu (retour)
#define BUTTON_GREEN      1       // Identifiant du bouton vert (navigation haut)
#define BUTTON_RED        2       // Identifiant du bouton rouge (validation)
#define BUTTON_YELLOW     3       // Identifiant du bouton jaune (navigation bas)

// === CONFIGURATION SYSTÈME ===

// Paramètres temporels
#define DISPLAY_UPDATE_INTERVAL    250    // Intervalle entre les mises à jour d'affichage (ms)
#define MENU_SCROLL_INTERVAL       600    // Intervalle de défilement du menu (ms)
#define DISPLAY_CHECK_INTERVAL    45000   // Intervalle pour vérifier l'état de l'écran (ms)
#define SYSTEM_CHECK_INTERVAL     15000   // Intervalle pour les vérifications système (ms)
#define INIT_RETRY_DELAY          500     // Délai entre les tentatives d'initialisation (ms)
#define WIFI_CHECK_INTERVAL       5000    // Intervalle pour vérifier la connexion WiFi (ms)
#define POT_READ_INTERVAL         100     // Intervalle de lecture des potentiomètres (ms)
#define BUTTON_CHECK_INTERVAL     50      // Intervalle de vérification des boutons (ms)
#define BUTTON_DEBOUNCE_DELAY     50      // Délai d'anti-rebond pour les boutons (ms)
#define SERVO_UPDATE_INTERVAL     20      // Intervalle de mise à jour des servomoteurs (ms)
#define STEPPER_UPDATE_INTERVAL   5       // Intervalle de mise à jour du moteur pas à pas (ms)
#define DISPLAY_REFRESH_INTERVAL  200     // Intervalle de rafraîchissement de l'affichage (ms)

// Configuration de l'affichage
#define DISPLAY_UPDATE_THROTTLE    200    // Limite de mise à jour d'affichage (ms)

// Configuration Servomoteurs
#define SERVO_MIN_PULSE_WIDTH     500     // Largeur d'impulsion minimale (us)
#define SERVO_MAX_PULSE_WIDTH     2500    // Largeur d'impulsion maximale (us)
#define SERVO_MIN_ANGLE           0       // Angle minimum (degrés)
#define SERVO_MAX_ANGLE           180     // Angle maximum (degrés)
#define SERVO_FREQUENCY           50      // Fréquence PWM (Hz)

// Configuration moteur pas à pas
#define STEPPER_STEPS_PER_REV     200     // Nombre de pas par tour complet
#define STEPPER_MAX_SPEED         1000    // Vitesse maximale en pas/seconde
#define STEPPER_ACCELERATION      500     // Accélération en pas/seconde²

// Configuration ADC
#define ADC_RESOLUTION            4095    // Résolution ADC (12 bits pour ESP32)
#define ADC_SMOOTHING_FACTOR      0.1     // Facteur de lissage pour les lectures ADC (filtre passe-bas)
#define POT_DEADZONE              10      // Zone morte pour éviter le bruit du potentiomètre

// Nombre maximum de tentatives
#define MAX_INIT_ATTEMPTS         3       // Nombre maximal de tentatives d'initialisation

// === CONFIGURATION INTERFACE MENU ===

// Limites pour les écrans de menu avec boutons
#define MAX_MENUS                 8       // Nombre maximum de menus
#define MAX_MENU_ITEMS            6       // Nombre maximum d'éléments par menu
#define MAX_BUTTONS               4       // Nombre de boutons physiques
#define MAX_SCREENS               4       // Nombre maximum d'écrans de menu

// === CONFIGURATION TÂCHES FREERTOS ===

// Taille des piles de tâches FreeRTOS (en mots)
#define DISPLAY_TASK_STACK_SIZE   2048    // Réduit pour optimisation mémoire - LCD simple
#define POT_TASK_STACK_SIZE       2048    // Tâche de lecture des potentiomètres
#define MOTOR_TASK_STACK_SIZE     3072    // Tâche de contrôle des moteurs
#define WIFI_TASK_STACK_SIZE      4096    // Tâche WiFi
#define SYSTEM_TASK_STACK_SIZE    2048    // Réduit pour optimisation mémoire

// Configuration mémoire
#ifndef MEMORY_OPTIMIZATION_ENABLED
#define MEMORY_OPTIMIZATION_ENABLED true // Active les optimisations de mémoire
#endif
#define STRING_BUFFER_SIZE         128     // Taille des buffers pour les chaînes de caractères
#define LOG_BUFFER_SIZE            192     // Taille des buffers de logging
#define MAX_DISPLAY_BUFFER_SIZE    128     // Réduit - LCD a besoin de moins de mémoire que TFT

// Priorités des tâches FreeRTOS (0-24, 24 étant la plus haute)
#define DISPLAY_TASK_PRIORITY     2       // Priorité de la tâche d'affichage
#define POT_TASK_PRIORITY         2       // Priorité de la tâche de lecture des potentiomètres
#define MOTOR_TASK_PRIORITY       3       // Priorité de la tâche de contrôle des moteurs (plus élevée)
#define WIFI_TASK_PRIORITY        1       // Priorité de la tâche WiFi

// Configuration de FreeRTOS
#define configMAX_TASKS                       15  // Optimisation ressources

// Configuration de FreeRTOS pour les statistiques de tâches
#define configUSE_TRACE_FACILITY              1
#define configUSE_STATS_FORMATTING_FUNCTIONS  1

// === INFORMATIONS SYSTÈME ===

#define SYSTEM_VERSION            "2.0.0"     // Version du système
#define SYSTEM_NAME               "Kite PiloteV3"  // Nom du système
#define SYSTEM_BUILD_DATE         __DATE__    // Date de compilation
#define SYSTEM_BUILD_TIME         __TIME__    // Heure de compilation

#endif // CONFIG_H
