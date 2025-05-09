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

// Broches SDA et SCL pour bus I2C (écran LCD)
#define I2C_SDA           21    // Broche SDA pour bus I2C (écran LCD)
#define I2C_SCL           22    // Broche SCL pour bus I2C (écran LCD)

// Paramètres de l'écran LCD 20×4 I2C
#define LCD_COLS          20    // Nombre de colonnes du LCD
#define LCD_ROWS          4     // Nombre de lignes du LCD
#define LCD_I2C_ADDR      0x27  // Adresse I2C du module LCD

// Broches des potentiomètres
#define POT_DIRECTION     34    // ADC1_CH6 : potentiomètre de direction
#define POT_TRIM          35    // ADC1_CH7 : potentiomètre de trim
#define POT_LENGTH        32    // ADC1_CH4 : potentiomètre longueur des lignes

// Broches des boutons
#define BUTTON_UP_PIN     25    // Navigation haut (bouton vert)
#define BUTTON_DOWN_PIN   26    // Navigation bas (bouton jaune)
#define BUTTON_SELECT_PIN 27    // Validation / sélection (bouton rouge)
#define BUTTON_BACK_PIN   35    // Retour / annulation (bouton bleu)

// LED indicatrice
#define LED_PIN           2     // LED d'état principale

// Broches des servomoteurs
#define SERVO_DIRECTION_PIN 13  // Contrôle de la direction
#define SERVO_TRIM_PIN      14  // Contrôle du trim
#define SERVO_LINEMOD_PIN   15  // Modulation de la longueur des lignes via servo (si utilisé)

// Broches du moteur pas à pas
#define STEPPER_IN1       5     // IN1 du driver pas à pas
#define STEPPER_IN2       18    // IN2 du driver pas à pas
#define STEPPER_IN3       19    // IN3 du driver pas à pas
#define STEPPER_IN4       23    // IN4 du driver pas à pas

// Paramètres des servomoteurs
#define SERVO_MIN_PULSE_WIDTH  500   // Largeur minimale d'impulsion (µs)
#define SERVO_MAX_PULSE_WIDTH  2500  // Largeur maximale d'impulsion (µs)
#define SERVO_MIN_ANGLE        0     // Angle minimum (degrés)
#define SERVO_MAX_ANGLE        180   // Angle maximum (degrés)
#define SERVO_FREQUENCY        50    // Fréquence PWM pour servos (Hz)

// Paramètres du moteur pas à pas
#define STEPPER_STEPS_PER_REV  200   // Nombre de pas par tour complet
#define STEPPER_MAX_SPEED      1000  // Vitesse max en pas/sec
#define STEPPER_ACCELERATION   500   // Accélération en pas/sec²

// Configuration ADC
#define ADC_RESOLUTION         4095  // Résolution ADC 12 bits (ESP32)
// No change needed here
#define POT_DEADZONE           10    // Zone morte pour éviter bruit

// === ACTIVATION/DÉSACTIVATION DES MODULES ===
// Mettre à 1 pour activer, 0 pour désactiver chaque module
#define MODULE_WEBSERVER_ENABLED      0   // Serveur web
#define MODULE_WIFI_ENABLED           1   // WiFi
#define MODULE_SERVO_ENABLED          1   // Servomoteurs
#define MODULE_DISPLAY_ENABLED        1   // Affichage LCD
#define MODULE_API_ENABLED            1   // API REST
#define MODULE_OTA_ENABLED            1   // Mise à jour OTA
#define MODULE_LOGGING_ENABLED        1   // Journalisation
#define MODULE_SENSORS_ENABLED        1   // Capteurs
#define MODULE_AUTOPILOT_ENABLED      1   // Autopilot
#define MODULE_WINCH_ENABLED          1   // Treuil

// Les macros MODULE_X_ENABLED deviennent des variables runtime modifiables
extern bool moduleWebserverEnabled;
extern bool moduleWifiEnabled;
extern bool moduleServoEnabled;
extern bool moduleDisplayEnabled;
extern bool moduleApiEnabled;
extern bool moduleOtaEnabled;
extern bool moduleLoggingEnabled;
extern bool moduleSensorsEnabled;
extern bool moduleAutopilotEnabled;
extern bool moduleWinchEnabled;

// === CONFIGURATION RÉSEAU ===

// Configuration WiFi
#define WIFI_SSID          "Wokwi-GUEST"   // SSID du réseau WiFi
#define WIFI_PASSWORD      ""               // Mot de passe WiFi
#define SERVER_PORT        80               // Port HTTP du serveur web
#define WIFI_TIMEOUT_MS    10000            // Durée max (ms) pour se connecter au WiFi

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
#define DISPLAY_UPDATE_INTERVAL    250    // Intervalle (ms) entre maj affichage
#define MENU_SCROLL_INTERVAL       600    // Intervalle de défilement du menu (ms)
#define DISPLAY_CHECK_INTERVAL     45000  // Vérification écran toutes les 45s
#define SYSTEM_CHECK_INTERVAL      15000  // Intervalle pour les vérifications système (ms)
#define INIT_RETRY_DELAY           500    // Délai entre les tentatives d'initialisation (ms)
#define WIFI_CHECK_INTERVAL        5000   // Vérification WiFi toutes les 5s
#define POT_READ_INTERVAL          100    // Lecture potentiomètres toutes les 100ms
#define BUTTON_CHECK_INTERVAL      50     // Lecture boutons toutes les 50ms
#define BUTTON_DEBOUNCE_DELAY      50     // Délai d'anti-rebond pour les boutons (ms)
#define SERVO_UPDATE_INTERVAL      20     // Maj servos toutes les 20ms
#define STEPPER_UPDATE_INTERVAL    5      // Intervalle de mise à jour du moteur pas à pas (ms)
#define DISPLAY_REFRESH_INTERVAL   200    // Intervalle de rafraîchissement de l'affichage (ms)

// Configuration de l'affichage
#define DISPLAY_UPDATE_THROTTLE    200    // Limite de mise à jour d'affichage (ms)

// === CONFIGURATION JOURNALISATION ===

// Niveaux de journalisation
#define LOG_LEVEL_NONE    0       // Pas de journalisation
#define LOG_LEVEL_ERROR   1       // Erreurs critiques uniquement
#define LOG_LEVEL_WARNING 2       // Erreurs et avertissements
#define LOG_LEVEL_INFO    3       // Informations générales
#define LOG_LEVEL_DEBUG   4       // Messages de débogage détaillés

// Constantes liées à la journalisation
#define LOG_TIMESTAMP_BUFFER_SIZE 30      // Taille du buffer d'horodatage
#define MEM_HISTORY_SIZE          30      // Nombre d'entrées dans l'historique mémoire
#define DEFAULT_LOG_LEVEL         LOG_LEVEL_INFO  // Niveau de log par défaut

// Codes de couleur pour terminal ANSI
#define COLOR_RED         "\033[31m"  // Rouge pour les erreurs
#define COLOR_GREEN       "\033[32m"  // Vert pour les informations
#define COLOR_YELLOW      "\033[33m"  // Jaune pour les avertissements
#define COLOR_BLUE        "\033[34m"  // Bleu
#define COLOR_MAGENTA     "\033[35m"  // Magenta
#define COLOR_CYAN        "\033[36m"  // Cyan pour le débogage
#define COLOR_RESET       "\033[0m"   // Réinitialise la couleur

// === CONFIGURATION MÉMOIRE ===

// Configuration mémoire
#ifndef MEMORY_OPTIMIZATION_ENABLED
#define MEMORY_OPTIMIZATION_ENABLED true  // Active les optimisations de mémoire
#endif
#define STRING_BUFFER_SIZE         128    // Taille des buffers pour chaînes
#define LOG_BUFFER_SIZE            192    // Taille du buffer de log
#define MAX_DISPLAY_BUFFER_SIZE    128    // Réduit - LCD a besoin de moins de mémoire que TFT
#define JSON_DOCUMENT_SIZE         2048   // Taille pour les documents JSON

// === CONFIGURATION TÂCHES FREERTOS ===

// Taille des piles de tâches FreeRTOS (en mots)
#define DISPLAY_TASK_STACK_SIZE   4096    // Réduit pour optimisation mémoire - LCD simple
#define POT_TASK_STACK_SIZE       2048    // Tâche de lecture des potentiomètres
#define MONITOR_TASK_STACK_SIZE   4096    // Taille de pile pour la tâche de monitoring
#define MOTOR_TASK_STACK_SIZE     3072    // Tâche de contrôle des moteurs
#define WIFI_TASK_STACK_SIZE      4096    // Tâche WiFi
#define SYSTEM_TASK_STACK_SIZE    2048    // Réduit pour optimisation mémoire
#define BUTTON_TASK_STACK_SIZE    2048    // Tâche pour la gestion des boutons
#define LOGGING_TASK_STACK_SIZE   2048    // Tâche pour la journalisation
#define IMU_TASK_STACK_SIZE       3072    // Tâche pour le capteur IMU
#define WINCH_TASK_STACK_SIZE     3072    // Tâche pour le treuil

// Priorités des tâches FreeRTOS (0-24, 24 étant la plus haute)
#define DISPLAY_TASK_PRIORITY      2      // Priorité tâche affichage
#define WIFI_TASK_PRIORITY         1      // Priorité tâche WiFi
#define POT_TASK_PRIORITY          2      // Priorité tâche potentiomètres
#define MOTOR_TASK_PRIORITY        3      // Priorité tâche moteurs
#define SYSTEM_TASK_PRIORITY       1      // Priorité tâche système
#define BUTTON_TASK_PRIORITY       2      // Priorité tâche boutons
#define LOGGING_TASK_PRIORITY      1      // Priorité tâche journalisation
#define IMU_TASK_PRIORITY          3      // Priorité tâche IMU
#define WINCH_TASK_PRIORITY        3      // Priorité tâche treuil

// Configuration de FreeRTOS
#define configMAX_TASKS            15     // Nombre max de tâches autorisées
#define MAX_TASKS                  10     // Nombre maximum de tâches gérées par TaskManager

// Configuration de FreeRTOS pour les statistiques de tâches
#define configUSE_TRACE_FACILITY              1
#define configUSE_STATS_FORMATTING_FUNCTIONS  1

// === INFORMATIONS SYSTÈME ===

#define SYSTEM_NAME        "Kite PiloteV3"    // Nom du système
#define SYSTEM_VERSION     "3.0.0"            // Version du firmware
#define SYSTEM_BUILD_DATE  __DATE__           // Date de compilation
#define SYSTEM_BUILD_TIME  __TIME__           // Heure de compilation

#endif // CONFIG_H
