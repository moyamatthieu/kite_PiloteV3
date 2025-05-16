/*
  -----------------------
  Kite PiloteV3 - Fichier de configuration globale
  -----------------------
  
  Ce fichier regroupe les constantes de configuration globales et les valeurs par défaut.
  Les configurations spécifiques aux pilotes sont dans leurs fichiers d'en-tête respectifs (par ex., imu_driver.h).
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === CONFIGURATION GÉNÉRALE DU SYSTÈME ===
#define SYSTEM_NAME        "Kite PiloteV3"    // Nom du système
#define SYSTEM_VERSION     "3.0.0"            // Version du firmware
#define SYSTEM_BUILD_DATE  __DATE__           // Date de compilation
#define SYSTEM_BUILD_TIME  __TIME__           // Heure de compilation

// ==========================================================================
// SYSTEM ORCHESTRATOR CONFIGURATION
// ==========================================================================
#ifndef SYSTEM_VERSION
#define SYSTEM_VERSION "1.0.0-refactor" // Default system version
#endif

#define WDT_DEFAULT_TIMEOUT_SECONDS 10 // Watchdog Timer default timeout in seconds

// File d'attente de messages système
// #define SYSTEM_MESSAGE_QUEUE_SIZE 20 // Supprimé car redéfini plus bas
// Taille de la pile pour le moniteur système
// #define SYSTEM_MONITOR_STACK_SIZE 4096 // Supprimé car redéfini plus bas
// Priorité du moniteur système
// #define SYSTEM_MONITOR_PRIORITY 2 // Supprimé car redéfini plus bas
// Intervalle de surveillance du système (ms)
#define SYSTEM_MONITOR_INTERVAL_MS 5000

// Configuration des servomoteurs
#define MAX_SERVOS_PER_DRIVER 4
#define SERVO_DEFAULT_MIN_PULSE_WIDTH_US 500   // Largeur minimale d'impulsion (us)
#define SERVO_DEFAULT_MAX_PULSE_WIDTH_US 2500  // Largeur maximale d'impulsion (us)
#define SERVO_DEFAULT_MIN_ANGLE_DEG -90        // Angle minimal (degrés)
#define SERVO_DEFAULT_MAX_ANGLE_DEG 90         // Angle maximal (degrés)
#define SERVO_DEFAULT_NEUTRAL_ANGLE_DEG 0      // Angle neutre (degrés)

// Task configuration for the system restart task
#define CONFIG_RESTART_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)
#define CONFIG_DEFAULT_RESTART_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

// === CONFIGURATION DES BROCHES PAR DÉFAUT ===
// Ces valeurs peuvent être surchargées par des configurations spécifiques au moment de l'exécution.

// --- Bus I2C ---
#define I2C_DEFAULT_SDA_PIN           21
#define I2C_DEFAULT_SCL_PIN           22

// --- Affichage LCD (I2C) ---
#define DISPLAY_DEFAULT_I2C_ADDRESS   0x27
#define DISPLAY_DEFAULT_COLUMNS       20
#define DISPLAY_DEFAULT_ROWS          4

// --- Potentiomètres (ADC) ---
#define POT_DEFAULT_DIRECTION_PIN     34    // ADC1_CH6
#define POT_DEFAULT_TRIM_PIN          35    // ADC1_CH7
#define POT_DEFAULT_LENGTH_PIN        32    // ADC1_CH4
#define BUTTON_DOWN_PIN   26    // Navigation bas (bouton jaune)
#define BUTTON_SELECT_PIN 27    // Validation / sélection (bouton rouge)
#define BUTTON_BACK_PIN   35    // Retour / annulation (bouton bleu)
#define BUTTON_UP_PIN     25    // Navigation haut (bouton vert)

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
#define WIFI_DEFAULT_SSID            "KitePiloteV3_AP"      // SSID par défaut en mode AP
#define WIFI_DEFAULT_PASSWORD        "kite12345"            // Mot de passe par défaut en mode AP
#define WIFI_DEFAULT_CHANNEL         1                      // Canal WiFi par défaut
#define WIFI_DEFAULT_TIMEOUT_MS      30000                  // Timeout de connexion WiFi (30sec)

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
#define BUTTON_UP         BUTTON_GREEN  // Alias pour la navigation
#define BUTTON_DOWN       BUTTON_YELLOW // Alias pour la navigation
#define BUTTON_BACK       BUTTON_BLUE   // Alias pour retour
#define BUTTON_SELECT     BUTTON_RED    // Alias pour validation

// Constantes pour les boutons
#define BUTTON_DEFAULT_DEBOUNCE_MS     50     // Temps de debounce par défaut en ms
#define BUTTON_DEFAULT_LONG_PRESS_MS   1000   // Temps pour considérer un appui long en ms

// Constantes pour les potentiomètres
#define POT_DEFAULT_ADC_MIN            0      // Valeur minimale ADC par défaut
#define POT_DEFAULT_ADC_MAX            4095   // Valeur maximale ADC par défaut (ESP32 = 12 bits)
#define POT_DEFAULT_SMOOTHING_FACTOR   0.2f   // Facteur de lissage par défaut (0.0-1.0)

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

// ==========================================================================
// ===                        TASK MANAGER CONFIG                       ===
// ==========================================================================
#define SYSTEM_MESSAGE_QUEUE_SIZE 10 // Taille de la file de messages système
#define SYSTEM_MONITOR_STACK_SIZE 2048 // Taille de la pile pour la tâche de monitoring système
#define SYSTEM_MONITOR_PRIORITY (tskIDLE_PRIORITY + 2) // Priorité de la tâche de monitoring système
// #define SYSTEM_MONITOR_INTERVAL_MS 5000 // Supprimé car défini plus haut

// === INFORMATIONS SYSTÈME ===

#define SYSTEM_NAME        "Kite PiloteV3"    // Nom du système
#define SYSTEM_VERSION     "3.0.0"            // Version du firmware
#define SYSTEM_BUILD_DATE  __DATE__           // Date de compilation
#define SYSTEM_BUILD_TIME  __TIME__           // Heure de compilation

// ==========================================================================
// ===                        PID CONTROLLER CONFIG                       ===
// ==========================================================================
#define PID_DEFAULT_KP 1.0f
#define PID_DEFAULT_KI 0.1f
#define PID_DEFAULT_KD 0.01f
#define PID_DEFAULT_SETPOINT 0.0f
#define PID_DEFAULT_MIN_OUTPUT -255.0f
#define PID_DEFAULT_MAX_OUTPUT 255.0f
#define PID_DEFAULT_SAMPLE_TIME_MS 10
#define PID_DEFAULT_INTEGRAL_LIMIT 1000.0f // Default integral limit for anti-windup

// ==========================================================================
// ===                        CONTROL SERVICE CONFIG                      ===
// ==========================================================================
#define CONTROL_SERVICE_TASK_STACK_SIZE 4096
#define CONTROL_SERVICE_TASK_PRIORITY TaskPriority::PRIORITY_HIGH 
#define CONTROL_SERVICE_DEFAULT_LOOP_DELAY_MS 20 // Fréquence de la boucle de contrôle principale

// Paramètres de contrôle spécifiques (exemples, à affiner)
#define TARGET_LINE_TENSION_MIN 50.0f   // Newton
#define TARGET_LINE_TENSION_MAX 200.0f  // Newton
#define TARGET_ALTITUDE_MIN 30.0f     // Mètres
#define TARGET_ALTITUDE_MAX 150.0f    // Mètres

// ==========================================================================
// ===                        ACTUATOR SERVICE CONFIG                   ===
// ==========================================================================
#define ACTUATOR_SERVICE_TASK_STACK_SIZE 4096
#define ACTUATOR_SERVICE_TASK_PRIORITY TaskPriority::PRIORITY_HIGH // Priorité élevée pour une réactivité des actionneurs
#define ACTUATOR_SERVICE_DEFAULT_LOOP_DELAY_MS 50 // Fréquence de la boucle si des mises à jour périodiques sont nécessaires

// Définir des ID pour les servos si plusieurs sont utilisés et gérés par ce service
#define SERVO_LEFT_ID 0
#define SERVO_RIGHT_ID 1
// #define SERVO_RUDDER_ID 2 // Exemple

// Plages de commandes normalisées (exemples, peuvent être spécifiques aux drivers HAL)
#define ACTUATOR_NORMALIZED_MIN -1.0f
#define ACTUATOR_NORMALIZED_MAX  1.0f

// Limites pour les actionneurs spécifiques (peuvent être dans les configs des drivers HAL aussi)
// WINCH
#define WINCH_DEFAULT_MIN_SPEED ACTUATOR_NORMALIZED_MIN // -1.0 pour dérouler max
#define WINCH_DEFAULT_MAX_SPEED ACTUATOR_NORMALIZED_MAX // +1.0 pour enrouler max

// SERVOS (angles en degrés, par exemple)
#define SERVO_DEFAULT_MIN_ANGLE -90.0f // Degrés
#define SERVO_DEFAULT_MAX_ANGLE  90.0f // Degrés
#define SERVO_DEFAULT_NEUTRAL_ANGLE 0.0f   // Degrés

// === CONFIGURATION TREUIL (WINCH) ===
#define WINCH_DEFAULT_MOTOR_PIN_A 12   // Broche de contrôle A du moteur
#define WINCH_DEFAULT_MOTOR_PIN_B 13   // Broche de contrôle B du moteur
#define WINCH_DEFAULT_SPEED_PIN   14   // Broche PWM pour contrôle de vitesse
#define WINCH_DEFAULT_PWM_CHANNEL 4    // Canal PWM pour le contrôle de vitesse

// === CONFIGURATION TÂCHES FREERTOS ===

#endif // CONFIG_H
