/*
  -----------------------
  Kite PiloteV3 - Module de Gestion des Tâches (Interface)
  -----------------------
  
  Interface du gestionnaire de tâches FreeRTOS pour le système Kite PiloteV3.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== INTERFACE PUBLIQUE =====
  Ce fichier définit l'interface publique pour le gestionnaire de tâches qui
  orchestre l'exécution des différentes fonctionnalités du système.
  
  Principales fonctionnalités exposées :
  - TaskManager::begin() : Initialisation du gestionnaire et de ses ressources
  - TaskManager::startTasks() : Création et démarrage des tâches
  - TaskManager::stopTasks() : Arrêt des tâches
  - TaskManager::checkTasksHealth() : Surveillance de l'état des tâches
  
  Interactions avec d'autres modules :
  - UIManager : Gestion de l'interface utilisateur
  - WiFiManager : Communication sans fil
  - Tous les modules qui s'exécutent comme des tâches FreeRTOS
  
  Contraintes techniques :
  - Les tâches ont des priorités différentes (1-24, 24 étant la plus élevée)
  - Les tailles de pile doivent être définies avec soin selon les besoins de chaque tâche
  - Les délais doivent utiliser vTaskDelayUntil plutôt que vTaskDelay pour une meilleure précision
*/

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <vector>
#include <string>
#include "config.h"
#include "hardware/io/ui_manager.h"
#include "communication/wifi_manager.h"

// === STRUCTURES ===

// Structure pour les messages
typedef struct {
    uint8_t level;     // Niveau de log
    char tag[16];      // Tag du message
    char message[128]; // Contenu du message
} Message;

// Structure pour les métriques des tâches.

/**
 * Structure pour les métriques des tâches.
 */
typedef struct {
    uint32_t cpuUsage;            // Utilisation du CPU par la tâche
    uint32_t stackHighWaterMark;  // Marque haute de la pile
    uint32_t lastRunTime;         // Dernière exécution de la tâche
} TaskStat;

/**
 * Structure pour les paramètres de tâches.
 */
typedef struct {
    int taskIndex;       // Index de la tâche
    void* manager;       // Pointeur vers le gestionnaire associé
    uint32_t period;     // Période d'exécution de la tâche
    bool isRealtime;     // Indique si la tâche est en temps réel
} TaskParams;

/**
 * Structure pour la configuration des tâches.
 */
typedef struct {
    const char* name;         // Nom de la tâche
    TaskFunction_t function;  // Fonction associée à la tâche
    uint32_t stackSize;       // Taille de la pile
    UBaseType_t priority;     // Priorité de la tâche
    BaseType_t core;          // Coeur d'exécution
    uint32_t period;          // Période d'exécution
    bool isRealtime;          // Indique si la tâche est en temps réel
} TaskConfig;

/**
 * Définition d'une tâche
 */
typedef struct {
    const char* name;         // Nom de la tâche
    TaskFunction_t taskFunction;  // Fonction associée à la tâche
    uint32_t stackSize;       // Taille de la pile
    UBaseType_t priority;     // Priorité de la tâche
    TaskHandle_t handle;      // Handle de la tâche
} TaskDefinition;

// === CLASSE TASK MANAGER ===
/**
 * Gestionnaire de tâches multiples
 * Permet de gérer et surveiller les tâches FreeRTOS
 */
class TaskManager {
private:
    // Handles des tâches
    static TaskHandle_t displayTaskHandle;      // Handle pour la tâche d'affichage (rendu statique)
    static TaskHandle_t buttonTaskHandle;       // Handle pour la tâche des boutons (rendu statique)
    static TaskHandle_t inputTaskHandle;        // Handle pour la tâche des potentiomètres
    static TaskHandle_t networkTaskHandle;      // Handle pour la tâche réseau
    static TaskHandle_t controlTaskHandle;      // Handle pour la tâche de contrôle
    static TaskHandle_t sensorTaskHandle;       // Handle pour la tâche des capteurs
    TaskHandle_t wifiMonitorTaskHandle;         // Handle pour la tâche de surveillance WiFi
    TaskHandle_t systemMonitorTaskHandle;       // Handle pour la tâche de surveillance système
    
    // Variables d'état
    bool running;            // Indique si le gestionnaire est en cours d'exécution
    bool tasksRunning;       // Indique si les tâches sont en cours d'exécution
    unsigned long lastTaskMetricsTime; // Dernière mise à jour des métriques
    
    // Handles et statistiques des tâches
    TaskHandle_t taskHandles[MAX_TASKS]; // Tableau des handles des tâches
    TaskParams* taskParams[MAX_TASKS];   // Tableau des paramètres des tâches
    TaskStat taskStats[MAX_TASKS];       // Tableau des statistiques des tâches
    
    // Ressources partagées
    static QueueHandle_t messageQueue;      // File de messages partagée
    static SemaphoreHandle_t displayMutex;  // Mutex pour l'affichage
    static UIManager* uiManager;            // Pointeur vers le gestionnaire d'interface utilisateur
    static WiFiManager* wifiManager;        // Pointeur vers le gestionnaire WiFi
    
    // Fonctions de tâches
    static void displayTask(void* parameters);  // Fonction pour la tâche d'affichage
    static void buttonTask(void* parameters);   // Fonction pour la tâche des boutons
    static void networkTask(void* parameters);  // Fonction pour la tâche réseau
    static void controlTask(void* parameters);  // Fonction pour la tâche de contrôle
    static void inputTask(void* parameters);    // Fonction pour la tâche d'entrée
    static void monitorTask(void* parameters);  // Fonction pour la tâche de surveillance
    static void sensorTask(void* parameters);   // Fonction pour la tâche des capteurs
    
public:
    // Constructeur et destructeur
    TaskManager();  // Constructeur du gestionnaire de tâches
    ~TaskManager(); // Destructeur du gestionnaire de tâches
    
    // Initialisation et démarrage
    bool begin(UIManager* ui, WiFiManager* wifi); // Initialise le gestionnaire avec les gestionnaires associés
    bool startTasks();                            // Démarre les tâches gérées
    void stopTasks();                             // Arrête les tâches gérées
    void stopAllTasks();                          // Arrête toutes les tâches
    
    // Surveillance des tâches
    void updateTaskMetrics();  // Met à jour les métriques des tâches
    bool checkTasksHealth();   // Vérifie l'état de santé des tâches
    
    // Getters
    bool isRunning() const { return running; } // Retourne l'état du gestionnaire

    // === API dynamique des modules ===
    bool setModuleEnabled(const char* name, bool enabled);
    void getAllModulesStatus(std::vector<std::pair<std::string, std::string>>& out);
};

#endif // TASK_MANAGER_H
