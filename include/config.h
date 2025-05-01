/*
  -----------------------
  Kite PiloteV3 - Fichier de configuration
  -----------------------
  
  Ce fichier regroupe toutes les constantes configurables et définitions de pins
  pour faciliter la maintenance et la personnalisation du système.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === CONFIGURATION HARDWARE ===

// Paramètres de l'écran ILI9341
// Pour la rotation 2 (90° à gauche), on inverse largeur et hauteur
#define SCREEN_WIDTH      240      // Largeur de l'écran en pixels (après rotation)
#define SCREEN_HEIGHT     320      // Hauteur de l'écran en pixels (après rotation)

// Pins SPI pour l'écran TFT
#define TFT_CS            5       // Pin CS pour l'écran
#define TFT_DC            27        // Pin DC pour l'écran
#define TFT_RST           33       // Pin RESET pour l'écran
#define TFT_MOSI          23       // Pin MOSI pour l'écran
#define TFT_MISO          19       // Pin MISO pour l'écran
#define TFT_CLK           18       // Pin CLK pour l'écran

// Pins I2C pour l'écran tactile FT6206
#define I2C_SDA           21       // Pin SDA pour I2C
#define I2C_SCL           22       // Pin SCL pour I2C
#define TOUCH_IRQ         4        // Pin d'interruption tactile (optionnel)

// Pins GPIO
#define LED_PIN           2        // Pin pour la LED indicatrice
#define BUTTON_BLUE_PIN   15        // Pin pour le bouton bleu
#define BUTTON_GREEN_PIN  16        // Pin pour le bouton vert

// === CONFIGURATION RÉSEAU ===

// Configuration WiFi
#define WIFI_SSID         "Wokwi-GUEST"   // SSID du réseau WiFi
#define WIFI_PASSWORD     ""              // Mot de passe WiFi (vide pour réseau ouvert)
#define SERVER_PORT       80              // Port du serveur web
#define WIFI_TIMEOUT_MS   20000           // Timeout pour la connexion WiFi (ms)

// === CONFIGURATION INTERFACE ===

// Couleurs (format 16-bit 565)
#define COLOR_BLACK       0x0000
#define COLOR_BLUE        0x001F
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_YELLOW      0xFFE0
#define COLOR_WHITE       0xFFFF
#define COLOR_ORANGE      0xFD20
#define COLOR_LIGHTGREY   0xC618

// Paramètres d'affichage
#define TEXT_SIZE_TITLE   3          // Taille du texte pour les titres
#define TEXT_SIZE_NORMAL  2          // Taille du texte normal
#define TEXT_SIZE_SMALL   1          // Taille du texte petit

// === CONFIGURATION SYSTÈME ===

// Paramètres temporels
#define DISPLAY_ROTATION_INTERVAL 5000    // Intervalle de rotation des écrans (ms)
#define DISPLAY_CHECK_INTERVAL    30000   // Intervalle de vérification de l'écran (ms)
#define SYSTEM_CHECK_INTERVAL     10000   // Intervalle de vérification du système (ms)
#define INIT_RETRY_DELAY          500     // Délai entre les tentatives d'initialisation (ms)

// Nombre maximum de tentatives
#define MAX_INIT_ATTEMPTS         3       // Nombre maximal de tentatives d'initialisation

// === CONFIGURATION INTERFACE TACTILE ===

// Limites pour les écrans
#define MAX_SCREENS               10      // Nombre maximum d'écrans dans l'interface
#define MAX_BUTTONS_PER_SCREEN    10      // Nombre maximum de boutons par écran

// === CONFIGURATION TÂCHES FREERTOS ===

// Taille des piles de tâches FreeRTOS (en mots)
#define DISPLAY_TASK_STACK_SIZE   4096    // Taille de pile pour la tâche d'affichage
#define TOUCH_TASK_STACK_SIZE     4096    // Taille de pile pour la tâche tactile
#define WIFI_TASK_STACK_SIZE      8192    // Taille de pile pour la tâche WiFi/WebServer
#define SYSTEM_TASK_STACK_SIZE    8192    // Taille de pile pour la tâche système/monitoring

// Configuration mémoire
#define USE_STATIC_MEMORY          true    // Utiliser des allocations statiques quand possible
#define STRING_BUFFER_SIZE         128     // Taille des buffers pour les chaînes de caractères
#define LOG_BUFFER_SIZE            256     // Taille du buffer pour les messages de journalisation

// Priorités des tâches FreeRTOS (0-24, 24 étant la plus haute)
#define DISPLAY_TASK_PRIORITY     2       // Priorité de la tâche d'affichage
#define TOUCH_TASK_PRIORITY       2       // Priorité de la tâche tactile
#define WIFI_TASK_PRIORITY        1       // Priorité de la tâche WiFi

// Configuration de FreeRTOS
#define configMAX_TASKS                       25  // Nombre maximum de tâches

// Configuration de FreeRTOS pour les statistiques de tâches
#define configUSE_TRACE_FACILITY              1
#define configUSE_STATS_FORMATTING_FUNCTIONS  1

// === INFORMATIONS SYSTÈME ===

#define SYSTEM_VERSION            "2.0.0"     // Version du système
#define SYSTEM_NAME               "Kite PiloteV3"  // Nom du système
#define SYSTEM_BUILD_DATE         __DATE__    // Date de compilation
#define SYSTEM_BUILD_TIME         __TIME__    // Heure de compilation

#endif // CONFIG_H
