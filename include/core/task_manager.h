/*
  -----------------------
  Kite PiloteV3 - Module de gestion des tâches
  -----------------------
  
  Module de gestion des tâches multiples et parallèles pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../include/config.h"    // Inclure le fichier config.h pour utiliser les constantes communes
#include "../hardware/io/display.h"

// Priorités des tâches (utilisation des constantes définies dans config.h)
#define TASK_PRIORITY_DISPLAY        DISPLAY_TASK_PRIORITY
#define TASK_PRIORITY_TOUCH          TOUCH_TASK_PRIORITY
#define TASK_PRIORITY_WIFI_MONITOR   WIFI_TASK_PRIORITY
#define TASK_PRIORITY_SYSTEM_MONITOR WIFI_TASK_PRIORITY

// Tailles des piles (stack) des tâches en mots (utilisation des constantes définies dans config.h)
#define STACK_SIZE_DISPLAY        DISPLAY_TASK_STACK_SIZE
#define STACK_SIZE_TOUCH          TOUCH_TASK_STACK_SIZE
#define STACK_SIZE_WIFI_MONITOR   WIFI_TASK_STACK_SIZE
#define STACK_SIZE_SYSTEM_MONITOR SYSTEM_TASK_STACK_SIZE

// Délais de rafraîchissement des tâches (en millisecondes)
#define DISPLAY_UPDATE_INTERVAL    100  // Mise à jour de l'affichage rapide
#define TOUCH_CHECK_INTERVAL        50  // Vérification fréquente des événements tactiles
#define WIFI_CHECK_INTERVAL      10000  // Vérification du WiFi toutes les 10 secondes
// Utilisation de la constante définie dans config.h pour éviter la redéfinition
// #define SYSTEM_CHECK_INTERVAL     5000  // Vérification du système toutes les 5 secondes

// Structure des messages pour la communication inter-tâches
typedef struct {
  uint8_t type;           // Type de message
  uint16_t value;         // Valeur associée
  String message;         // Message texte (si nécessaire)
} TaskMessage;

// Types de messages
enum MessageType {
  MSG_NONE = 0,
  MSG_DISPLAY_UPDATE,
  MSG_TOUCH_EVENT,
  MSG_WIFI_STATUS,
  MSG_SYSTEM_STATUS,
  MSG_ERROR,
  MSG_TASK_LIST
};

class TaskManager {
  public:
    // Constructeur et destructeur
    TaskManager();
    ~TaskManager();
    
    // Initialisation du gestionnaire de tâches
    void begin(DisplayManager* display, AsyncWebServer* server);
    
    // Démarrage des tâches
    void startTasks();
    
    // Arrêt des tâches
    void stopTasks();
    
    // Envoi de messages entre tâches
    static bool sendMessage(MessageType type, uint16_t value, const char* message);
    
    // Status des tâches
    bool isRunning() const;
    
  private:
    // Handles des tâches
    TaskHandle_t displayTaskHandle;
    TaskHandle_t touchTaskHandle;
    TaskHandle_t wifiMonitorTaskHandle;
    TaskHandle_t systemMonitorTaskHandle;
    
    // File d'attente pour la communication inter-tâches
    static QueueHandle_t messageQueue;
    
    // Mutex pour l'accès aux ressources partagées
    static SemaphoreHandle_t displayMutex;
    
    // Références aux objets globaux
    static DisplayManager* displayManager;
    static AsyncWebServer* webServer;
    
    // État du gestionnaire
    bool running;
    
    // Fonctions statiques pour les tâches FreeRTOS
    static void displayTask(void* parameter);
    static void touchTask(void* parameter);
    static void wifiMonitorTask(void* parameter);
    static void systemMonitorTask(void* parameter);
    
    // Fonctions utilitaires
    static void processMessage(const TaskMessage& msg);
};

#endif // TASK_MANAGER_H
